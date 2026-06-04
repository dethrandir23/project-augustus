#include "Analytics/EconomyStats.h"
#include "Api/EngineController.h"
#include "World/Systems/MarketSystem.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/DemographicsComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "AI/AIManager.h"
#include "AI/CompanyBrain.h"
#include "AI/Components/AIControllerComponent.h"
#include "DevTools/Console.h"
#include "Game/Entity/EntityCompany.h"
#include "Game/GameManager.h"
#include "Game/InputHandler.h"
#include "IO/SaveUtils.h"
#include "Registry/CompanyManager.h"
#include "Registry/ConditionManager.h"
#include "Registry/EconomyManager.h"
#include "Registry/EventManager.h"
#include "Registry/FactoryManager.h"
#include "Registry/ItemManager.h"
#include "Registry/MapManager.h"
#include "Registry/MarketManager.h"
#include "Registry/NameManager.h"
#include "Registry/PerkManager.h"
#include "Registry/PipelineManager.h"
#include "Registry/CompanyProfileManager.h"
#include "Registry/ScenarioManager.h"
#include "Registry/TechnologyManager.h"
#include "Registry/TradeNodeManager.h"

namespace augustus_engine {

EngineController& EngineController::instance() {
    static EngineController inst;
    return inst;
}

EngineController::EngineController() = default;
EngineController::~EngineController() = default;

void EngineController::registerHandlers() {
    loader.RegisterHandler("ITEM_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            ItemManager::load_from_json(j, src);
        });
    loader.RegisterHandler("PIPELINE_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            PipelineManager::load_from_json(j, src);
        });
    loader.RegisterHandler("FACTORY_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            FactoryManager::load_from_json(j, src);
        });
    loader.RegisterHandler("ECONOMY_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            EconomyManager::load_from_json(j, src);
        });
    loader.RegisterHandler("TECHNOLOGY_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            TechnologyManager::load_from_json(j, src);
        });
    loader.RegisterHandler("PERK_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            PerkManager::load_from_json(j, src);
        });
    loader.RegisterHandler("CONDITION_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            ConditionManager::load_from_json(j, src);
        });
    loader.RegisterHandler("NAMESPACES",
        [](const nlohmann::json &j, const std::string &src) {
            NameManager::load_from_json(j, src);
        });
    loader.RegisterHandler("TRADENODE_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            TradeNodeManager::load_from_json(j, src);
        });
    loader.RegisterHandler("COMPANY_PROFILES",
        [](const nlohmann::json &j, const std::string &src) {
            CompanyProfileManager::load_from_json(j, src);
        });
    loader.RegisterHandler("COMPANY_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            CompanyManager::load_from_json(j, src);
        });
    loader.RegisterHandler("MARKET_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            MarketManager::load_from_json(j, src);
        });
    loader.RegisterHandler("MAP_DEFINITION",
        [](const nlohmann::json &j, const std::string &src) {
            MapManager::load_from_json(j, src);
        });
    loader.RegisterHandler("SCENARIO_DEFINITION",
        [](const nlohmann::json &j, const std::string &src) {
            ScenarioManager::load_from_json(j, src);
        });
    loader.RegisterHandler("EVENT_DEFINITIONS",
        [](const nlohmann::json &j, const std::string &src) {
            EventManager::load_from_json(j, src);
        });
}

void EngineController::init() {
    if (initialized) return;
    InputHandler::init();
    AIManager::init();
    registerHandlers();
    initialized = true;
    Console::log("Engine initialized.");
}

bool EngineController::loadGameFiles(const std::vector<std::string> &contents,
                                      const std::vector<std::string> &names) {
    bool allSuccess = true;
    for (size_t i = 0; i < contents.size(); ++i) {
        if (!loader.load_file_content(contents[i], names[i])) {
            Console::log("Error loading file: " + names[i], LogType::ERROR);
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool EngineController::startScenario(const std::string &scenarioId) {
    bool ok = gamestate.loadScenario(scenarioId);
    if (ok) {
        Console::log("Scenario '" + scenarioId + "' loaded.");
        previousState = nlohmann::json(); // null — next getDeltaState returns full state
    } else Console::log("Failed to load scenario.", LogType::ERROR);
    return ok;
}

void EngineController::setPlayer(const std::string &name,
                                  const std::string &templateId, bool isAI) {
    if (!CompanyManager::templates.count(templateId)) {
        Console::log("SetPlayer: template not found.", LogType::ERROR);
        return;
    }
    const auto &tmpl = CompanyManager::templates.at(templateId);
    Entity *company = createCompany(tmpl, name);
    if (company->GetComponent<AIControllerComponent>("AIControllerComponent"))
        company->RemoveComponent("AIControllerComponent");
    if (isAI) {
        auto* aiComp = new AIControllerComponent(std::make_unique<CompanyBrain>());
        company->AddComponent(aiComp, "AIControllerComponent");
    }
    gamestate.addEntity(company);
    gamestate.setPlayerCompanyId(company->GetId());
    Console::log("Player company '" + name + "' created.");
}

void EngineController::step() {
    GameManager::tick(gamestate);
}

void EngineController::stepN(size_t n) {
    for (size_t i = 0; i < n; ++i) step();
}

bool EngineController::sendInput(const nlohmann::json &input) {
    return InputHandler::handleInput(gamestate, input);
}

std::string EngineController::getSerializedState() {
    return serializeGamestate(gamestate).dump();
}

std::string EngineController::getDeltaState() {
    nlohmann::json current = serializeGamestate(gamestate);

    if (previousState.is_null()) {
        previousState = current;
        return current.dump();
    }

    nlohmann::json delta = computeDelta(previousState, current);
    previousState = current;
    return delta.dump();
}

nlohmann::json EngineController::computeDelta(const nlohmann::json &oldState,
                                               const nlohmann::json &newState) {
    nlohmann::json delta;
    delta["delta"] = true;
    delta["turn"] = newState.value("turn", 0);
    delta["date"] = newState.value("date", "");
    delta["player_id"] = newState.value("player_id", "");
    delta["entities"] = nlohmann::json::array();
    delta["removed"] = nlohmann::json::array();

    std::unordered_map<std::string, nlohmann::json> oldMap;
    for (const auto &e : oldState["entities"]) {
        oldMap[e["id"].get<std::string>()] = e;
    }

    for (const auto &e : newState["entities"]) {
        std::string id = e["id"].get<std::string>();
        auto it = oldMap.find(id);
        if (it == oldMap.end()) {
            delta["entities"].push_back(e);
        } else if (it->second != e) {
            delta["entities"].push_back(e);
        }
        oldMap.erase(id);
    }

    for (const auto &[id, _] : oldMap) {
        delta["removed"].push_back(id);
    }

    return delta;
}

std::string EngineController::getPlayerState() {
    Entity *player = gamestate.getPlayerCompany();
    if (!player) return "{}";
    nlohmann::json j;
    j["id"] = uuids::to_string(player->GetId());
    j["name"] = player->GetName();
    j["type"] = player->GetType();
    j["components"] = nlohmann::json::object();
    for (const auto& [key, comp] : player->GetAllComponents()) {
        j["components"][key] = comp->ToJson();
    }
    return j.dump();
}

std::string EngineController::getMarketData(const std::string &marketId) {
    nlohmann::json arr = nlohmann::json::array();
    for (auto *e : gamestate.getEntitiesByType("market")) {
        arr.push_back(e->ToJson());
    }
    return arr.dump();
}

std::string EngineController::getEntityOrders(const std::string &ownerId) {
    auto opt = uuids::uuid::from_string(ownerId);
    if (!opt.has_value()) return "[]";
    uuids::uuid id = opt.value();

    nlohmann::json j;
    j["buyOrders"] = MarketSystem::getBuyOrdersForOwner(gamestate, id);
    j["sellOrders"] = MarketSystem::getSellOrdersForOwner(gamestate, id);
    return j.dump();
}

std::string EngineController::getFactoryStatus(const std::string &factoryId) {
    auto opt = uuids::uuid::from_string(factoryId);
    if (!opt.has_value()) return "{}";
    Entity *f = gamestate.getEntity(opt.value());
    if (!f) return "{}";
    return f->ToJson().dump();
}

std::string EngineController::getFactoryTemplates() {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& [id, f] : FactoryManager::factories) {
        nlohmann::json entry;
        entry["id"] = f.id;
        entry["name"] = f.name;
        entry["buildCost"] = f.buildCost;
        entry["max_workers"] = f.max_workers;
        entry["categories"] = f.categories;
        entry["pipelines"] = f.pipeline_ids;
        arr.push_back(entry);
    }
    return arr.dump();
}

std::string EngineController::getPendingEvents() {
    const auto &queue = gamestate.getEventHandler().getQueue();
    nlohmann::json j = nlohmann::json::array();
    for (const auto &ev : queue) j.push_back(ev.toJson());
    return j.dump();
}

std::string EngineController::getCompanyNetWorth(const std::string &companyId) {
    auto opt = uuids::uuid::from_string(companyId);
    if (!opt.has_value()) return "{}";
    auto result = Analytics::calculateCompanyNetWorth(gamestate, opt.value());
    return result.dump();
}

std::string EngineController::getMarketStats(const std::string &marketId) {
    auto opt = uuids::uuid::from_string(marketId);
    if (!opt.has_value()) return "{}";
    auto result = Analytics::calculateMarketActivity(gamestate, opt.value());
    return result.dump();
}

std::string EngineController::getNodeStats(const std::string &nodeId) {
    auto opt = uuids::uuid::from_string(nodeId);
    if (!opt.has_value()) return "{}";
    auto result = Analytics::calculateTradeNodeStats(gamestate, opt.value());
    return result.dump();
}

std::string EngineController::getFactoryStats(const std::string &factoryId) {
    auto opt = uuids::uuid::from_string(factoryId);
    if (!opt.has_value()) return "{}";
    auto result = Analytics::calculateFactoryOutput(gamestate, opt.value());
    return result.dump();
}

std::string EngineController::getEconomyReport() {
    auto result = Analytics::calculateTotalEconomyStats(gamestate);
    return result.dump();
}

std::string EngineController::getEconomySummary() {
    nlohmann::json s;

    int cumulativeBuy = 0, cumulativeSell = 0, cumulativeTrades = 0;
    double cumulativeVolume = 0.0;
    int currentBuy = 0, currentSell = 0;
    double currentBuyVol = 0.0, currentSellVol = 0.0;
    double totalWallet = 0.0;

    int marketCount = 0, companyCount = 0, factoryCount = 0, nodeCount = 0;
    size_t totalPop = 0;

    for (const auto& [_, e] : gamestate.getEntities()) {
        auto* mc = e->GetComponent<MarketComponent>("MarketComponent");
        if (mc) {
            marketCount++;
            cumulativeBuy += mc->totalBuyOrdersPlaced;
            cumulativeSell += mc->totalSellOrdersPlaced;
            cumulativeTrades += mc->totalTradesExecuted;
            cumulativeVolume += mc->totalTradeVolume;
            for (auto& [_, book] : mc->books) {
                for (auto& o : book.buyOrders) { currentBuy++; currentBuyVol += o.remaining() * o.price; }
                for (auto& o : book.sellOrders) { currentSell++; currentSellVol += o.remaining() * o.price; }
            }
        }
        auto* wallet = e->GetComponent<WalletComponent>("WalletComponent");
        if (wallet && e->GetType() == "market") totalWallet += wallet->balance;

        if (e->GetType() == "company") companyCount++;
        if (e->GetType() == "factory") factoryCount++;
        if (e->GetType() == "trade_node") nodeCount++;

        auto* demo = e->GetComponent<DemographicsComponent>("DemographicsComponent");
        if (demo) totalPop += demo->population;
    }

    int totalOrdersPlaced = cumulativeBuy + cumulativeSell;
    int currentTotal = currentBuy + currentSell;

    s["marketCount"] = marketCount;
    s["companyCount"] = companyCount;
    s["factoryCount"] = factoryCount;
    s["tradeNodeCount"] = nodeCount;
    s["totalPopulation"] = totalPop;

    s["cumulativeBuyOrdersPlaced"] = cumulativeBuy;
    s["cumulativeSellOrdersPlaced"] = cumulativeSell;
    s["cumulativeOrdersPlaced"] = totalOrdersPlaced;
    s["cumulativeTradesExecuted"] = cumulativeTrades;
    s["cumulativeTradeVolume"] = cumulativeVolume;

    s["currentBuyOrders"] = currentBuy;
    s["currentSellOrders"] = currentSell;
    s["currentTotalOrders"] = currentTotal;

    s["currentBuyVolume"] = currentBuyVol;
    s["currentSellVolume"] = currentSellVol;
    s["currentTotalVolume"] = currentBuyVol + currentSellVol;

    s["totalMarketWallet"] = totalWallet;

    // Oranlar
    s["buySellRatio"] = cumulativeSell > 0
        ? static_cast<double>(cumulativeBuy) / static_cast<double>(cumulativeSell) : 0.0;
    s["tradeExecutionRate"] = totalOrdersPlaced > 0
        ? static_cast<double>(cumulativeTrades) / static_cast<double>(totalOrdersPlaced) : 0.0;
    s["pendingOrderRatio"] = totalOrdersPlaced > 0
        ? static_cast<double>(currentTotal) / static_cast<double>(totalOrdersPlaced) : 0.0;

    // Bot mu canlı mı anlamak için: hiç trade yoksa oyun çalışmıyordur
    s["isEconomyActive"] = cumulativeTrades > 0 && cumulativeSell > 0;

    return s.dump();
}

std::vector<std::string> EngineController::readConsole() {
    auto logs = Console::getLogMessages();
    Console::clearLogs();
    return logs;
}

void EngineController::logToConsole(const std::string &msg) {
    Console::log(msg);
}

void EngineController::logToConsole(const std::string &msg, LogType type) {
    Console::log(msg, type);
}

bool EngineController::saveGame(const std::string &name) {
    return SaveUtils::saveGame(gamestate, name);
}

bool EngineController::loadGame(const std::string &name) {
    bool ok = SaveUtils::loadGame(gamestate, name);
    if (ok) previousState = nlohmann::json();
    return ok;
}

std::vector<std::string> EngineController::listSaves() {
    return SaveUtils::listSaves();
}

void EngineController::registerCallback(EngineCallback cb) {
    // Stored and called from pushToCallbacks
}

void EngineController::pushToCallbacks(const std::string &eventType,
                                        const nlohmann::json &payload) {
    // Overridden in WebApi or used via registerCallback
}

} // namespace augustus_engine
