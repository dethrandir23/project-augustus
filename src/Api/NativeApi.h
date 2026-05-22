#pragma once
#include "Api/EngineController.h"
#include "DevTools/Console.h"
#include <string>
#include <vector>

namespace project_augustus {
    void initEngine();
    bool loadGameFiles(const std::vector<std::string> &contents,
                       const std::vector<std::string> &names);
    bool startScenario(const std::string &scenarioId);
    void setPlayer(const std::string &name, const std::string &templateId, bool isAI);
    void step();
    std::string getSerializedState();
    std::string getPlayerState();
    std::string getMarketData(const std::string &marketId);
    std::string getFactoryStatus(const std::string &factoryId);
    bool sendInput(const std::string &inputJson);
    void logToConsole(const std::string &msg, LogType type);
    std::vector<std::string> readConsole();
    std::string getPendingEvents();
    bool saveGame(const std::string &name);
    bool loadGame(const std::string &name);
    std::vector<std::string> listSaves();
    EngineController &getController();
}
