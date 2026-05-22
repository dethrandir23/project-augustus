#pragma once
#include "AI/AIBrain.h"
#include <random>

class CompanyBrain : public AIBrain {
public:
    CompanyBrain();

    CompanyBrain(unsigned int seed);

    std::string getType() const override { return "company"; }

    void execute(Entity& entity, Gamestate& gamestate) override;

    nlohmann::json toJson() const override;

    void fromJson(const nlohmann::json& j) override;

    // --- Configurable Parameters ---
    float sellThreshold = 500.0f;
    float investThreshold = 5000.0f;
    float investMinScore = 0.5f;
    int pickerTopK = 3;
    double pickerTemperature = 0.7;
    unsigned int seed = 42;

private:
    std::mt19937 rng;

    void manageDebt(Entity& company, Gamestate& gamestate);
    void buildFactories(Entity& company, Gamestate& gamestate);
    void sellSurplus(Entity& company, Gamestate& gamestate);
    void handleEvents(Entity& company, Gamestate& gamestate);

    nlohmann::json makeInput(const std::string& type,
                             const nlohmann::json& payload,
                             const Entity& company) const;
};
