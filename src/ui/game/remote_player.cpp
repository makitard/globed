#include "remote_player.hpp"
#include "name_colors.hpp"
#include <util.hpp>

bool operator==(const PlayerAccountData& lhs, const PlayerAccountData& rhs) {
    return lhs.cube == rhs.cube \
        && lhs.ship == rhs.ship \
        && lhs.ball == rhs.ball \
        && lhs.ufo == rhs.ufo \
        && lhs.wave == rhs.wave \
        && lhs.robot == rhs.robot \
        && lhs.spider == rhs.spider \
        && lhs.color1 == rhs.color1 \
        && lhs.color2 == rhs.color2;
}

bool RemotePlayer::init(PlayerAccountData data, bool isSecond_, RemotePlayerSettings settings_) {
    if (!CCNode::init()) {
        return false;
    }

    isSecond = isSecond_;

    innerNode = CCNode::create();
    innerNode->setAnchorPoint({0.5f, 0.5f});
    this->addChild(innerNode);

    isDefault = data == DEFAULT_PLAYER_ACCOUNT_DATA;

    checkpointNode = CCSprite::createWithSpriteFrameName("checkpoint_01_001.png");
    checkpointNode->setZOrder(1);
    checkpointNode->setID("dankmeme.globed/remote-player-practice");

    this->addChild(checkpointNode);

    settings = settings_;

    updateData(data, isDefault);

    if ((!settings.practiceIcon) || (!settings.secondNameEnabled && isSecond)) {
        checkpointNode->setVisible(false);
    }

    return true;
}

void RemotePlayer::tick(const SpecificIconData& data, bool practice, bool dead) {
    if (data.gameMode != lastMode) {
        lastMode = data.gameMode;
        setActiveIcon(lastMode);
    }

    if (settings.defaultMiniIcons && (data.isMini != wasMini || firstTick)) {
        wasMini = data.isMini;
        spCube->updatePlayerFrame(wasMini ? DEFAULT_PLAYER_ACCOUNT_DATA.cube : realCube, IconType::Cube);
        spBall->updatePlayerFrame(wasMini ? DEFAULT_PLAYER_ACCOUNT_DATA.ball : realBall, IconType::Ball);
    }

    if (settings.practiceIcon && (practice != wasPractice || firstTick)) {
        wasPractice = practice;
        if (!settings.secondNameEnabled && isSecond) practice = false;

        checkpointNode->setVisible(practice);
    }

    if (data.isGrounded != wasGrounded || firstTick) {
        wasGrounded = data.isGrounded;
        spRobot->m_robotSprite->tweenToAnimation(wasGrounded ? "run" : "fall_loop", 0.2f);
        spSpider->m_spiderSprite->tweenToAnimation(wasGrounded ? "run" : "fall_loop", 0.2f);
    }

    if (dead != wasDead && settings.deathEffects) {
        wasDead = dead;
        if (dead) {
            playDeathEffect();
        }
    }

    firstTick = false;
}

void RemotePlayer::setActiveIcon(IconGameMode mode) {
    SimplePlayer* requiredPlayer;
    switch (mode) {
    case IconGameMode::CUBE:
        requiredPlayer = spCube;
        break;
    case IconGameMode::SHIP:
        requiredPlayer = spShip;
        break;
    case IconGameMode::BALL:
        requiredPlayer = spBall;
        break;
    case IconGameMode::UFO:
        requiredPlayer = spUfo;
        break;
    case IconGameMode::WAVE:
        requiredPlayer = spWave;
        break;
    case IconGameMode::ROBOT:
        requiredPlayer = spRobot;
        break;
    case IconGameMode::SPIDER:
        requiredPlayer = spSpider;
        break;
    default:
        throw std::invalid_argument("Did you really try to set active mode to IconGameMode::NONE?");
    }
    
    // hide all
    for (SimplePlayer* obj : {spCube, spShip, spBall, spUfo, spWave, spRobot, spSpider}) {
        obj->setVisible(false);
    }

    requiredPlayer->setVisible(true);
}

void RemotePlayer::updateData(PlayerAccountData data, bool areDefaults) {
    innerNode->removeAllChildren();
    if (labelName)
        labelName->removeFromParent();

    if (!areDefaults) {
        isDefault = false;
    }

    log::debug("updating data death effect to {}", data.deathEffect);

    deathEffectId = data.deathEffect;

    // create icons
    spCube = SimplePlayer::create(data.cube);
    spCube->updatePlayerFrame(data.cube, IconType::Cube);
    spCube->setID("dankmeme.globed/remote-player-cube");
    realCube = data.cube;

    spShip = SimplePlayer::create(data.ship);
    spShip->updatePlayerFrame(data.ship, IconType::Ship);
    spShip->setID("dankmeme.globed/remote-player-ship");
    spShip->setPosition({0.f, -5.f});
    
    spShipPassenger = SimplePlayer::create(data.cube);
    spShipPassenger->updatePlayerFrame(data.cube, IconType::Cube);
    spShipPassenger->setID("dankmeme.globed/remote-player-ship-passenger");
    spShipPassenger->setPosition({0.f, 10.f});
    spShipPassenger->setScale(0.55f);
    spShip->addChild(spShipPassenger);

    spBall = SimplePlayer::create(data.ball);
    spBall->updatePlayerFrame(data.ball, IconType::Ball);
    spBall->setID("dankmeme.globed/remote-player-ball");
    realBall = data.ball;

    spUfo = SimplePlayer::create(data.ufo);
    spUfo->updatePlayerFrame(data.ufo, IconType::Ufo);
    spUfo->setID("dankmeme.globed/remote-player-ufo");

    spUfoPassenger = SimplePlayer::create(data.cube);
    spUfoPassenger->updatePlayerFrame(data.cube, IconType::Cube);
    spUfoPassenger->setID("dankmeme.globed/remote-player-ufo-passenger");
    spUfoPassenger->setPosition({0.f, 5.f});
    spUfoPassenger->setScale(0.55f);
    spUfo->addChild(spUfoPassenger);

    spWave = SimplePlayer::create(data.wave);
    spWave->updatePlayerFrame(data.wave, IconType::Wave);
    spWave->setID("dankmeme.globed/remote-player-wave");

    spRobot = SimplePlayer::create(data.robot);
    spRobot->updatePlayerFrame(data.robot, IconType::Robot);
    spRobot->setID("dankmeme.globed/remote-player-robot");

    spSpider = SimplePlayer::create(data.spider);
    spSpider->updatePlayerFrame(data.spider, IconType::Spider);
    spSpider->setID("dankmeme.globed/remote-player-spider");

    // get colors
    auto primary = GameManager::get()->colorForIdx(data.color1);
    auto secondary = GameManager::get()->colorForIdx(data.color2);

    if (isSecond) {
        setValuesAndAdd(secondary, primary, data.glow); // swap colors for duals
    } else {
        setValuesAndAdd(primary, secondary, data.glow);
    }

    lastMode = IconGameMode::NONE;

    // update name
    name = data.name;
    float nameOffset = settings.nameOffset;

    if (isSecond) {
        nameOffset *= -1; // reverse direction for dual
    }

    if (!settings.namesEnabled) {
        if (settings.practiceIcon) {
            if (isSecond && !settings.secondNameEnabled) return;

            // if no names, just put the practice icon above the player's head
            checkpointNode->setScale(settings.nameScale * 0.8);
            checkpointNode->setPosition({0.f, 0.f + nameOffset});
            return;
        }
    } else {
        if (isSecond && !settings.secondNameEnabled) return;

        auto cpNodeWidth = checkpointNode->getContentSize().width;

        labelName = CCLabelBMFont::create(name.c_str(), "chatFont.fnt");
        labelName->setID("dankmeme.globed/remote-player-name");
        labelName->setPosition({0.f, 0.f + nameOffset});
        labelName->setScale(settings.nameScale);
        labelName->setZOrder(1);
        labelName->setOpacity(settings.nameOpacity);
        if (settings.nameColors) {
            labelName->setColor(pickNameColor(name));
        }
        this->addChild(labelName);

        if (settings.practiceIcon) {
            checkpointNode->setScale(settings.nameScale * 0.8);
            checkpointNode->setPosition({-(labelName->getContentSize().width / 2) - cpNodeWidth / 2, 0.f + nameOffset});
        }
    }
}

// returns a rand() value from 0.0f to 1.0f
inline float rng() {
    return rand() / 32767.0f;
}

void RemotePlayer::playDeathEffect() {
    // re from PlayerObject::playDeathEffect
    log::debug("death effect: playing with id {}", deathEffectId);
    auto particles = CCParticleSystemQuad::create("explodeEffect.plist");
    particles->setPosition({0.f, 0.f});

    auto particleRemoveSeq = CCSequence::create(
        CCDelayTime::create(2.0f),
        CCCallFunc::create(particles, callfunc_selector(CCSprite::removeFromParent)),
        nullptr
    );

    particles->runAction(particleRemoveSeq);

    this->addChild(particles);

    if (settings.defaultDeathEffects || deathEffectId <= 1) {
        // TODO make this closer to the actual circle wave death effect
        auto* wave = CCCircleWave::create(10.f, 80.f, 0.4f, false, true);
        wave->m_color = ccColor3B{255, 255, 255};
        this->addChild(wave);
        return;
    }

    // this is actually disgusting
    // beware

    auto deId = deathEffectId - 1;
    auto effectFileBase = CCString::createWithFormat("playerExplosion_%02d_", deId);

    log::debug("death effect: base path: {}", effectFileBase);
    
    int frameStart = 1;
    int frameEnd = 10;
    auto effectRootNumber = "001";
    float rootScale = 1.0f;
    float rootRotation = 0.0f;
    float animDelay;
    float delayTimeDt = 6.f;
    float fadeoutDt = 6.f;
    bool needsCircleWave = true;
    ccColor3B circleWaveColor;
    bool disableSecondAction = false;
    
    // + 0x50 is setScale
    // + 0x54 is getScale
    // + 0x58 is setScaleY
    // + 0x5c is getScaleY
    // + 0xa8 is setRotation
    
    // figure out the colors properly for circle wave !!!
    // they are all kinda small !!!
    switch (deathEffectId) {
    case 2:
        frameStart = static_cast<int>(rng() <= 0.5) + 1;
        frameEnd = 12;
        rootScale = (rng() * 2.0f - 1.0f) * 0.2f + 1.5f;
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = rng() * 0.01 + 0.05;
        delayTimeDt = animDelay * 6.f;
        fadeoutDt = animDelay * 6.f;
        circleWaveColor = ccColor3B{.r = 154, .g = 153, .b = 25};
        break;
    case 3:
        frameStart = 2;
        frameEnd = 10;
        rootScale = (rng() * 2.0f - 1.0f) * 0.2f + 1.2f;
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = rng() * 0.01 + 0.05;
        delayTimeDt = animDelay * 6.f;
        fadeoutDt = animDelay * 6.f;
        circleWaveColor = ccColor3B{.r = 0, .g = 0, .b = 128};
        break;
    case 4: // explodeEffectGrav
        //  this one also kinda weird
        frameStart = 1;
        frameEnd = 10;
        rootScale = (rng() * 2.0f - 1.0f) * 0.2f + 1.4f;
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = rng() * 0.005f + 0.04f;
        delayTimeDt = animDelay * 5.f;
        fadeoutDt = animDelay * 10.f;
        circleWaveColor = ccColor3B{.r = 0, .g = 0, .b = 128};
        break;
    case 5: // explodeEffectVortex
        frameStart = static_cast<int>(rng() <= 0.5) + 1;
        frameEnd = 9;
        rootScale = (rng() * 2.0f - 1.0f) * 0.1f + 1.2f;
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = (rng() * 2.0f - 1.0f) * 0.005f + 0.05f;
        delayTimeDt = animDelay * 8.0f;
        fadeoutDt = animDelay * 6.0f;
        break;
    case 6:
        frameStart = static_cast<int>(rng() <= 0.5) + 2;
        frameEnd = 10;
        rootScale = (rng() * 2.0f - 1.0f) * 0.15f + 1.25f;
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = (rng() * 2.0f - 1.0f) * 0.005f + 0.05f;
        delayTimeDt = animDelay * 6.0f;
        fadeoutDt = animDelay * 6.0f;
        break;
    case 7: // explodeEffectGrav
        frameStart = 1;
        frameEnd = 13;
        rootScale = (rng() * 2.0f - 1.0f) * 0.15f + 1.3f; //multiplied by some random ass thing i have no ida what is field_0xbc
        animDelay = 0.055f;
        delayTimeDt = 0.44f;
        fadeoutDt = 0.33f;
        break;
    case 8:
        frameStart = static_cast<int>(rng() <= 0.5) + 1;
        frameEnd = 11;
        rootScale = (rng() * 2.0f - 1.0f) * 0.2f + 1.3f; // mult'd by another random ass thing
        rootRotation = (rng() * 2.0f - 1.0f) * 90.0f;
        animDelay = rng() * 0.005f + 0.04f;
        delayTimeDt = animDelay * 6.0f;
        fadeoutDt = animDelay * 6.0f;
        break;
    case 9: // explodeEffectVortex
        frameStart = 2;
        frameEnd = 11;
        rootScale = rng() * 0.1f + 0.8f;
        animDelay = rng() * 0.01f + 0.045f;
        delayTimeDt = animDelay * 5.0f;
        fadeoutDt = animDelay * 8.0f;
        break;
    case 10: // explodeEffectVortex
        // maybe decrease frameStart by 1
        frameStart = 2;
        frameEnd = 13;
        rootScale = (rng() * 2.0f - 1.0f) * 0.1f + 1.0f;
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = (rng() * 2.0f - 1.0f) * 0.005f + 0.045f;
        delayTimeDt = animDelay * 10.0f;
        fadeoutDt = animDelay * 3.0f;
        break;
    case 11: // explodeEffectVortex
        // this kinda weird too
        frameStart = 1;
        frameEnd = 12;
        rootScale = (rng() * 2.0f - 1.0f) * 0.3f + 1.5f; // another random ass mult
        animDelay = (rng() * 2.0f - 1.0f) * 0.005f + 0.035f;
        delayTimeDt = animDelay * 7.0f;
        fadeoutDt = animDelay * 6.0f;
        break;
    case 12: // explodeEffectVortex
        frameStart = 1;
        frameEnd = 12;
        rootScale = (rng() * 2.0f - 1.0f) * 0.1f + 1.25f; // another random ass mult
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = (rng() * 2.0f - 1.0f) * 0.005f + 0.04f;
        delayTimeDt = animDelay * 7.0f;
        fadeoutDt = animDelay * 6.0f; // color is potentially  0x3f4ccccd
        break;
    case 13: // explodeEffectVortex
        frameStart = 1;
        frameEnd = 12;
        rootScale = (rng() * 0.2f + 0.9f); // another mult
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = (rng() * 0.005f + 0.04f);
        delayTimeDt = animDelay * 5.0f;
        fadeoutDt = animDelay * 8.0f;
        break;
    case 14:
        frameStart = 2;
        frameEnd = 12;
        rootScale = (rng() * 0.4f + 1.0f);
        // rob why..
        if (rng() >= 0.25f) {
            if (rng() < 0.25f) {
                circleWaveColor = {.r = 0, .g = 0, .b = 52};
            } else if (rng() < 0.25) {
                circleWaveColor = {.r = 0, .g = 0, .b = 135};
            }
        } else {
            circleWaveColor = {.r = 0, .g = 0, .b = 180};
        }
        animDelay = (rng() * 0.005f) + 0.045f;
        delayTimeDt = animDelay * 5.0f;
        fadeoutDt = animDelay * 8.0f;
        break;
    case 15: // explodeEffectVortex
        effectRootNumber = "002"; // mf
        frameStart = 3;
        frameEnd = 15;
        rootScale = (rng() * 0.2f + 1.0f); // random ass mult
        rootRotation = (rng() * 2.0f - 1.0f) * 45.0f;
        animDelay = rng() * 0.004f + 0.033f;
        delayTimeDt = animDelay * 5.0f;
        fadeoutDt = animDelay * 8.0f;
        break;
    case 16: // explodeEffectVortex, two circle waves
        frameStart = 2;
        frameEnd = 16;
        rootScale = (rng() * 0.4f + 1.4f); // random ass mult
        rootRotation = rng() * 360.f;
        animDelay = rng() * 0.004f + 0.04f;
        disableSecondAction = true; // mf x2
        break;
    case 17:
        effectRootNumber = "002"; // mf x3
        frameStart = 2;
        frameEnd = 0x13;
        rootScale = (rng() * 0.3f + 1.0f); // random as mult
        animDelay = rng() * 0.005f + 0.05f;
        delayTimeDt = animDelay * 12.0f;
        fadeoutDt = animDelay * 6.0f;
        break;
    }

    auto effectRootFile = fmt::format("{}{}.png", effectFileBase->getCString(), effectRootNumber);

    log::debug("death effect: creating root file: {}", effectRootFile);
    auto rootspr = CCSprite::createWithSpriteFrameName(effectRootFile.c_str());
    rootspr->setScale(rootScale);
    rootspr->setRotation(rootRotation);

    if (rootspr == nullptr) {
        log::debug("death effect: creating root file failed.");
        return;
    }

    auto sfc = CCSpriteFrameCache::get();

    auto frames = CCArray::create();
    for (int frame = frameStart; frame <= frameEnd; frame++) {
        auto effectFile = CCString::createWithFormat("%s%03d.png", effectFileBase->getCString(), frame);

        auto spr = sfc->spriteFrameByName(effectFile->getCString());
        if (spr == nullptr) {
            log::warn("death effect: frame path is nullptr: {}", effectFile);
            continue;
        }
        frames->addObject(spr);
    }

    auto animation = CCAnimation::createWithSpriteFrames(frames, animDelay);
    auto animate = CCAnimate::create(animation);
    auto callfunc = CCCallFunc::create(rootspr, callfunc_selector(CCSprite::removeFromParent));
    auto sequence = CCSequence::create(animate, callfunc, nullptr);
    rootspr->runAction(sequence);

    if (!disableSecondAction) {
        auto delaytime = CCDelayTime::create(delayTimeDt);
        auto fadeout = CCFadeOut::create(fadeoutDt);
        sequence = CCSequence::create(delaytime, fadeout, nullptr);
        rootspr->runAction(sequence);
    }

    this->addChild(rootspr);

    if (needsCircleWave) {
        auto cw = CCCircleWave::create(10.0f, 110.0f, 0.6f, false, true);
        cw->m_color = circleWaveColor;
        this->addChild(cw);
    }
}

void deCallFunc() {

}

void RemotePlayer::setRotationX(float x) { innerNode->setRotationX(x); }
void RemotePlayer::setRotationY(float y) { innerNode->setRotationY(y); }
void RemotePlayer::setRotation(float y) { innerNode->setRotation(y); }
void RemotePlayer::setScale(float scale) { innerNode->setScale(scale); }
void RemotePlayer::setScaleX(float scale) { innerNode->setScaleX(scale); }
void RemotePlayer::setScaleY(float scale) { innerNode->setScaleY(scale); }
float RemotePlayer::getRotationX() { return innerNode->getRotationX(); }
float RemotePlayer::getRotationY() { return innerNode->getRotationY(); }
float RemotePlayer::getRotation() { return innerNode->getRotation(); }
void RemotePlayer::setVisible(bool visible) {
    if (innerNode)
        innerNode->setVisible(visible);

    if (labelName)
        labelName->setVisible(visible);
}

void RemotePlayer::setOpacity(unsigned char opacity) {
    for (SimplePlayer* obj : {spCube, spShip, spBall, spUfo, spWave, spRobot, spSpider, spShipPassenger, spUfoPassenger}) {
        obj->setOpacity(opacity);
    }
}

RemotePlayer* RemotePlayer::create(bool isSecond, RemotePlayerSettings settings_, PlayerAccountData data) {
    auto ret = new RemotePlayer;
    if (ret && ret->init(data, isSecond, settings_)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void RemotePlayer::setValuesAndAdd(ccColor3B primary, ccColor3B secondary, bool glow) {
    // sets colors and adds them this node
    for (SimplePlayer* obj : {spCube, spShip, spBall, spUfo, spWave, spRobot, spSpider}) {
        obj->setColor(primary);
        obj->setSecondColor(secondary);
        obj->setVisible(false);
        obj->setGlowOutline(glow);
        innerNode->addChild(obj);
    }

    // passengers
    for (SimplePlayer* obj : {spShipPassenger, spUfoPassenger}) {
        obj->setColor(primary);
        obj->setSecondColor(secondary);
        obj->setGlowOutline(glow);
    }
}
