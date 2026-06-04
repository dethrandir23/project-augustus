#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../../lib/nlohmann/json.hpp"

// Senaryodaki bir şirketin ayarları
struct ScenarioCompanyDef {
    std::string instance_id;
    std::string template_id;
    std::string name_override;
    double start_capital;
    std::string profile;
};

// Senaryodaki bir şehrin override ayarları
struct ScenarioNodeDef {
    std::string instance_id; // Map'teki instance_id ile eşleşmeli (örn: "core_debug_london")
    std::string market_id;   // Hangi pazara bağlı?
    int population_override;
    std::vector<std::string> extra_pipelines;
};

// Ana Senaryo Verisi
struct ScenarioData {
    std::string id;
    std::string name;
    std::string map_id; // "core_debug_map"
    std::string start_date;
    
    std::vector<ScenarioCompanyDef> companies;
    std::vector<ScenarioNodeDef> nodes;
};

class ScenarioManager {
public:
    // Senaryoları hafızada tutuyoruz (Seçim ekranı için)
    static inline std::unordered_map<std::string, ScenarioData> scenarios;

    static void load_from_json(const nlohmann::json &j, const std::string &source) {
        try {
            ScenarioData s;
            s.id = j.at("id").get<std::string>();
            s.name = j.at("name").get<std::string>();
            s.map_id = j.at("map_id").get<std::string>();
            s.start_date = j.value("start_date", "1836-01-01");

            // Şirketleri Yükle
            if (j.contains("companies")) {
                for (const auto& cEntry : j["companies"]) {
                    ScenarioCompanyDef c;
                    c.instance_id = cEntry.at("instance_id").get<std::string>();
                    c.template_id = cEntry.at("template_id").get<std::string>();
                    c.name_override = cEntry.value("name", "");
                    c.start_capital = cEntry.value("capital", 0.0);
                    c.profile = cEntry.value("profile", "balanced");
                    s.companies.push_back(c);
                }
            }

            // Node Override'ları Yükle
            if (j.contains("nodes")) {
                for (const auto& nEntry : j["nodes"]) {
                    ScenarioNodeDef n;
                    n.instance_id = nEntry.at("instance_id").get<std::string>();
                    n.market_id = nEntry.value("market_id", "");
                    n.population_override = nEntry.value("population", -1); // -1 = Elleme
                    n.extra_pipelines = nEntry.value("additional_pipelines", std::vector<std::string>{});
                    s.nodes.push_back(n);
                }
            }

            scenarios[s.id] = s;
        } catch(...) { /* Log error */ }
    }
};