/**
 * @file NativeApi.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief The native API Interface.
 * @details This file contains the API for native C++ applications.
 * @version 0.3 (Refactored & Logged)
 * @date 2026-02-04
 */
#pragma once
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
#include "../Registry/FactoryManager.h"
#include "../Registry/ItemManager.h"
#include "../Registry/MapManager.h"
#include "../Registry/NameManager.h"
#include "../Registry/PerkManager.h"
#include "../Registry/PipelineManager.h"
#include "../Registry/ScenarioManager.h"
#include "../Registry/TechnologyManager.h"
#include "../Registry/TradeNodeManager.h"
#include "Economy/Company.h"
#include "Game/IdUtils.h"

namespace GameApi {

// Singleton Instances
extern Gamestate globalGamestate;
extern GameLoader loader;

/**
 * @brief Initializes the engine and registers all JSON handlers.
 * @details Call this ONCE at the start of the application.
 */
void initEngine();

/**
 * @brief Loads game data files (Mods, Core Data) from JS.
 */
bool loadGameFiles(const std::vector<std::string> &file_contents,
                   const std::vector<std::string> &file_names);

/**
 * @brief Starts a new game by loading a specific scenario.
 * @param scenarioId The ID of the scenario to load (e.g., "debug_scenario").
 * @return true if successful.
 */
bool startScenario(const std::string &scenarioId);

void SetPlayer(const std::string &companyName, const std::string &companyId);

/**
 * @brief Advances the simulation by one tick.
 */
void step();
/**
 * @brief Returns the full game state as JSON.
 */
std::string getSerializedState();

/**
 * @brief Handles player input (CLI commands, etc.)
 */
bool sendInput(const std::string &input);

void logToConsole(const std::string &message, LogType type);

std::vector<std::string> readConsole();

void subscribeToEvents();

std::string getPendingEvents();

}

