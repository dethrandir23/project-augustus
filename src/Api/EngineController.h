#pragma once
#include "Game/Gamestate.h"
#include "Registry/GameLoader.h"
#include <memory>
#include <functional>
#include <string>
#include <vector>

using EngineCallback = std::function<void(const std::string &, const nlohmann::json &)>;

class EngineController {
public:
    EngineController();
    ~EngineController();

    // --- Init & Data Loading ---
    void init();
    bool loadGameFiles(const std::vector<std::string> &contents,
                       const std::vector<std::string> &names);
    bool startScenario(const std::string &scenarioId);
    void setPlayer(const std::string &name, const std::string &templateId, bool isAI);

    // --- Game Loop ---
    void step();
    void stepN(size_t n);

    // --- Input ---
    bool sendInput(const nlohmann::json &input);

    // --- State Queries ---
    std::string getSerializedState();
    std::string getPlayerState();
    std::string getMarketData(const std::string &marketId);
    std::string getFactoryStatus(const std::string &factoryId);
    std::string getPendingEvents();

    // --- Console ---
    std::vector<std::string> readConsole();
    void logToConsole(const std::string &msg);

    // --- Save / Load ---
    bool saveGame(const std::string &name);
    bool loadGame(const std::string &name);
    std::vector<std::string> listSaves();

    // --- Event Push Registration ---
    void registerCallback(EngineCallback cb);

    // --- Internal ---
    Gamestate &getGamestate() { return gamestate; }

private:
    Gamestate gamestate;
    GameLoader loader;
    bool initialized = false;

    void registerHandlers();
    void pushToCallbacks(const std::string &eventType, const nlohmann::json &payload);
};
