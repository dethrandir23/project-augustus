#pragma once
#include "Core/ECS/Component.h"
#include "AI/AIBrain.h"
#include <memory>

class AIControllerComponent : public Component {
public:
    AIControllerComponent() = default;

    explicit AIControllerComponent(std::unique_ptr<AIBrain> brain)
        : brain(std::move(brain)) {}

    std::string GetComponentType() const override { return "AIControllerComponent"; }

    AIBrain* getBrain() const { return brain.get(); }

    void setBrain(std::unique_ptr<AIBrain> b) { brain = std::move(b); }

    bool hasBrain() const { return brain != nullptr; }

    float riskTolerance = 0.5f;
    float aggression = 0.5f;
    float expansionism = 0.5f;

    nlohmann::json ToJson() const override {
        auto j = nlohmann::json::object();
        j["riskTolerance"] = riskTolerance;
        j["aggression"] = aggression;
        j["expansionism"] = expansionism;
        if (brain) {
            j["brain"] = brain->toJson();
        }
        return j;
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        if (j.contains("riskTolerance")) riskTolerance = j["riskTolerance"].get<float>();
        if (j.contains("aggression")) aggression = j["aggression"].get<float>();
        if (j.contains("expansionism")) expansionism = j["expansionism"].get<float>();
    }

private:
    std::unique_ptr<AIBrain> brain;
};
