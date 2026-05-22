#pragma once
#include "AI/AIBrain.h"
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
    float sellRatio = 0.5f;
    float budgetBaseRatio = 0.3f;
    float budgetHappinessWeight = 0.4f;
    float richThreshold = 5000.0f;
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
