#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

class NameManager {
public:
    static inline std::unordered_map<std::string, std::vector<std::string>> namePools;

    static void load_from_json(const nlohmann::json& j, const std::string& src) {
        try {
            for (const auto& entry : j.at("entries")) {
                std::string id = entry.at("id").get<std::string>();
                std::vector<std::string> names = entry.at("names").get<std::vector<std::string>>();
                namePools[id] = names;
            }
        } catch(...) {}
    }

    // Rastgele isim çek
    static std::string getRandomName(const std::string& poolId) {
        if (namePools.find(poolId) == namePools.end() || namePools[poolId].empty()) {
            return "Unknown Company";
        }
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, namePools[poolId].size() - 1);
        
        return namePools[poolId][distr(gen)];
    }
};