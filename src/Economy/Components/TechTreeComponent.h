#pragma once
#include "Core/ECS/Component.h"
#include <vector>
#include <string>
#include <algorithm>

class TechTreeComponent : public Component {
public:
    std::vector<std::string> knownTechnologies;

    std::string GetComponentType() const override { return "TechTreeComponent"; }

    void unlock(const std::string& techId) {
        if (!isUnlocked(techId)) {
            knownTechnologies.push_back(techId);
        }
    }

    bool isUnlocked(const std::string& techId) const {
        return std::find(knownTechnologies.begin(), knownTechnologies.end(), techId) != knownTechnologies.end();
    }
};