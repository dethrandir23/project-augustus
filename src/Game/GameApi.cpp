/**
 * @file GameApi.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief The WebAssembly API Interface.
 * @details This file contains the bindings exposed to the JavaScript frontend
 * via Emscripten. It connects the JS world to the C++ Engine Core.
 * @version 0.2 (Refactored)
 * @date 2026-02-04
 */

#include "../../lib/nlohmann/json.hpp"
#include <emscripten/bind.h>
#include <string>
#include <vector>

// Core Engine Components
#include "../Registry/GameLoader.h"
#include "GameManager.h"
#include "Gamestate.h"
#include "InputHandler.h"

// DevTools
#include "../DevTools/Console.h"

// All Managers (For Registration)
#include "../Registry/CompanyManager.h" // Template Manager
#include "../Registry/ConditionManager.h"
#include "../Registry/EconomyManager.h"
#include "../Registry/FactoryManager.h"
#include "../Registry/ItemManager.h"
#include "../Registry/MapManager.h"
#include "../Registry/NameManager.h"
#include "../Registry/PerkManager.h"
#include "../Registry/PipelineManager.h"
#include "../Registry/ScenarioManager.h"
#include "../Registry/TechnologyManager.h"
#include "../Registry/TradeNodeManager.h"

// Singleton Instances
static Gamestate globalGamestate;
static GameLoader loader;

/**
 * @brief Initializes the engine and registers all JSON handlers.
 * @details Call this ONCE at the start of the application.
 */
void initEngine() {
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

  // 4. Map & Scenario
  loader.RegisterHandler("MAP_DEFINITION",
                         [](const nlohmann::json &j, const std::string &src) {
                           MapManager::load_from_json(j, src);
                         });
  loader.RegisterHandler("SCENARIO_DEFINITION",
                         [](const nlohmann::json &j, const std::string &src) {
                           ScenarioManager::load_from_json(j, src);
                         });
}

/**
 * @brief Loads game data files (Mods, Core Data) from JS.
 */
bool loadGameFiles(const std::vector<std::string> &file_contents,
                   const std::vector<std::string> &file_names) {
  bool allSuccess = true;
  for (size_t i = 0; i < file_contents.size(); ++i) {
    if (!loader.load_file_content(file_contents[i], file_names[i])) {
      // Loglama sistemi olunca buraya yazarız
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
  return globalGamestate.loadScenario(scenarioId);
}

/**
 * @brief Loads the pixel data for the current map.
 * @details JS reads the image, passes the raw pixels here.
 */
void loadMapImage(int width, int height, const std::vector<uint32_t> &pixels) {
  // Gamestate içindeki Map objesini init et
  globalGamestate.getMap().init(globalGamestate.getMap().getId(), pixels);
}

/**
 * @brief Advances the simulation by one tick.
 */
void step() { GameManager::stepGamestate(globalGamestate); }

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
  return InputHandler::handleInput(globalGamestate, input);
}

/**
 * @brief Emscripten Binding Definitions.
 */
EMSCRIPTEN_BINDINGS(my_game_module) {
  // Core Functions
  emscripten::function("initEngine", &initEngine);
  emscripten::function("loadGameFiles", &loadGameFiles);
  emscripten::function("startScenario", &startScenario);
  emscripten::function("loadMapImage", &loadMapImage); // Harita resmi için yeni

  // Game Loop
  emscripten::function("step", &step);
  emscripten::function("getSerializedState", &getSerializedState);
  emscripten::function("sendInput", &sendInput);

  // Data Types
  emscripten::register_vector<Log>("LogList");
  emscripten::register_vector<std::string>("StringList");
  emscripten::register_vector<uint32_t>("PixelList");

  emscripten::enum_<LogType>("LogType")
      .value("INFO", LogType::INFO)
      .value("ERROR", LogType::ERROR)
      .value("WARNING", LogType::WARNING);

  // Struct registration
  emscripten::value_object<Log>("Log")
      .field("message", &Log::message)
      .field("logType", &Log::logType)
      .field("timestamp", &Log::timestamp);

  // Console class bindings
  emscripten::class_<Console>("Console")
      .class_function("help", &Console::help)
      .class_function("parseInput", &Console::parseInput)
      .class_function(
          "log",
          emscripten::select_overload<void(const std::string &)>(&Console::log))
      .class_function(
          "logWithType",
          emscripten::select_overload<void(const std::string &, LogType)>(
              &Console::log))
      .class_function("logObject",
                      emscripten::select_overload<void(Log)>(&Console::log))

      .class_function("readLog", &Console::readLog)
      .class_function("getLog", &Console::getLog)
      .class_function("clearLogs", &Console::clearLogs)
      .class_function("removeLog", &Console::removeLog)
      .class_function("getLogCount", &Console::getLogCount)
      .class_function("getLogs", &Console::getLogs)
      .class_function("getLogMessages", &Console::getLogMessages);
};