#pragma once
#include "../../lib/nlohmann/json.hpp"
#include "../Core/Types.h"
#include "../DevTools/Console.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

struct TradeNodeTemplate {
    std::string id;
    std::string name_pool_id;
    int min_pop;
    int max_pop;
    int initial_capital;
    std::vector<std::string> local_pipelines;
    std::vector<std::string> consumption_pipelines;
    std::vector<ItemStack> start_inventory;
};

class TradeNodeManager {
public:
    static inline std::unordered_map<std::string, TradeNodeTemplate> templates;

    static void load_from_json(const nlohmann::json& j, const std::string& src) {
        try {
            for (const auto& entry : j.at("entries")) {
                TradeNodeTemplate t;
                t.id = entry.at("id").get<std::string>();
                t.name_pool_id = entry.value("name_pool", "default_names");
                t.initial_capital = entry.value("initial_capital", 0);

                if (entry["initial_population"].is_object()) {
                    t.min_pop = entry["initial_population"].value("min", 100);
                    t.max_pop = entry["initial_population"].value("max", 100);
                } else {
                    t.min_pop = t.max_pop = entry.value("initial_population", 100);
                }

                t.local_pipelines = entry.value("local_pipelines", std::vector<std::string>{});
                t.consumption_pipelines = entry.value("consumption_profile", std::vector<std::string>{});

                if (entry.contains("starting_inventory")) {
                    auto& si = entry["starting_inventory"];
                    if (si.is_object()) {
                        for (auto it = si.begin(); it != si.end(); ++it) {
                            t.start_inventory.push_back({it.key(), it.value().get<float>()});
                        }
                    } else if (si.is_array()) {
                        for (const auto& item : si) {
                            t.start_inventory.push_back({
                                item.at("id").get<std::string>(),
                                item.at("amount").get<float>()
                            });
                        }
                    }
                }

                templates[t.id] = t;
            }
            Console::log("Loaded " + std::to_string(templates.size()) + " trade node templates from " + src, LogType::INFO);
        } catch (const std::exception& e) {
            Console::log("TradeNode load error in " + src + ": " + e.what(), LogType::ERROR);
        }
    }
};