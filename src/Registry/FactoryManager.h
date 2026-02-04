#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../Core/Types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using json = nlohmann::json;

struct FactoryData {
    std::string id;
    std::string name;
    int max_workers;
    std::vector<std::string> required_techs;
    std::vector<std::string> categories;
    std::vector<std::string> pipeline_ids;
};

class FactoryManager {
public:
    static inline std::unordered_map<std::string, FactoryData> factories;

    static void load_from_json(const json& j, const std::string& source) {
        try {
            for (const auto& entry : j.at("entries")) {
                FactoryData f;
                
                f.id = entry.at("id").get<std::string>();
                f.name = entry.at("name").get<std::string>();
                
                f.max_workers = entry.value("max_workers", 1000); 
                
                f.required_techs = entry.value("required_techs", std::vector<std::string>{});
                f.categories = entry.value("categories", std::vector<std::string>{});
                f.pipeline_ids = entry.value("pipelines", std::vector<std::string>{});

                factories[f.id] = f;
            }
        } catch (const std::exception& e) {
             std::cerr << "Factory Load Error in " << source << ": " << e.what() << std::endl;
        }
    }
};