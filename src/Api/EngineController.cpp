#include "Api/EngineController.h"
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
#include "Registry/ScenarioManager.h"
#include "Registry/TechnologyManager.h"
#include "Registry/TradeNodeManager.h"

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
    if (ok) Console::log("Scenario '" + scenarioId + "' loaded.");
    else Console::log("Failed to load scenario.", LogType::ERROR);
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
    nlohmann::json payload = serializeGamestate(gamestate);
    pushToCallbacks("tick_complete", payload);
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
    // Search by instance ID or UUID
    // For now, return all markets
    nlohmann::json arr = nlohmann::json::array();
    for (auto *e : gamestate.getEntitiesByType("market")) {
        arr.push_back(e->ToJson());
    }
    return arr.dump();
}

std::string EngineController::getFactoryStatus(const std::string &factoryId) {
    // Parse UUID and return that factory's state
    auto opt = uuids::uuid::from_string(factoryId);
    if (!opt.has_value()) return "{}";
    Entity *f = gamestate.getEntity(opt.value());
    if (!f) return "{}";
    return f->ToJson().dump();
}

std::string EngineController::getPendingEvents() {
    const auto &queue = gamestate.getEventHandler().getQueue();
    nlohmann::json j = nlohmann::json::array();
    for (const auto &ev : queue) j.push_back(ev.toJson());
    return j.dump();
}

std::vector<std::string> EngineController::readConsole() {
    auto logs = Console::getLogMessages();
    Console::clearLogs();
    return logs;
}

void EngineController::logToConsole(const std::string &msg) {
    Console::log(msg);
}

bool EngineController::saveGame(const std::string &name) {
    return SaveUtils::saveGame(gamestate, name);
}

bool EngineController::loadGame(const std::string &name) {
    return SaveUtils::loadGame(gamestate, name);
}

std::vector<std::string> EngineController::listSaves() {
    return SaveUtils::listSaves();
}

void EngineController::registerCallback(EngineCallback cb) {
    // In WebApi, we'll store the JS callback and call it from pushToCallbacks
    // For now, store in a vector
    // This will be handled by WebApi's emscripten::val storage
}

void EngineController::pushToCallbacks(const std::string &eventType,
                                        const nlohmann::json &payload) {
    // Overridable in derived WebApi or handled via emscripten::val
}
