#pragma once

#include "../../lib/nlohmann/json.hpp"
#include <string>

// Global ekonomi sabitleri
struct EconomyConstants {
    size_t price_min;
    size_t price_max;
    double recruit_base;
};

class EconomyManager {
public:
    static inline EconomyConstants data; // Global erişim noktası

    static void load_from_json(const nlohmann::json &j, const std::string &source) {
        try {
            // "entries" objesinin içindeki "price" objesi
            if(j.contains("entries") && j["entries"].contains("price")) {
                auto price_data = j["entries"]["price"];
                data.price_min = price_data.value("min", 1);
                data.price_max = price_data.value("max", 10000);
                data.recruit_base = price_data.value("recruit_base", 1000.0);
            }
        } catch (std::exception &e) {
            // Loglama sistemi buraya
        }
    }
};