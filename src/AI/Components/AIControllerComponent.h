#pragma once
#include "Core/ECS/Component.h"

class AIControllerComponent : public Component {
public:
    // add modifier set later

    // std::string personalityId;
    // std::unordered_map<std::string, float> dynamicDesires;

    AIControllerComponent() = default;

    std::string GetComponentType() const override { return "AIControllerComponent"; }

    // ToJson and UpdateFromJson
    nlohmann::json ToJson() const override {
        return nlohmann::json::object();
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        // Currently no persistent state to load
    }
    

    // hookable functions
    // void addDesireModifier(const std::string& action, float amount) {
    //     dynamicDesires[action] += amount;
    // }
};