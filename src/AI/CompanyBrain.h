#pragma once
#include "AI/AIBrain.h"
#include "Core/GameConstants.h"
#include <random>

class CompanyBrain : public AIBrain {
public:
    CompanyBrain();

    CompanyBrain(unsigned int seed);

    std::string getType() const override { return "company"; }

    void execute(Entity& entity, Gamestate& gamestate) override;

    void loadProfile(const std::string& profileId);

    nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json& j) override;

    // --- Profile ---
    std::string profileId = "balanced";

    // --- Configurable Parameters ---
    float sellThreshold = GameConstants::SELL_THRESHOLD;
    float investThreshold = GameConstants::INVEST_THRESHOLD;
    float investDivisor = GameConstants::INVEST_DIVISOR;
    float investMinScore = GameConstants::INVEST_MIN_SCORE;
    float hiringCostPerWorker = GameConstants::HIRING_COST_PER_WORKER;
    float noiseLevel = GameConstants::AI_NOISE;    // 0.0 = perfect, 1.0 = chaotic
    int pickerTopK = GameConstants::PICKER_TOP_K;
    double pickerTemperature = GameConstants::PICKER_TEMPERATURE;
    unsigned int seed = 42;

private:
    std::mt19937 rng;

    void manageDebt(Entity& company, Gamestate& gamestate);
    void hireWorkers(Entity& company, Gamestate& gamestate);
    void buyInputs(Entity& company, Gamestate& gamestate);
    void buildFactories(Entity& company, Gamestate& gamestate);
    void sellSurplus(Entity& company, Gamestate& gamestate);
    void handleEvents(Entity& company, Gamestate& gamestate);

    nlohmann::json makeInput(const std::string& type,
                             const nlohmann::json& payload,
                             const Entity& company) const;
};
