#pragma once
#include "Core/ECS/Component.h"

class WorkforceComponent : public Component {
public:
    size_t currentWorkers = 0;
    size_t maxWorkers = 100;

    std::string GetComponentType() const override { return "WorkforceComponent"; }
    
    // İşçi alma mantığı buraya
    size_t recruit(size_t amount) {
        size_t space = maxWorkers - currentWorkers;
        size_t toAdd = std::min(amount, space);
        currentWorkers += toAdd;
        return toAdd;
    }
};