#pragma once
#include "Core/ECS/Component.h"
#include "../Core/Types.h"
#include <vector>

class PerkComponent : public Component {
public:
    std::vector<ActivePerk> activePerks;

    std::string GetComponentType() const override { return "PerkComponent"; }

    void addPerk(const std::string& id, int duration) {
        activePerks.push_back({id, duration});
    }

    void removePerk(const std::string& id) {
        activePerks.erase(std::remove_if(activePerks.begin(), activePerks.end(), [id](const ActivePerk& perk) {
            return perk.perkId == id;
        }), activePerks.end());
    };

};