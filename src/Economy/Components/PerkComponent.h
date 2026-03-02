// PerkComponent.h
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

    nlohmann::json ToJson() const override {
        nlohmann::json j_list = nlohmann::json::array();
        for (const auto& perk : activePerks) {
                j_list.push_back({{"id", perk.perkId}, {"duration", perk.remainingDuration}});
            }
        return {{"activePerks", j_list}};
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        activePerks.clear();
        if (j.contains("activePerks") && j["activePerks"].is_array()) {
            for (const auto& item : j["activePerks"]) {
                activePerks.push_back({
                    item.value("id", ""), 
                    item.value("duration", 0)
                });
            }
        }
    }   

};