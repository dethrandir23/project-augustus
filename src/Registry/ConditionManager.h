#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_map>

struct ConditionData {
    std::string id;
    std::vector<std::string> scope;
    std::string description;
};

class ConditionManager {
public:
    static inline std::unordered_map<std::string, ConditionData> conditions;

    static void load_from_json(const nlohmann::json& j, const std::string& src) {
        try {
            for (const auto& entry : j.at("entries")) {
                ConditionData c;
                c.id = entry.at("id").get<std::string>();
                c.description = entry.value("description", "");
                c.scope = entry.value("scope", std::vector<std::string>{});
                
                conditions[c.id] = c;
            }
        } catch(...) {}
    }
};