#pragma once
#include "Core/ECS/Component.h"

class AIControllerComponent : public Component {
public:
    // add modifier set later

    // std::string personalityId;
    // std::unordered_map<std::string, float> dynamicDesires;

    AIControllerComponent() = default;

    std::string GetComponentType() const override { return "AIControllerComponent"; }

    // hookable functions
    // void addDesireModifier(const std::string& action, float amount) {
    //     dynamicDesires[action] += amount;
    // }
};