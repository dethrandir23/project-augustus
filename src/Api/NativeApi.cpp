/**
 * @file NativeApi.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief The native API Interface.
 */

#include "NativeApi.h"
#include "AI/AIRegistry.h"
#include "AI/Components/AIControllerComponent.h"
#include "Game/Entity/EntityCompany.h"
#include "Game/Gamestate.h"
#include "Game/IdUtils.h"
#include "IO/SaveUtils.h"

// Tüm Manager'lar...
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

namespace GameApi {

Gamestate *globalGamestate = nullptr;
GameLoader loader;

void initEngine() {
  if (!globalGamestate) {
    globalGamestate = new Gamestate();
  }

  InputHandler::init();
  AIRegistry::init();

  // 1. Basic Economy & Items
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

  // 2. Techs, Perks & Conditions
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

  // 3. World & Entities Templates
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

  // 4. Map, Scenario & Events
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

  Console::log("Game initialized successfully.");
}

bool loadGameFiles(const std::vector<std::string> &file_contents,
                   const std::vector<std::string> &file_names) {
  bool allSuccess = true;
  for (size_t i = 0; i < file_contents.size(); ++i) {
    if (!loader.load_file_content(file_contents[i], file_names[i])) {
      Console::log("Error loading file: " + file_names[i], LogType::ERROR);
      allSuccess = false;
    }
  }
  return allSuccess;
}

bool startScenario(const std::string &scenarioId) {
  if (globalGamestate->loadScenario(scenarioId)) {
    Console::log("Scenario loaded successfully.");
    return true;
  } else {
    Console::log("Failed to load scenario.", LogType::ERROR);
    return false;
  }
}

void SetPlayer(const std::string &companyName, const std::string &templateId,
               bool isAI) {
  if (CompanyManager::templates.find(templateId) ==
      CompanyManager::templates.end()) {
    Console::log("SetPlayer Hata: Company Template bulunamadi.",
                 LogType::ERROR);
    return;
  }

  const auto &tmpl = CompanyManager::templates[templateId];

  Entity *playerCompany = createCompany(tmpl, companyName);
  if (playerCompany->GetComponent<AIControllerComponent>(
          "AIControllerComponent")) {
    playerCompany->RemoveComponent("AIControllerComponent");
  }
  if (isAI) {
    playerCompany->AddComponent(new AIControllerComponent(),
                                "AIControllerComponent");
  }
  if (isAI) {
    playerCompany->AddComponent(new AIControllerComponent(),
                                "AIControllerComponent");
  }

  globalGamestate->addEntity(playerCompany);
  globalGamestate->setPlayerCompanyId(playerCompany->GetId());

  Console::log("Player company '" + companyName + "' olusturuldu.");
}

void step() {
  if (!globalGamestate)
    return;
  GameManager::tick(*globalGamestate);
}

std::string getSerializedState() {
  if (!globalGamestate)
    return "{}";
  return serializeGamestate(*globalGamestate).dump();
}

bool sendInput(const std::string &input) {
  if (!globalGamestate)
    return false;

  try {
    // String'i JSON'a çevir!
    nlohmann::json j = nlohmann::json::parse(input);
    if (InputHandler::handleInput(*globalGamestate, j)) {
      Console::log("Input handled successfully.");
      return true;
    }
  } catch (const std::exception &e) {
    Console::log("Invalid JSON input: " + std::string(e.what()),
                 LogType::ERROR);
  }
  return false;
}

void logToConsole(const std::string &message, LogType type) {
  Console::log(message, type);
}

std::vector<std::string> readConsole() {
  auto logs = Console::getLogMessages();
  Console::clearLogs();
  return logs;
}

void subscribeToEvents() {
  if (!globalGamestate)
    return;
  globalGamestate->getEventHandler().subscribe([](const Event &e) {
    // Send signal to frontend
  });
}

std::string getPendingEvents() {
  if (!globalGamestate)
    return "[]";

  const auto &queue = globalGamestate->getEventHandler().getQueue();
  nlohmann::json j = nlohmann::json::array();
  for (const auto &ev : queue) {
    j.push_back(ev.toJson());
  }
  return j.dump();
}

bool saveGame(const std::string &saveName) {
  return SaveUtils::saveGame(*globalGamestate, saveName);
}

bool loadGame(const std::string &saveName) {
  return SaveUtils::loadGame(*globalGamestate, saveName);
}
} // namespace GameApi