/**
 * @file NativeApi.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief The native API Interface.
 * @details This file contains the API for native C++ applications.
 * @version 0.3 (Refactored & Logged)
 * @date 2026-02-04
 */

#include "NativeApi.h"
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>

// Core Engine Components
#include "../Game/GameManager.h"
#include "../Game/Gamestate.h"
#include "../Game/InputHandler.h"
#include "../Registry/GameLoader.h"

// DevTools
#include "../DevTools/Console.h"

// All Managers (For Registration)
#include "../Registry/CompanyManager.h" // Template Manager
#include "../Registry/ConditionManager.h"
#include "../Registry/EconomyManager.h"
#include "../Registry/EventManager.h"
#include "../Registry/FactoryManager.h"
#include "../Registry/ItemManager.h"
#include "../Registry/MapManager.h"
#include "../Registry/NameManager.h"
#include "../Registry/PerkManager.h"
#include "../Registry/PipelineManager.h"
#include "../Registry/ScenarioManager.h"
#include "../Registry/TechnologyManager.h"
#include "../Registry/TradeNodeManager.h"

namespace GameApi {
 Gamestate* globalGamestate = nullptr;
 GameLoader loader;  
/**
 * @brief Initializes the engine and registers all JSON handlers.
 * @details Call this ONCE at the start of the application.
 */
void initEngine() {

  if (!globalGamestate) {
      globalGamestate = new Gamestate();
  }

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

  // 4. Map & Scenario
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

/**
 * @brief Loads game data files (Mods, Core Data) from JS.
 */
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

/**
 * @brief Starts a new game by loading a specific scenario.
 * @param scenarioId The ID of the scenario to load (e.g., "debug_scenario").
 * @return true if successful.
 */
bool startScenario(const std::string &scenarioId) {
  if (globalGamestate.loadScenario(scenarioId)) {
    Console::log("Scenario loaded successfully.");
    return true;
  } else {
    Console::log("Failed to load scenario.", LogType::ERROR);
    return false;
  }
}

void SetPlayer(const std::string &companyName, const std::string &companyId) {
    CompanyTemplate tmpl = CompanyManager::templates[companyId];
    Company company(IdUtils::generateUuid(), tmpl);
    company.setName(companyName);
    globalGamestate.setPlayerCompanyId(company.getId());
    globalGamestate.addCompany(company);
}

/**
 * @brief Advances the simulation by one tick.
 */
void step() {
  GameManager::stepGamestate(globalGamestate);
  Console::log("Game stepped");
}

/**
 * @brief Returns the full game state as JSON.
 */
std::string getSerializedState() {
  return serializeGamestate(globalGamestate).dump();
}

/**
 * @brief Handles player input (CLI commands, etc.)
 */
bool sendInput(const std::string &input) {
  if (InputHandler::handleInput(globalGamestate, input)) {
    Console::log("Input handled successfully.");
    return true;
  } else {
    Console::log("Failed to handle input.", LogType::ERROR);
    return false;
  }
}

void logToConsole(const std::string &message, LogType type) {
  Console::log(message, type);
}

std::vector<std::string> readConsole() { return Console::getLogMessages(); }

void subscribeToEvents() {
  globalGamestate.getEventHandler().subscribe([](const Event &e) {
    // Frontend'e sinyal gönder (JNI, Emscripten vs.)
    // GameApi::sendToUI("SHOW_EVENT", e.tmpl->id);
  });
}

// Event Queue'sunu JSON string olarak döndüren bir API fonksiyonu
std::string getPendingEvents() {
  const auto &queue = globalGamestate.getEventHandler().getQueue();
  nlohmann::json j;
  return j.dump();
}
} // namespace GameApi