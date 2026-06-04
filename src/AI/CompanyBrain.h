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
    float sellThreshold = 10.0f;
    float investThreshold = 2000.0f;       // minimum rezerv
    float investDivisor = 2000.0f;         // her 2k fazla para = +1 desire
    float investMinScore = 2.0f;           // ROI bazli skorda esik
    float hiringCostPerWorker = 1.0f;      // isci basi ucret (dusuk tut, GameManager da bedava isci gonderiyor)
    int pickerTopK = 3;
    double pickerTemperature = 0.7;
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
