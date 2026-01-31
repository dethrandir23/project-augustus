/**
 * @file GameApi.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief The WebAssembly API Interface.
 * @details This file contains the bindings exposed to the JavaScript frontend
 * via Emscripten. It manages the global gamestate instance and provides entry
 * points for game control.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "../../lib/nlohmann/json.hpp"
#include "../Economy/Company.h"
#include "../Economy/Factory.h"
#include "../Economy/Templates.h"
#include "../World/Market.h"
#include "../World/TradeNode.h"
#include "Game/IdUtils.h"
#include "GameManager.h"
#include "Gamestate.h"
#include <emscripten/bind.h>
#include <iostream>
#include <unordered_map>
#include "InputHandler.h"
#include "../Registry/ItemManager.h"
#include "../Registry/GameLoader.h"

/**
 * @brief The singleton instance of the game world.
 * @details Created globally to persist state across JS function calls.
 */
static Gamestate globalGamestate;
static GameLoader loader;

void initEngine() {
  loader.RegisterHandler("ITEM_DEFINITIONS", [](const nlohmann::json &j, const std::string& src){
    ItemManager::load_from_json(j, src);
  });
}

bool loadGameFiles(const std::vector<std::string> &file_contents, const std::vector<std::string> &file_names) {
  for (size_t i = 0; i < file_contents.size(); ++i) {
    if (!loader.load_file_content(file_contents[i], file_names[i])) {
      //std::cerr << "Error loading file: " << file_names[i] << std::endl;
      return false;
    }
  }
  return true;
}

/**
 * @brief API Endpoint: Retrieves the current game state as a JSON string.
 * @return std::string JSON representation of the world.
 */
std::string getSerializedState() { return globalGamestate.serialize(); }

/**
 * @brief API Endpoint: Advances the simulation by one turn.
 */
void step() { GameManager::stepGamestate(globalGamestate); }

/**
 * @brief API Endpoint: Initializes a blank game with a player profile.
 * @param playerName The name of the player's company.
 */
void startNewGame(const std::string &playerName) {
  globalGamestate.setPlayerCompanyId(
      globalGamestate.createPlayerCompany(playerName));
}

/**
 * @brief API Endpoint: Sets up a predefined scenario for testing purposes.
 * @details Creates a company, a central market, a city (Constantinople),
 * and gives the player a Sawmill with some starting resources.
 * @param playerName The name of the player's company.
 */
void startNewDebugGame(std::string playerName) {
  // 1. Create the Player Company
  uuids::uuid pId = globalGamestate.createPlayerCompany(playerName);
  Company *player = globalGamestate.getCompany(pId);

  // 2. Create a Central Market
  uuids::uuid marketId = IdUtils::generateUuid();
  Market centralMarket(marketId, "Central Market");

  // Seed market with initial resources to prevent zero-price issues
  centralMarket.addResource({ItemType::WOOD, 100.0f});
  centralMarket.addResource({ItemType::COAL, 50.0f});

  // 3. Create a Trade Node (City)
  TradeNodeTemplate cityTmpl;
  cityTmpl.name = "Constantinople";
  cityTmpl.population = 1000;
  cityTmpl.recruitablePopulationRatio = 0.5f;

  // Define City Demands
  cityTmpl.likelyDemandedResources.push_back(
      {Resource{ItemType::WOOD, 0.0f}, 0.1f});
  cityTmpl.likelyDemandedProducts.push_back(
      {Product{ItemType::LUMBER, 0.0, 0.0f}, 0.05f});

  TradeNode city = makeTradeNode(cityTmpl, marketId);
  uuids::uuid cityId = city.getId();

  // 4. Assign a Factory to the Player (Sawmill)
  uuids::uuid factoryId = IdUtils::generateUuid();
  Factory sawmill("Player Sawmill", factoryId, pId);

  // Apply Template Pipelines
  for (const auto &p : DefaultFactoryTemplates::SAWMILL.pipelines) {
    sawmill.addPipeline(p);
  }
  sawmill.addEmployees(20); // Start with 20 employees

  // 5. Register everything to the global Gamestate
  globalGamestate.addMarket(marketId, centralMarket);
  globalGamestate.addTradeNode(cityId, city);
  globalGamestate.addFactory(factoryId, sawmill);

  // Link factory to company
  player->addFactory(factoryId);

  // Grant starting resources to the player (for production)
  player->addResource({ItemType::WOOD, 50.0f});
}

bool sendInput(const std::string &input) {
  return InputHandler::handleInput(globalGamestate, input);
}

/**
 * @brief Emscripten Binding Definitions.
 * @details Maps C++ functions to JavaScript callable names.
 */
EMSCRIPTEN_BINDINGS(my_game_module) {
  emscripten::function("initEngine", &initEngine);
  emscripten::function("loadGameFiles", &loadGameFiles);
  emscripten::function("getSerializedState", &getSerializedState);
  emscripten::function("step", &step);
  emscripten::function("startNewGame", &startNewGame);
  emscripten::function("startNewDebugGame", &startNewDebugGame);
  emscripten::function("sendInput", &sendInput);
  emscripten::register_vector<std::string>("StringList");
};