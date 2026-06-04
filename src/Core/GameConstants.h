#pragma once
#include <cstddef>

namespace GameConstants {

// === CompanyBrain ===
inline constexpr float SELL_THRESHOLD = 10.0f;
inline constexpr float INVEST_THRESHOLD = 2000.0f;
inline constexpr float INVEST_DIVISOR = 2000.0f;
inline constexpr float INVEST_MIN_SCORE = 2.0f;
inline constexpr float HIRING_COST_PER_WORKER = 1.0f;
inline constexpr float HIRE_BUDGET_RATIO = 0.5f;
inline constexpr float BUY_BUDGET_RATIO = 0.3f;
inline constexpr float MIN_BUY_THRESHOLD = 0.1f;
inline constexpr double FALLBACK_PRICE = 1.0;
inline constexpr int PICKER_TOP_K = 3;
inline constexpr double PICKER_TEMPERATURE = 0.7;
inline constexpr float AI_NOISE = 0.1f;

// === TradeNodeBrain ===
inline constexpr float SELL_RATIO = 0.5f;
inline constexpr float MIN_SELL_UNIT = 1.0f;
inline constexpr float BUDGET_BASE_RATIO = 0.3f;
inline constexpr float BUDGET_HAPPINESS_WEIGHT = 0.4f;
inline constexpr float TRADE_RICH_THRESHOLD = 5000.0f;
inline constexpr float TRADE_INPUT_BUFFER = 5.0f;
inline constexpr float MIN_BUY_UNIT = 0.01f;
inline constexpr float MIN_BUDGET = 1.0;

// === EconomyEvaluator ===
inline constexpr float MISSING_TEMPLATE_SCORE = -9999.0f;
inline constexpr float NO_WALLET_SCORE = -1.0f;
inline constexpr float DEMAND_BONUS_SCALE = 0.01f;
inline constexpr float DEMAND_BONUS_CAP = 1.0f;
inline constexpr float ROI_SCALE = 100.0f;
inline constexpr float DEBT_PENALTY_DIVISOR = 500.0f;
inline constexpr float FIRST_FACTORY_BONUS = 1.0f;

// === GameManager ===
inline constexpr size_t WORKERS_PER_TICK = 100;
inline constexpr float POP_GROWTH_COEFF = 0.0001f;
inline constexpr float RECRUITABLE_POP_RATIO = 0.4f;
inline constexpr float HAPPINESS_REQUIRED_RATIO = 0.01f;
inline constexpr float GLOBAL_PRODUCTION_MODIFIER = 1.0f;

// === EconomyUtils ===
inline constexpr float INPUT_BUFFER_MULTIPLIER = 5.0f;
inline constexpr float MIN_WORKER_EFFICIENCY = 0.001f;
inline constexpr double LABOR_POINTS_PER_WORKER = 10.0;
inline constexpr double LABOR_POINTS_PER_POP = 0.3;

// === MarketSystem ===
inline constexpr double REFUND_EPSILON = 0.0001;

// === InputHandler ===
inline constexpr double FAKE_DEMAND_PRICE_MULT = 2.0;
inline constexpr double CANCEL_PRICE_EPSILON = 0.001;

}
