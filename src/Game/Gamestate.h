#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// Core Entities
#include "../Economy/Company.h"
#include "../Economy/Factory.h"
#include "../World/TradeNode.h"
#include "../World/Map.h"
#include "World/Market.h"
#include "Game/EventHandler.h"
#include "Game/IdUtils.h"

// Managers (Data Registry)
#include "../Registry/ScenarioManager.h"
#include "../Registry/MapManager.h"
#include "../Registry/TradeNodeManager.h"
#include "../Registry/CompanyManager.h" // Şirket şablonları için
#include "../Registry/MarketManager.h"  // Market tanımları için

class Gamestate {
public:
    Gamestate() = default;

    bool loadScenario(const std::string& scenarioId);
    void advanceDate();
    std::string getDateString() const;

    // Entity Retrieval
    Factory* getFactory(const uuids::uuid& id);
    Company* getCompany(const uuids::uuid& id);
    Market* getMarket(const uuids::uuid& id);
    TradeNode* getTradeNode(const uuids::uuid& id);

    // Getters
    uuids::uuid getPlayerCompanyId() const { return playerCompanyId; }
    Map& getMap() { return map; }
    int getCurrentTurn() const { return currentTurn; }
    int getCurrentYear() const { return currentYear; }
    int getCurrentMonth() const { return currentMonth; }
    int getCurrentDay() const { return currentDay; }
    int getGameSpeed() const { return gameSpeed; }

    // Setters
    void setPlayerCompanyId(const uuids::uuid& id) { playerCompanyId = id; }
    void setGameSpeed(int speed) { gameSpeed = speed; }

    // adders
    void addFactory(const Factory& f);
    void addCompany(const Company& c);
    void addMarket(const Market& m);
    void addNode(const TradeNode& n);
    
    // Container Access
    auto& getMarkets() { return markets; }
    auto& getNodes() { return nodes; }
    auto& getCompanies() { return companies; }
    auto& getFactories() { return factories; }

    void clear();

    // Full Serialization Helper
    friend nlohmann::json serializeGamestate(const Gamestate& g);

    EventHandler& getEventHandler() { return eventHandler; }
    const EventHandler& getEventHandler() const { return eventHandler; }

    inline std::optional<Company> getPlayerCompany() {
        if (!companies.count(playerCompanyId))
            return std::nullopt;
        return companies[playerCompanyId];
    }

int gameSpeed = 1;
bool paused = true;
float accumulator = 0.0f;
void togglePause() { paused = !paused; }
private:
    int currentTurn = 0;
    int currentYear, currentMonth, currentDay;


    uuids::uuid playerCompanyId;
    std::unordered_map<uuids::uuid, Market> markets;
    std::unordered_map<uuids::uuid, TradeNode> nodes;
    std::unordered_map<uuids::uuid, Company> companies;
    std::unordered_map<uuids::uuid, Factory> factories;

    Map map;

    std::unordered_map<std::string, uuids::uuid> instanceIdToUUID;

    EventHandler eventHandler;
};