#pragma once
#include "AI/AIBrain.h"
#include "Core/GameConstants.h"
#include "Game/IdUtils.h"
#include <random>

class TradeNodeBrain : public AIBrain {
public:
    TradeNodeBrain();

    explicit TradeNodeBrain(unsigned int seed);

    std::string getType() const override { return "trade_node"; }

    void execute(Entity& entity, Gamestate& gamestate) override;

    nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json& j) override;

    // --- Configurable Parameters ---
    float sellRatio = GameConstants::SELL_RATIO;
    float budgetBaseRatio = GameConstants::BUDGET_BASE_RATIO;
    float budgetHappinessWeight = GameConstants::BUDGET_HAPPINESS_WEIGHT;
    float richThreshold = GameConstants::TRADE_RICH_THRESHOLD;
    unsigned int seed = 42;

private:
    std::mt19937 rng;

    uuids::uuid getMarketId(Entity& node) const;
    nlohmann::json makeInput(const std::string& type,
                             const nlohmann::json& payload,
                             const Entity& node) const;

    void sellSurplus(Entity& node, Gamestate& gamestate);
    void buyConsumptionNeeds(Entity& node, Gamestate& gamestate);
    void handleEvents(Entity& node, Gamestate& gamestate);
};
