#pragma once
#include "settings_remote_player.hpp"

struct GlobedGameSettings {
    bool displayProgress;
    bool newProgress;
    unsigned char playerOpacity;
    RemotePlayerSettings rpSettings;
};