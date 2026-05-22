#include "NativeApi.h"

namespace project_augustus {

static EngineController *controller = nullptr;

EngineController &getController() {
    if (!controller) controller = new EngineController();
    return *controller;
}

void initEngine() { getController().init(); }

bool loadGameFiles(const std::vector<std::string> &contents,
                   const std::vector<std::string> &names) {
    return getController().loadGameFiles(contents, names);
}

bool startScenario(const std::string &scenarioId) {
    return getController().startScenario(scenarioId);
}

void setPlayer(const std::string &name, const std::string &templateId, bool isAI) {
    getController().setPlayer(name, templateId, isAI);
}

void step() { getController().step(); }

std::string getSerializedState() { return getController().getSerializedState(); }

std::string getPlayerState() { return getController().getPlayerState(); }

std::string getMarketData(const std::string &marketId) {
    return getController().getMarketData(marketId);
}

std::string getFactoryStatus(const std::string &factoryId) {
    return getController().getFactoryStatus(factoryId);
}

bool sendInput(const std::string &inputJson) {
    try {
        nlohmann::json j = nlohmann::json::parse(inputJson);
        return getController().sendInput(j);
    } catch (const std::exception &e) {
        Console::log("Invalid JSON input: " + std::string(e.what()), LogType::ERROR);
        return false;
    }
}

void logToConsole(const std::string &msg, LogType type) {
    Console::log(msg, type);
}

std::vector<std::string> readConsole() {
    return getController().readConsole();
}

std::string getPendingEvents() {
    return getController().getPendingEvents();
}

bool saveGame(const std::string &name) { return getController().saveGame(name); }

bool loadGame(const std::string &name) { return getController().loadGame(name); }

std::vector<std::string> listSaves() { return getController().listSaves(); }

} // namespace project_augustus
