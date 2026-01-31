/**
 * @file GameManager.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of the main game loop logic.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "GameManager.h"
#include "Gamestate.h"
#include "../Economy/Company.h"
#include "Game/IdUtils.h"

void GameManager::stepGamestate(Gamestate &gamestate) {
    
    // ==========================================
    // STEP 1: Market Preparation
    // ==========================================
    // Clean up obsolete demand/supply signals and optimize inventory stacks from the previous turn.
    for (auto &[id, market] : gamestate.getMarkets()) {
        market.optimiseMarket();
    }

    // ==========================================
    // STEP 2: Production Cycle
    // ==========================================
    // Companies distribute resources to factories and trigger production.
    for (auto &[id, company] : gamestate.getCompanies()) {
        // 2.1: Logistics - Move Manpower and Resources to Factories
        company.fillEmployeesOfFactories(gamestate);
        company.fillResourcesOfFactories(gamestate);
        
        // 2.2: Operation - Process inputs into outputs based on efficiency
        company.processFactories(gamestate);
        
        // 2.3: Collection - Gather finished goods back to Company HQ
        company.collectOutputs(gamestate);
    }

    // ==========================================
    // STEP 3: World Consumption (Population Needs)
    // ==========================================
    // TradeNodes (Cities) consume resources to sustain population.
    // They buy from the market if local stocks are insufficient.
    for (auto &[id, node] : gamestate.getNodes()) {
        Market* market = gamestate.getMarket(node.getMarketId());
        if (market) {
            // This triggers Buy Pressure on the market
            node.ConsumePopulationResources(*market);
        }
    }

    // ==========================================
    // STEP 4: Company Trading
    // ==========================================
    // Companies (Player & AI) execute buy/sell orders.
    // @note This phase can also be triggered via InputHandler for player actions.
    // Current Implementation: Companies sell surplus and buy shortages in their logic loops 
    // or through explicit AI controllers (to be implemented).

    // ==========================================
    // STEP 5: Market Dynamics (Price Calculation)
    // ==========================================
    // Recalculate prices based on the Supply (Step 2 & 4) and Demand (Step 3 & 4) generated this turn.
    // "sellPressure" and "buyPressure" are now at their most accurate for this tick.
    for (auto &[id, market] : gamestate.getMarkets()) {
        market.calculateMarketPrices();
    }

    // ==========================================
    // STEP 6: Financial Maintenance
    // ==========================================
    // Wages are paid, debts are serviced, and financial reports are updated.
    for (auto &[id, company] : gamestate.getCompanies()) {
        // @todo Implement: company.payWages();
        // @todo Implement: company.updateFinancials();
    }
}