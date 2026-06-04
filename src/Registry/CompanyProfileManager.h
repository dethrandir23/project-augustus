#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../DevTools/Console.h"
#include <string>
#include <unordered_map>
#include <vector>

struct CompanyProfile {
    std::string id;
    std::string name;

    float noiseLevel = 0.2f;
    float investThreshold = 2000.0f;
    float investDivisor = 2000.0f;
    float investMinScore = 2.0f;

    std::unordered_map<std::string, float> categoryPreferences;

    float sellThreshold = 10.0f;
    float sellRatio = 0.5f;

    float factoryBudgetRatio = 0.3f;
};

class CompanyProfileManager {
public:
    static inline std::unordered_map<std::string, CompanyProfile> profiles;

    static void load_from_json(const nlohmann::json& j, const std::string& source) {
        try {
            for (const auto& entry : j.at("entries")) {
                CompanyProfile p;
                p.id = entry.at("id").get<std::string>();
                p.name = entry.value("name", p.id);

                p.noiseLevel = entry.value("noiseLevel", p.noiseLevel);
                p.investThreshold = entry.value("investThreshold", p.investThreshold);
                p.investDivisor = entry.value("investDivisor", p.investDivisor);
                p.investMinScore = entry.value("investMinScore", p.investMinScore);

                if (entry.contains("categoryPreferences")) {
                    for (auto it = entry["categoryPreferences"].begin(); it != entry["categoryPreferences"].end(); ++it) {
                        p.categoryPreferences[it.key()] = it.value().get<float>();
                    }
                }

                p.sellThreshold = entry.value("sellThreshold", p.sellThreshold);
                p.sellRatio = entry.value("sellRatio", p.sellRatio);
                p.factoryBudgetRatio = entry.value("factoryBudgetRatio", p.factoryBudgetRatio);

                profiles[p.id] = p;
            }
        } catch (const std::exception& e) {
            Console::log("CompanyProfile load error in " + source + ": " + e.what(), LogType::ERROR);
        }
    }
};
