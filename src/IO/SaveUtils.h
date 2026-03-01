// SaveUtils.h
#pragma once
#include "Game/Gamestate.h"

/**
 * @namespace SaveUtils
 * @brief Utility functions for managing game save states, including serialization and compression.
 */
namespace SaveUtils {

    bool saveGame(const Gamestate &gamestate, const std::string &saveName);

    bool loadGame(Gamestate &gamestate, const std::string &saveName);
    Gamestate loadGame(const std::string &saveName);

    std::vector<std::string> listSaves();
}