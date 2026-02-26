#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../Core/Types.h"
#include "Registry/EventManager.h"
#include <string>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;

struct CompanyTemplate {
    std::string id;
    // namespace (isim havuzu) şimdilik string, sonra NameGenerator'a bağlarız
    std::string name_pool_id; 
    
    size_t start_manpower;
    double start_capital;
    std::vector<std::string> start_techs;
    std::vector<ItemStack> start_inventory;
    std::vector<std::string> start_perks;
    // TODO: add start factories
};

class CompanyManager {
public:
    static inline std::unordered_map<std::string, CompanyTemplate> templates;

    static void load_from_json(const json& j, const std::string& source) {
        try {
            for (const auto& entry : j.at("entries")) {
                CompanyTemplate t;
                t.id = entry.at("id").get<std::string>();
                t.name_pool_id = entry.value("namespace", "default");
                t.start_manpower = entry.value("manpower", 100);
                t.start_capital = entry.value("capital", 1000.0);
                t.start_techs = entry.value("technologies", std::vector<std::string>{});
                t.start_perks = entry.value("perks", std::vector<std::string>{});

                // Inventory parsing (core_id -> id mapping)
                if(entry.contains("inventory")) {
                    for(const auto& item : entry["inventory"]) {
                        t.start_inventory.push_back({
                            item.at("id").get<std::string>(),
                            item.at("amount").get<float>()
                        });
                    }
                }
                
                templates[t.id] = t;
            }
        } catch (const std::exception& e) {
             // std::cerr << "Company Template Load Error: " << e.what() << std::endl;
        }
    }
};