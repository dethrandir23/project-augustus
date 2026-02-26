#pragma once
#include "Core/ECS/Component.h"

class ManpowerPoolComponent : public Component {
public:
    size_t availableWorkers = 0;

    ManpowerPoolComponent(size_t start = 0) : availableWorkers(start) {}

    std::string GetComponentType() const override { return "ManpowerPoolComponent"; }

    void add(size_t count) { availableWorkers += count; }
    
    // İşçi fabrikaya gönderildiğinde havuzdan düşer
    bool allocate(size_t count) {
        if (availableWorkers >= count) {
            availableWorkers -= count;
            return true;
        }
        return false;
    }
};