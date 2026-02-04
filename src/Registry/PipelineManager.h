#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../Core/Types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using json = nlohmann::json;

struct PipelineData {
    std::string id;
    std::string name;
    std::vector<std::string> categories;

    std::vector<ItemStack> inputs; 
    std::vector<ItemStack> outputs;
};

class PipelineManager {
public:
    static inline std::unordered_map<std::string, PipelineData> pipelines;

    static void load_from_json(const json& j, const std::string& source) {
        try {
            for (const auto& entry : j.at("entries")) {
                PipelineData p;
                
                p.id = entry.at("id").get<std::string>();
                p.name = entry.at("name").get<std::string>();
                p.categories = entry.value("categories", std::vector<std::string>{});

                // INPUTS Parsing (Array -> Vector)
                if (entry.contains("inputs")) {
                    for (const auto& inp : entry["inputs"]) {
                        p.inputs.push_back({
                            inp.at("item_id").get<std::string>(),
                            inp.at("amount").get<float>()
                        });
                    }
                }

                // OUTPUTS Parsing
                if (entry.contains("outputs")) {
                    for (const auto& out : entry["outputs"]) {
                        p.outputs.push_back({
                            out.at("item_id").get<std::string>(),
                            out.at("amount").get<float>()
                        });
                    }
                }

                pipelines[p.id] = p;
                // std::cout << "Pipeline loaded: " << p.name << std::endl;
            }
        } catch (const std::exception& e) {
             //std::cerr << "Pipeline Load Error in " << source << ": " << e.what() << std::endl;
        }
    }
};