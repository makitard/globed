#include "corrector.hpp"
#include "../util.hpp"

const float DASH_DEGREES_PER_SECOND = 900.f; // this is weird, if too fast use 720.f

PlayerData emptyPlayerData() {
    return PlayerData {
        .timestamp = 0.f,
        .player1 = SpecificIconData {
            .x = 0.f,
            .y = 0.f,
            .rot = 0.f,
            .gameMode = IconGameMode::CUBE,
            .isHidden = true,
            .isDashing = false,
            .isUpsideDown = false,
            .isMini = false,
            .isGrounded = true,
        },
        .player2 = SpecificIconData {
            .x = 0.f,
            .y = 0.f,
            .rot = 0.f,
            .gameMode = IconGameMode::CUBE,
            .isHidden = true,
            .isDashing = false,
            .isUpsideDown = false,
            .isMini = false,
            .isGrounded = true,
        },
        .isPractice = false,
    };
}

bool closeEqual(float f1, float f2) {
    float fmin = f1 - 0.002f;
    float fmax = f1 + 0.002f;
    return f2 > fmin && f2 < fmax;
}

void PlayerCorrector::feedRealData(const std::unordered_map<int, PlayerData>& data) {
    for (const auto& [playerId, data] : data) {
        if (!playerData.contains(playerId)) {
            playersNew.push_back(playerId);
            playerData[playerId] = PlayerCorrectionData {
                .timestamp = data.timestamp,
                .newerFrame = data,
                .olderFrame = emptyPlayerData(),
                .sentPackets = 0,
                .tryCorrectTimestamp = false
            };
        } else {
            playerData[playerId].olderFrame = playerData[playerId].newerFrame;
            playerData[playerId].sentPackets += 1;

            // Removing this if clause makes it smoother on lower latency,
            // but potentially horrid on higher latencies.
            if (playerData[playerId].tryCorrectTimestamp || playerData[playerId].sentPackets < 60 || playerData[playerId].sentPackets % 60 == 0) {
                playerData[playerId].timestamp = playerData[playerId].olderFrame.timestamp;
            }
            playerData[playerId].newerFrame = data;
        }
    }

    std::vector<int> toRemove;

    for (const auto& [playerId, _] : playerData) {
        if (!data.contains(playerId)) {
            toRemove.push_back(playerId);
        }
    }

    std::lock_guard lock(mtx);
    for (int id : toRemove) {
        playersGone.push_back(id);
        playerData.erase(id);
    }
}

void PlayerCorrector::interpolate(const std::pair<RemotePlayer*, RemotePlayer*>& player, float frameDelta, int playerId) {
    std::lock_guard lock(mtx);
    if (!playerData.contains(playerId)) {
        return;
    }

    interpolateSpecific(player.first, frameDelta, playerId, false);
    interpolateSpecific(player.second, frameDelta, playerId, true);

    playerData.at(playerId).timestamp += frameDelta;
}

void PlayerCorrector::interpolateSpecific(RemotePlayer* player, float frameDelta, int playerId, bool isSecond) {
    auto& data = playerData.at(playerId);
    auto& olderData = isSecond ? data.olderFrame.player2 : data.olderFrame.player1;
    auto& newerData = isSecond ? data.newerFrame.player2 : data.newerFrame.player1;
    auto gamemode = newerData.gameMode;

    if (!newerData.isHidden) {
        player->setVisible(true);
        auto scale = newerData.isMini ? 0.6f : 1.0f;
        player->setScale(scale);

        if (gamemode != IconGameMode::CUBE) { // cube reacts differently to m_isUpsideDown i think
            player->setScaleY(scale * (newerData.isUpsideDown ? -1 : 1));
        }
        
        player->tick(newerData, data.newerFrame.isPractice);
    } else {
        player->setVisible(false);
        return;
    }

    auto olderTime = data.olderFrame.timestamp;
    auto newerTime = data.newerFrame.timestamp;
    
    // on higher pings targetUpdateDelay works like a charm,
    // the other one is a laggy mess

    // auto wholeTimeDelta = newerTime - olderTime;
    auto wholeTimeDelta = targetUpdateDelay;

    auto targetDelayIncrement = frameDelta / targetUpdateDelay;

    float currentTime = data.timestamp;
    float timeDelta = currentTime - olderTime;
    float timeDeltaRatio = timeDelta / wholeTimeDelta;

    auto pos = CCPoint {
        std::lerp(olderData.x, newerData.x, timeDeltaRatio),
        std::lerp(olderData.y, newerData.y, timeDeltaRatio),
    };

    if (gamemode == IconGameMode::SPIDER && newerData.isGrounded) {
        pos.y = olderData.y;
    }

    float rot = newerData.rot;

    if (newerData.isDashing && (gamemode == IconGameMode::CUBE || gamemode == IconGameMode::BALL)) {
        float dashDelta = DASH_DEGREES_PER_SECOND * wholeTimeDelta;
        if (newerData.isUpsideDown) {
            dashDelta *= -1;
        }

        auto base = player->getRotation();
        rot = base + std::lerp(0, dashDelta, timeDeltaRatio);
    } else {
        // disable rot interp if the spin is too big (such as from 580 degrees to -180).
        // dont ask me why that can happen, just gd moment.
        if (abs(newerData.rot - olderData.rot) < 360) {
            rot = std::lerp(olderData.rot, newerData.rot, timeDeltaRatio);
        }
    }

    if (timeDeltaRatio < 0.f || timeDeltaRatio > 2.f) {
        data.tryCorrectTimestamp = true;
        // log::debug("got a hiccup, tdr = {}, y: {} <-> {} = {}", timeDeltaRatio, olderData.y, newerData.y, pos.y);
    }

    player->setPosition(pos);
    player->setRotation(rot);
}

void PlayerCorrector::setTargetDelta(float dt) {
    targetUpdateDelay = dt;
}

void PlayerCorrector::joinedLevel() {
    playerData.clear();
    playersGone.clear();
    playersNew.clear();
}

std::vector<int> PlayerCorrector::getPlayerIds() {
    return globed_util::mapKeys(playerData);
}

std::vector<int> PlayerCorrector::getNewPlayers() {
    auto copy = playersNew;
    playersNew.clear();
    return copy;
}

std::vector<int> PlayerCorrector::getGonePlayers() {
    auto copy = playersGone;
    playersGone.clear();
    return copy;
}