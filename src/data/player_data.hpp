#pragma once
#include <stdint.h>
#include "../net/bytebuffer.hpp"

enum class IconGameMode : uint8_t {
    CUBE = 0,
    SHIP = 1,
    BALL = 2,
    UFO = 3,
    WAVE = 4,
    ROBOT = 5,
    SPIDER = 6,
};

uint8_t gmToNumber(IconGameMode gm);
IconGameMode numberToGm(uint8_t number);

struct SpecificIconData {
    float x;
    float y;
    float xRot;
    float yRot;
    IconGameMode gameMode;

    bool isHidden;
    bool isDashing;
    bool isUpsideDown;
};

struct PlayerData {
    SpecificIconData player1;
    SpecificIconData player2;
    
    bool isPractice;
};


void encodeSpecificIcon(const SpecificIconData& data, ByteBuffer& buffer);
SpecificIconData decodeSpecificIcon(ByteBuffer& buffer);

void encodePlayerData(const PlayerData& data, ByteBuffer& buffer);
PlayerData decodePlayerData(ByteBuffer& buffer);