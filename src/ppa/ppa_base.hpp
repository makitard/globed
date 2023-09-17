#pragma once
#include <Geode/Geode.hpp>
#include "../data/data.hpp"
#include "../ui/remote_player.hpp"

using namespace geode::prelude;

class PPAEngine {
public:
    virtual ~PPAEngine();
    virtual void updateSpecificPlayer(
        RemotePlayer* player,
        const SpecificIconData& data,
        float frameDelta,
        int playerId,
        bool isSecond
    ) = 0;
    virtual void updatePlayer(
        std::pair<RemotePlayer*, RemotePlayer*>& player,
        const PlayerData& data,
        float frameDelta,
        int playerId
    );
    virtual void updateHiddenPlayer(
        RemotePlayer* player,
        const SpecificIconData& data,
        float frameDelta,
        int playerId,
        bool isSecond
    );

    virtual void addPlayer(int playerId, const PlayerData& data);
    virtual void removePlayer(int playerId);

    void setTargetDelta(float dt);
protected:
    float targetUpdateDelay;
};