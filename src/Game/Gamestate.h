#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <unordered_map>
#include <vector>

// Core Entities
#include "../Economy/Company.h"
#include "../Economy/Factory.h"
#include "../World/Market.h"
#include "../World/TradeNode.h"
#include "../World/Map.h"
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

    // adders
    void addFactory(const Factory& f) { factories.emplace(f.getId(), f); }
    void addCompany(const Company& c) { companies.emplace(c.getId(), c); }
    void addMarket(const Market& m) { markets.emplace(m.getId(), m); }
    void addNode(const TradeNode& n) { nodes.emplace(n.getId(), n); }
    
    // Container Access
    auto& getMarkets() { return markets; }
    auto& getNodes() { return nodes; }
    auto& getCompanies() { return companies; }
    auto& getFactories() { return factories; }

    void clear();

    // Full Serialization Helper
    friend nlohmann::json serializeGamestate(const Gamestate& g);

private:
    int currentTurn = 0;
    int currentYear, currentMonth, currentDay;

    uuids::uuid playerCompanyId;
    std::unordered_map<uuids::uuid, Market> markets;
    std::unordered_map<uuids::uuid, TradeNode> nodes;
    std::unordered_map<uuids::uuid, Company> companies;
    std::unordered_map<uuids::uuid, Factory> factories;

    Map map;

    // String ID (JSON'daki) -> Gerçek UUID (Engine'deki) eşleşmesi
    std::unordered_map<std::string, uuids::uuid> instanceIdToUUID;
};