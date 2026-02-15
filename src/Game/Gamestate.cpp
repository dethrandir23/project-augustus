#include "Gamestate.h"
#include "World/Market.h"
#include <iostream>

// --- Entity Getters ---
Factory* Gamestate::getFactory(const uuids::uuid& id) {
    auto it = factories.find(id); return (it != factories.end()) ? &it->second : nullptr;
}
Company* Gamestate::getCompany(const uuids::uuid& id) {
    auto it = companies.find(id); return (it != companies.end()) ? &it->second : nullptr;
}
Market* Gamestate::getMarket(const uuids::uuid& id) {
    auto it = markets.find(id); return (it != markets.end()) ? &it->second : nullptr;
}
TradeNode* Gamestate::getTradeNode(const uuids::uuid& id) {
    auto it = nodes.find(id); return (it != nodes.end()) ? &it->second : nullptr;
}

void Gamestate::addFactory(const Factory& f) { factories.emplace(f.getId(), f); }
void Gamestate::addCompany(const Company& c) { companies.emplace(c.getId(), c); }
void Gamestate::addMarket(const Market& m) { markets.emplace(m.getId(), m); }
void Gamestate::addNode(const TradeNode& n) { nodes.emplace(n.getId(), n); }

// --- Date System ---
void Gamestate::advanceDate() {
    currentTurn++;
    currentDay++;
    if (currentDay > 30) { currentDay = 1; currentMonth++; }
    if (currentMonth > 12) { currentMonth = 1; currentYear++; }
}

std::string Gamestate::getDateString() const {
    return std::to_string(currentDay) + "/" + std::to_string(currentMonth) + "/" + std::to_string(currentYear);
}

void Gamestate::clear() {
    markets.clear(); nodes.clear(); companies.clear(); factories.clear();
    instanceIdToUUID.clear();
    currentTurn = 0;
}

    bool Gamestate::loadScenario(const std::string& scenarioId) {
        clear();

        if (!ScenarioManager::scenarios.count(scenarioId)) return false;
        const auto& scen = ScenarioManager::scenarios.at(scenarioId);

        if (!scen.start_date.empty()) {
            int y, m, d;
            // sscanf en hızlı yöntemdir bu format için
            if (sscanf(scen.start_date.c_str(), "%d-%d-%d", &y, &m, &d) == 3) {
                currentYear = y;
                currentMonth = m;
                currentDay = d;
            } else {
                // Format hatalıysa default
                currentYear = 1836; currentMonth = 1; currentDay = 1;
            }
        } else {
            currentYear = 1836; currentMonth = 1; currentDay = 1;
        }

        if (!MapManager::maps.count(scen.map_id)) return false;
        const auto& mapDef = MapManager::maps.at(scen.map_id);

        // 1. Marketleri Yarat (MarketManager + Scenario verisiyle)
        for (const auto& nodeDef : mapDef.nodes) {
            std::string mStrId = nodeDef.default_market_id;
            
            // Scenario override kontrolü
            for(const auto& nOver : scen.nodes) {
                if(nOver.instance_id == nodeDef.instance_id && !nOver.market_id.empty())
                    mStrId = nOver.market_id;
            }

            if (instanceIdToUUID.find(mStrId) == instanceIdToUUID.end()) {
                uuids::uuid mUUID = IdUtils::generateUuid();
                // MarketManager'dan ismi çek, yoksa ID'yi kullan
                std::string mName = mStrId;
                if(MarketManager::markets.count(mStrId)) mName = MarketManager::markets.at(mStrId).name;

                markets.emplace(mUUID, Market(mUUID, mName));
                instanceIdToUUID[mStrId] = mUUID;
            }
        }

        // 2. TradeNode'ları Yarat
        for (const auto& nodeDef : mapDef.nodes) {
            std::string mStrId = nodeDef.default_market_id;
            // (Scenario override logic burada da çalışır...)
            uuids::uuid mUUID = instanceIdToUUID[mStrId];

            TradeNode newNode(nodeDef.template_id, mUUID);
            newNode.setName(nodeDef.name);

            // Senaryo Özelleştirmeleri
            for(const auto& nOver : scen.nodes) {
                if(nOver.instance_id == nodeDef.instance_id) {
                    if (nOver.population_override > 0) newNode.setPopulation(nOver.population_override);
                    
                    // EKSTRA PIPELINES (İstediğin özellik kanka!)
                    for(const auto& pipeId : nOver.extra_pipelines) {
                        newNode.getStorage().add(pipeId, 0.0f);
                    }
                }
            }

            nodes.emplace(newNode.getId(), newNode);
            instanceIdToUUID[nodeDef.instance_id] = newNode.getId();
            markets.at(mUUID).addNode(newNode.getId());
        }

        // 3. Şirketleri Yarat (CompanyManager Template Sistemiyle)
        for (const auto& compDef : scen.companies) {
            uuids::uuid cUUID = IdUtils::generateUuid();
            
            // Şablonu (Template) kullanıyoruz!
            Company newComp;
            if (CompanyManager::templates.count(compDef.template_id)) {
                const auto& tmpl = CompanyManager::templates.at(compDef.template_id);
                newComp = Company(cUUID, compDef.name_override.empty() ? "Company" : compDef.name_override);
                newComp.setCapital(compDef.start_capital > 0 ? compDef.start_capital : tmpl.start_capital);
                newComp.setManpower(tmpl.start_manpower);
                // Başlangıç teknolojileri ve perkleri ekle
                newComp.knownTechnologies = tmpl.start_techs;
                for(const auto& pId : tmpl.start_perks) newComp.addPerk(pId);
            } else {
                // Şablon yoksa düz şirket yarat
                newComp = Company(cUUID, compDef.name_override);
                newComp.setCapital(compDef.start_capital);
            }

            companies.emplace(cUUID, newComp);
            instanceIdToUUID[compDef.instance_id] = cUUID;

        }

        return true;
    }

template <typename T>
nlohmann::json mapToJson(const std::unordered_map<uuids::uuid, T>& map) {
    nlohmann::json j = nlohmann::json::object();
    for (const auto& [id, item] : map) {
        j[uuids::to_string(id)] = item; 
    }
    return j;
}

nlohmann::json serializeGamestate(const Gamestate& g) {
    nlohmann::json j;

    j["turn"] = g.currentTurn;
    j["date"] = g.getDateString();
    j["player_id"] = uuids::to_string(g.playerCompanyId);
    
    j["markets"] = mapToJson(g.markets);
    j["nodes"] = mapToJson(g.nodes);
    j["companies"] = mapToJson(g.companies);
    j["factories"] = mapToJson(g.factories);

    return j;
}