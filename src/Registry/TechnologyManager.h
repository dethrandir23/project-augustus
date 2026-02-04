#pragma once

#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using json = nlohmann::json;

struct TechEffect {
    std::string type;
    std::string category;
    float value;
};

struct TechData {
    std::string id;
    std::string name;
    std::vector<std::string> categories;
    int tier;
    int research_points;
    bool unlocked_at_start;
    
    std::vector<std::string> requires_tech_ids;
    std::vector<std::string> unlocks_condition_ids;
    
    std::vector<TechEffect> effects;
};

class TechnologyManager {
public:
    static inline std::unordered_map<std::string, TechData> techs;

    static void load_from_json(const json& j, const std::string& source) {
        try {
            
            auto entryList = j.contains("entries") ? j["entries"] : json::array();

            if (entryList.is_array()) {
                for (const auto& entry : entryList) {
                    parse_tech_entry(entry);
                }
            } 
            else if (entryList.is_object()) {
                for (auto& [key, value] : entryList.items()) {
                    json entry = value; 
                    entry["id"] = key; 
                    parse_tech_entry(entry);
                }
            }

        } catch (const std::exception& e) {
             // std::cerr << "Tech Load Error: " << e.what() << std::endl;
        }
    }

private:
    static void parse_tech_entry(const json& entry) {
        TechData t;
        t.id = entry.at("id").get<std::string>();
        t.name = entry.value("name", "Unknown Tech");
        t.categories = entry.value("categories", std::vector<std::string>{});
        t.tier = entry.value("tier", 1);
        t.research_points = entry.value("research_points", 0);
        t.unlocked_at_start = entry.value("unlocked_at_start", false);
        t.requires_tech_ids = entry.value("requires", std::vector<std::string>{});
        t.unlocks_condition_ids = entry.value("unlocks", std::vector<std::string>{});

        // Effect Parsing
        if (entry.contains("effects")) {
            for (const auto& eff : entry["effects"]) {
                TechEffect effect;
                effect.type = eff.value("type", "unknown");
                effect.category = eff.value("category", "");
                effect.value = eff.value("value", 0.0f);
                t.effects.push_back(effect);
            }
        }

        techs[t.id] = t;
    }
};