/**
 * @file GameManager.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Static controller class for executing the main game loop.
 * @details This class acts as the "Driver" of the simulation, orchestrating the 
 * sequence of events (Production -> Consumption -> Market Adjustments) within a single turn.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once
#include "../../lib/nlohmann/json.hpp"
#include "../Economy/Company.h"
#include "../Economy/Factory.h"
#include "../World/Market.h"
#include "../World/TradeNode.h"
#include "Game/IdUtils.h"
#include <iostream>
#include <unordered_map>

class Gamestate;

/**
 * @class GameManager
 * @brief Static utility class to step the simulation forward.
 */
class GameManager {
public:
    /** * @brief Deleted constructor. This class is purely static and should not be instantiated. 
     */
    GameManager() = delete;

    /**
     * @brief Advances the simulation by one tick (turn).
     * @details Executes the following lifecycle:
     * 1. Market Optimization (Cleanup)
     * 2. Production Cycle (Companies produce goods)
     * 3. Consumption Cycle (Populations consume goods)
     * 4. Trade Execution
     * 5. Price Calculation (Supply/Demand updates)
     * 6. Financial Maintenance
     * * @param gamestate Reference to the current game world state.
     */
    static void stepGamestate(Gamestate &gamestate);
};