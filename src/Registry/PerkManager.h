#pragma once
#include "../../lib/nlohmann/json.hpp"
#include "../Core/Types.h"
#include <string>
#include <vector>
#include <unordered_map>

struct PerkData {
    std::string id;
    std::string name;
    std::string description;
    
    int default_duration;
    
    std::vector<std::string> unlocks;
    std::vector<GameEffect> effects;
};

class PerkManager {
public:
    static inline std::unordered_map<std::string, PerkData> perks;

    static void load_from_json(const nlohmann::json& j, const std::string& src) {
        try {
            for (const auto& entry : j.at("entries")) {
                PerkData p;
                p.id = entry.at("id").get<std::string>();
                p.name = entry.value("name", "Unnamed Perk");
                p.description = entry.value("description", "");
                p.unlocks = entry.value("unlocks", std::vector<std::string>{});
                p.default_duration = entry.value("duration", -1);

                if (entry.contains("effects")) {
                    p.effects = entry.at("effects").get<std::vector<GameEffect>>();
                }
                
                perks[p.id] = p;
            }
        } catch(...) {}
    }
};