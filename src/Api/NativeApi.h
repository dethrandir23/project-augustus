/**
 * @file NativeApi.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief The native API Interface.
 */
#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>

// Core Engine Components
#include "Game/GameManager.h"
#include "Game/Gamestate.h"
#include "Game/InputHandler.h"
#include "Registry/GameLoader.h"
#include "DevTools/Console.h"
#include "IO/SaveUtils.h"

namespace GameApi {

    extern Gamestate* globalGamestate;
    extern GameLoader loader;

    void initEngine();

    bool loadGameFiles(const std::vector<std::string> &file_contents,
                       const std::vector<std::string> &file_names);

    bool startScenario(const std::string &scenarioId);

    void SetPlayer(const std::string &companyName, const std::string &templateId, bool isAI = false);

    void step();

    std::string getSerializedState();

    bool sendInput(const std::string &input);

    void logToConsole(const std::string &message, LogType type);

    std::vector<std::string> readConsole();

    void subscribeToEvents();

    std::string getPendingEvents();

    bool saveGame(const std::string& saveName);
    bool loadGame(const std::string& saveName);
}