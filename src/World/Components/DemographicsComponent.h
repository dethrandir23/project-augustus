#pragma once
#include "Core/ECS/Component.h"
#include <algorithm>

class DemographicsComponent : public Component {
public:
    size_t population = 0;
    size_t recruitablePop = 0;
    float happiness = 0.5f;

    DemographicsComponent(size_t startPop) : population(startPop) {
        recruitablePop = static_cast<size_t>(startPop * 0.4);
    }

    std::string GetComponentType() const override { return "DemographicsComponent"; }

    size_t recruit(size_t count) {
        size_t available = std::min(count, recruitablePop);
        recruitablePop -= available;
        return available;
    }

    void grow(float rate) {
        int change = static_cast<int>(population * rate);

        if(change > 0) population += change;
    }

    nlohmann::json ToJson() const override {
        return {
            {"population", population},
            {"recruitablePop", recruitablePop},
            {"happiness", happiness}
        };
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        population = j.value("population", (size_t)0);
        recruitablePop = j.value("recruitablePop", (size_t)0);
        happiness = j.value("happiness", 0.5f);
    }
};