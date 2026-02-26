/**
 * @file Types.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Common data structures and JSON serialization for the game "Producer".
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once
#include "nlohmann/json.hpp"
#include "uuid/uuid.h"
#include "Game/IdUtils.h"
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

enum class OrderType { BUY, SELL };

// Bir Emir (Order) Neye Benzer?
struct MarketOrder {
    uuids::uuid id;         // Emrin benzersiz kimliği
    uuids::uuid ownerId;    // Emri veren Şirket veya Node ID'si
    std::string itemId;     // Hangi eşya?
    OrderType type;         // Alış/Satış
    double price;           // Limit Fiyat (Bundan kötü fiyata işlem yapmam)
    float quantity;         // Toplam miktar
    float filledQuantity;   // Ne kadarı gerçekleşti?
    long timestamp;         // Zaman damgası (Önce gelen önce alır)

    MarketOrder(uuids::uuid owner, std::string item, OrderType type, double price, float qty)
        : ownerId(owner), itemId(item), type(type), price(price), quantity(qty) 
    {
        // Otomatik doldurulanlar:
        this->id = IdUtils::generateUuid();
        this->filledQuantity = 0.0f;
        this->timestamp = std::time(nullptr); // Şu anki zaman
    }

    // Boş constructor (Serialization için lazım olabilir)
    MarketOrder() = default;

    // Helper: Tamamlandı mı?
    bool isComplete() const { return filledQuantity >= quantity; }
    // Helper: Kalan miktar
    float remaining() const { return quantity - filledQuantity; }
};

// --- JSON Serialization ---
NLOHMANN_JSON_SERIALIZE_ENUM( OrderType, {
    {OrderType::BUY, "BUY"},
    {OrderType::SELL, "SELL"},
})

inline void to_json(nlohmann::json& j, const MarketOrder& o) {
    j = nlohmann::json{
        {"id", uuids::to_string(o.id)},
        {"ownerId", uuids::to_string(o.ownerId)},
        {"itemId", o.itemId},
        {"type", o.type},
        {"price", o.price},
        {"quantity", o.quantity},
        {"filled", o.filledQuantity},
        {"time", o.timestamp}
    };
}

inline MarketOrder from_json(const nlohmann::json& j) {
    MarketOrder order;
    order.id = uuids::uuid::from_string(j.at("id").get<std::string>()).value();
    order.ownerId = uuids::uuid::from_string(j.at("ownerId").get<std::string>()).value();
    j.at("itemId").get_to(order.itemId);
    j.at("type").get_to(order.type);
    j.at("price").get_to(order.price);
    j.at("quantity").get_to(order.quantity);
    j.at("filled").get_to(order.filledQuantity);
    j.at("time").get_to(order.timestamp);
    
    return order;
}

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