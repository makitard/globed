#pragma once
#include <Geode/Geode.hpp>
#include <data/player_account_data.hpp>
#include <data/player_data.hpp>
#include <data/settings_remote_player.hpp>

using namespace geode::prelude;

const PlayerAccountData DEFAULT_PLAYER_ACCOUNT_DATA = {
    .cube = 1,
    .ship = 1,
    .ball = 1,
    .ufo = 1,
    .wave = 1,
    .robot = 1,
    .spider = 1,
    .color1 = 0,
    .color2 = 3,
    .name = "Player",
};

bool operator==(const PlayerAccountData& lhs, const PlayerAccountData& rhs);

class RemotePlayer : public CCNode {
public:
    bool init(PlayerAccountData icons, bool isSecond_, RemotePlayerSettings settings_);

    void tick(const SpecificIconData& data, bool practice);
    void setActiveIcon(IconGameMode mode);
    void updateData(PlayerAccountData data, bool areDefaults = false);

    // those are proxy to innerNode.setXXX();
    // needed so that the player name label does not rotate when used in PPA engines
    void setRotationX(float x);
    void setRotationY(float y);
    void setRotation(float y);
    void setScale(float scale);
    void setScaleX(float scale);
    void setScaleY(float scale);
    float getRotationX();
    float getRotationY();
    float getRotation();

    // proxy to calling spXXX.XXX(), calls on all SimplePlayers
    void setOpacity(unsigned char opacity);

    static RemotePlayer* create(bool isSecond, RemotePlayerSettings settings_, PlayerAccountData data = DEFAULT_PLAYER_ACCOUNT_DATA);

    bool isDefault;
protected:
    void setValuesAndAdd(ccColor3B primary, ccColor3B secondary, bool glow);

    IconGameMode lastMode = IconGameMode::NONE;

    SimplePlayer* spCube;
    SimplePlayer* spShip;
    SimplePlayer* spBall;
    SimplePlayer* spUfo;
    SimplePlayer* spWave;
    SimplePlayer* spRobot;
    SimplePlayer* spSpider;
    CCLabelBMFont* labelName = nullptr;
    CCNode* innerNode;

    SimplePlayer* spShipPassenger;
    SimplePlayer* spUfoPassenger;
    
    std::string name;
    bool isSecond;

    RemotePlayerSettings settings;

    // these are for Default mini icon setting
    bool wasMini = false, firstTick = true;
    int realCube, realBall;

    // these are for practice icon setting
    bool wasPractice = false;
    CCSprite* checkpointNode;

    // for animations
    bool wasGrounded = false;
};