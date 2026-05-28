#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

struct MapNodeDef {
    std::string instance_id; // "core_london"
    std::string template_id; // "core_metropolis"
    std::string name;
    int x, y;
    std::string terrain;
    // Market ID'si senaryoda override edilebilir ama haritada default olabilir
    std::string default_market_id; 
};

struct MapDefinition {
    std::string id;
    std::string name;
    int width;
    int height;
    std::string texture_path; // "textures/map/provinces.png"
    
    std::vector<MapNodeDef> nodes;
};

class MapManager {
public:
    // Yüklenen tüm harita tanımları burada
    static inline std::unordered_map<std::string, MapDefinition> maps;

    static void load_from_json(const nlohmann::json& j, const std::string& src) {
        try {
            MapDefinition m;
            m.id = j.at("id").get<std::string>();
            m.name = j.at("name").get<std::string>();
            m.width = j.at("width").get<int>();
            m.height = j.at("height").get<int>();
            m.texture_path = j.at("background_texture").get<std::string>();

            if (j.contains("nodes")) {
                for (const auto& entry : j["nodes"]) {
                    MapNodeDef node;
                    node.instance_id = entry.at("instance_id").get<std::string>();
                    node.template_id = entry.at("template_id").get<std::string>();
                    node.name = entry.value("name", "Unknown Node");
                    node.x = entry.at("x").get<int>();
                    node.y = entry.at("y").get<int>();
                    node.terrain = entry.value("terrain", "plains");
                    node.default_market_id = entry.value("default_market_id", "");
                    
                    m.nodes.push_back(node);
                }
            }
            
            maps[m.id] = m;
            // std::cout << "Map Def Loaded: " << m.id << std::endl;
        } catch (const std::exception& e) {
             // std::cerr << "Map Load Error: " << e.what() << std::endl;
        }
    }
};