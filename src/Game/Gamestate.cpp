#include "Gamestate.h"
#include "Economy/Components/WalletComponent.h"
#include "Game/Entity/EntityFactory.h"
#include "Game/Entity/EntityMarket.h"
#include "Game/Entity/EntityTradeNode.h"
#include "Game/Entity/EntityCompany.h"
#include "Registry/ScenarioManager.h"
#include "Registry/MapManager.h"
#include "Registry/MarketManager.h"
#include "Registry/TradeNodeManager.h"
#include "Registry/CompanyManager.h"
#include "World/Components/DemographicsComponent.h"

bool Gamestate::loadFromSave(const nlohmann::json &j) {
    try {
        // 1. Önceki dünyayı kıyamete sürükle (Temizle)
        this->clear(); 

        // 2. Temel Zaman ve Tur Verileri
        this->currentTurn = j.value("turn", 0);
        
        // Tarihi string'den (Örn: "15/4/1836") tekrar integer değerlere parçala
        std::string dateStr = j.value("date", "1/1/1836");
        int d = 1, m = 1, y = 1836;
        if (sscanf(dateStr.c_str(), "%d/%d/%d", &d, &m, &y) == 3) {
            this->currentDay = d;
            this->currentMonth = m;
            this->currentYear = y;
        }

        // 3. Oyuncu ID'si (Kameranın / UI'ın takip edeceği kişi)
        if (j.contains("player_id")) {
            std::string pIdStr = j.at("player_id").get<std::string>();
            if (!pIdStr.empty()) {
                auto parsedOpt = uuids::uuid::from_string(pIdStr);
                if (parsedOpt.has_value()) {
                    this->playerCompanyId = parsedOpt.value();
                }
            }
        }

        // 4. ENTITY'LERİ DİRİLT (Asıl Sihir Burada)
        if (j.contains("entities") && j.at("entities").is_array()) {
            for (const auto& entityJson : j.at("entities")) {
                
                Entity* newEntity = new Entity();
                
                newEntity->UpdateFromJson(entityJson); 
                
                // Dünyaya ekle!
                this->addEntity(newEntity);
            }
        }

        // İPUCU: Eğer MapComponent vs kullanıyorsan ve Map'in bir Cache sistemi varsa,
        // dünyayı yükledikten hemen sonra Cache'i tazelemelisin!
        // ornekMapSistemi.BuildCache(*this);

        Console::log("Oyun basariyla yuklendi! Turn: " + std::to_string(this->currentTurn), LogType::INFO);
        return true;

    } catch (const nlohmann::json::exception& e) {
        Console::log("Save dosyasi JSON format hatasi: " + std::string(e.what()), LogType::ERROR);
        return false;
    } catch (const std::exception& e) {
        Console::log("Save dosyasi yuklenirken genel hata: " + std::string(e.what()), LogType::ERROR);
        return false;
    }
}

void Gamestate::addEntity(Entity* entity) {
    if (entity) {
        entities[entity->GetId()] = entity;
    }
}

Entity* Gamestate::getEntity(const uuids::uuid& id) {
    auto it = entities.find(id);
    return (it != entities.end()) ? it->second : nullptr;
}

void Gamestate::removeEntity(const uuids::uuid& id) {
    auto it = entities.find(id);
    if (it != entities.end()) {
        delete it->second;
        entities.erase(it);
  }
    
 }

 // deprecated
 bool Gamestate::handleInput(const nlohmann::json& input) {
     // This method is a placeholder for now. The real input handling is done
     // via InputHandler::handleInput which takes Gamestate& as a parameter.
     // We keep this for potential future use.
     return true;
 }

 std::vector<Entity*> Gamestate::getEntitiesByType(const std::string& type) {
    std::vector<Entity*> result;
    for (const auto& [id, entity] : entities) {
        if (entity->GetType() == type) {
            result.push_back(entity);
        }
    }
    return result;
}

void Gamestate::clear() {
    for (auto& [id, entity] : entities) {
        delete entity; 
    }
    entities.clear();
    instanceIdToUUID.clear();
    currentTurn = 0;
}

// --- ZAMAN SİSTEMİ ---
void Gamestate::advanceDate() {
    currentTurn++; currentDay++;
    if (currentDay > 30) { currentDay = 1; currentMonth++; }
    if (currentMonth > 12) { currentMonth = 1; currentYear++; }
}

std::string Gamestate::getDateString() const {
    return std::to_string(currentDay) + "/" + std::to_string(currentMonth) + "/" + std::to_string(currentYear);
}

// --- SENARYO YÜKLEME ---
bool Gamestate::loadScenario(const std::string& scenarioId) {
    clear();
    if (!ScenarioManager::scenarios.count(scenarioId)) return false;
    const auto& scen = ScenarioManager::scenarios.at(scenarioId);

    // Tarih Yükleme...
    if (!scen.start_date.empty()) {
        int y, m, d;
        if (sscanf(scen.start_date.c_str(), "%d-%d-%d", &y, &m, &d) == 3) {
            currentYear = y; currentMonth = m; currentDay = d;
        } else { currentYear = 1836; currentMonth = 1; currentDay = 1; }
    }

    if (!MapManager::maps.count(scen.map_id)) return false;
    const auto& mapDef = MapManager::maps.at(scen.map_id);

    // 1. Marketleri Yarat
    for (const auto& nodeDef : mapDef.nodes) {
        std::string mStrId = nodeDef.default_market_id;
        // Scenario override kontrolü (Eski kodun aynısı...)
        for(const auto& nOver : scen.nodes) {
            if(nOver.instance_id == nodeDef.instance_id && !nOver.market_id.empty())
                mStrId = nOver.market_id;
        }

        if (instanceIdToUUID.find(mStrId) == instanceIdToUUID.end()) {
            std::string mName = MarketManager::markets.count(mStrId) ? MarketManager::markets.at(mStrId).name : mStrId;
            
            Entity* newMarket = createMarket(mName, mStrId);
            // Template'den tariff_rate'i uygula
            if (MarketManager::markets.count(mStrId)) {
                auto* marketComp = newMarket->GetComponent<MarketComponent>("MarketComponent");
                if (marketComp) marketComp->tariffRate = MarketManager::markets.at(mStrId).tariff_rate;
            }
            addEntity(newMarket);
            instanceIdToUUID[mStrId] = newMarket->GetId();
        }
    }

    // 2. Şehirleri (TradeNode) Yarat
    for (const auto& nodeDef : mapDef.nodes) {
        std::string mStrId = nodeDef.default_market_id;
        uuids::uuid mUUID = instanceIdToUUID[mStrId]; // Market'in ID'si

        if (TradeNodeManager::templates.count(nodeDef.template_id)) {
            const auto& tmpl = TradeNodeManager::templates.at(nodeDef.template_id);
            
            // YENİ SİSTEM: createTradeNode kullanıyoruz
            Entity* newNode = createTradeNode(tmpl, mUUID);
            newNode->SetName(nodeDef.name);

            // Senaryo override'ları (Nüfus vb.)
            for(const auto& nOver : scen.nodes) {
                if(nOver.instance_id == nodeDef.instance_id) {
                    if (nOver.population_override > 0) {
                        newNode->GetComponent<DemographicsComponent>("DemographicsComponent")->population = nOver.population_override;
                    }
                    for(const auto& pipeId : nOver.extra_pipelines) {
                        auto* localProd = newNode->GetComponent<ProductionComponent>("LocalProduction");
                        if (localProd) localProd->addPipeline(pipeId);
                    }
                }
            }

            addEntity(newNode);
            instanceIdToUUID[nodeDef.instance_id] = newNode->GetId();
        }
    }

    // 3. Şirketleri Yarat
    for (const auto& compDef : scen.companies) {
        if (CompanyManager::templates.count(compDef.template_id)) {
            const auto& tmpl = CompanyManager::templates.at(compDef.template_id);
            
            // YENİ SİSTEM: createCompany kullanıyoruz
            Entity* newComp = createCompany(tmpl, compDef.name_override);

            if (compDef.start_capital > 0) {
                newComp->GetComponent<WalletComponent>("WalletComponent")->balance = compDef.start_capital;
            }

            addEntity(newComp);
            instanceIdToUUID[compDef.instance_id] = newComp->GetId();
        }
    }

    return true;
}

// --- SERİLEŞTİRME (Kısacık oldu!) ---
nlohmann::json serializeGamestate(const Gamestate& g) {
    nlohmann::json j;
    j["turn"] = g.currentTurn;
    j["date"] = g.getDateString();
    j["player_id"] = uuids::to_string(g.playerCompanyId);
    
    // Sadece entityleri dön, hepsi kendi 'to_json'unu biliyor!
    j["entities"] = nlohmann::json::array();
    for (const auto& [id, entity] : g.entities) {
        nlohmann::json ej = entity->ToJson();
        j["entities"].push_back(ej);
    }

    return j;
}