#pragma once
#include "Game/Gamestate.h"
#include "Registry/GameLoader.h"
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

using EngineCallback = std::function<void(const std::string &, const nlohmann::json &)>;

namespace augustus_engine {

class EngineController {
public:
    static EngineController& instance();

    EngineController(const EngineController&) = delete;
    EngineController& operator=(const EngineController&) = delete;

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
    std::string getDeltaState();
    std::string getPlayerState();
    std::string getMarketData(const std::string &marketId);
    std::string getEntityOrders(const std::string &ownerId);
    std::string getFactoryStatus(const std::string &factoryId);
    std::string getFactoryTemplates();
    std::string getPendingEvents();

    // --- Analytics ---
    std::string getCompanyNetWorth(const std::string &companyId);
    std::string getMarketStats(const std::string &marketId);
    std::string getNodeStats(const std::string &nodeId);
    std::string getFactoryStats(const std::string &factoryId);
    std::string getEconomyReport();
    std::string getEconomySummary();

    // --- Console ---
    std::vector<std::string> readConsole();
    void logToConsole(const std::string &msg);
    void logToConsole(const std::string &msg, LogType type);

    // --- Save / Load ---
    bool saveGame(const std::string &name);
    bool loadGame(const std::string &name);
    std::vector<std::string> listSaves();

    // --- Event Push Registration ---
    void registerCallback(EngineCallback cb);

    // --- Internal ---
    Gamestate &getGamestate() { return gamestate; }

private:
    EngineController();
    ~EngineController();

    Gamestate gamestate;
    GameLoader loader;
    bool initialized = false;

    void registerHandlers();
    void pushToCallbacks(const std::string &eventType, const nlohmann::json &payload);
    nlohmann::json computeDelta(const nlohmann::json &oldState,
                                 const nlohmann::json &newState);

    nlohmann::json previousState;
};

} // namespace augustus_engine
