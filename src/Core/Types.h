/**
 * @file Types.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Common data structures and JSON serialization for the game "Producer".
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <utility>

// --- ÖNEMLİ TİP TANIMI ---
// İleride string yerine Hash ID'ye geçmek istersek sadece burayı değiştireceğiz.
using ItemId = std::string; 

using Position = std::pair<int, int>;

/**
 * @brief THE UNIFIED ITEM STRUCTURE.
 * Replaces both 'Resource' and partial 'Product' logic.
 * Represents a bundle of items (e.g., "100x Wood").
 */
struct ItemStack {
  ItemId id;       ///< The string ID of the item (e.g., "core_wood_001")
  float quantity;  ///< How much of it we have.
};

/**
 * @brief Used for Market Listings where price matters per instance.
 * Replaces the old 'Product' struct effectively.
 */
struct MarketListing {
    ItemStack item; ///< Contains ID and Quantity
    double price;   ///< Unit price for this specific listing
};

struct GameEffect {
    std::string type;     // Örn: "debt_interest", "production_efficiency"
    std::string category; // Örn: "agriculture" (veya boş "all")
    float value;          // Örn: 0.10 (+%10) veya -0.05 (-%5)
    std::string mode;     // "additive" (topla) veya "multiplicative" (çarp) - Default: additive
};

// --- JSON SERIALIZATION ---

inline void from_json(const nlohmann::json& j, GameEffect& e) {
    j.at("type").get_to(e.type);
    e.value = j.at("value").get<float>();
    e.category = j.value("category", "all");
    e.mode = j.value("mode", "additive");
}

struct ActivePerk {
    std::string perkId;
    int remainingDuration; // -1 never ends

    std::string to_string() const {
      return perkId;
    }
};

// 1. Position
inline void to_json(nlohmann::json &j, const Position &p) {
  j = nlohmann::json{{"x", p.first}, {"y", p.second}};
}
inline void from_json(const nlohmann::json &j, Position &p) {
  p.first = j.at("x").get<int>();
  p.second = j.at("y").get<int>();
}

// 2. ItemStack (Eski Resource yerine artık bunu kullanacağız her yerde)
inline void to_json(nlohmann::json &j, const ItemStack &i) {
  j = nlohmann::json{
      {"id", i.id},
      {"quantity", i.quantity}
  };
}
inline void from_json(const nlohmann::json &j, ItemStack &i) {
  j.at("id").get_to(i.id);
  j.at("quantity").get_to(i.quantity);
}

// 3. MarketListing (Eski Product yerine, sadece markette lazım)
inline void to_json(nlohmann::json &j, const MarketListing &m) {
  j = nlohmann::json{
      {"item", m.item}, // Nested serialization kullanır (yukarıdaki ItemStack to_json'u çağırır)
      {"price", m.price}
  };
}
inline void from_json(const nlohmann::json &j, MarketListing &m) {
  j.at("item").get_to(m.item);
  j.at("price").get_to(m.price);
}

inline void to_json(nlohmann::json &j, const ActivePerk &a) {
    j = nlohmann::json{
        {"perk_id", a.perkId},
        {"remaining_duration", a.remainingDuration}
    };
}

inline void from_json(const nlohmann::json &j, ActivePerk &a) {
    j.at("perk_id").get_to(a.perkId);
    j.at("remaining_duration").get_to(a.remainingDuration);
}