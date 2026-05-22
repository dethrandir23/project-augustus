#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../Core/Types.h" // ItemStack burada tanımlı varsayıyorum
#include <vector>
#include <string>
#include <algorithm>
#include <optional>

class Inventory {
public:
    Inventory() = default;
    Inventory(double maxCapacity) : maxWeight(maxCapacity) {}

    // --- Core Operations ---

    // Eşya Ekleme
    void add(const std::string& itemId, double amount) {
        // Varsa üzerine ekle
        for (auto& item : items) {
            if (item.id == itemId) { // ItemStack içinde id string varsayıyorum
                item.quantity += amount;
                return;
            }
        }
        // Yoksa yeni oluştur
        items.push_back({itemId, static_cast<float>(amount)}); // ItemStack constructor'ına göre değişebilir
    }

    // Eşya Çıkarma (Yeterince varsa true döner ve siler)
    bool remove(const std::string& itemId, double amount) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->id == itemId) {
                if (it->quantity >= amount) {
                    it->quantity -= amount;
                    if (it->quantity <= 0.0001) { // Floating point hatası olmasın diye
                        items.erase(it); // Miktar bittiyse listeden sil
                    }
                    return true;
                }
                return false; // Yetersiz bakiye
            }
        }
        return false; // Hiç yok
    }

    // Miktar Sorgulama
    double getAmount(const std::string& itemId) const {
        for (const auto& item : items) {
            if (item.id == itemId) return item.quantity;
        }
        return 0.0;
    }

    bool has(const std::string& itemId, double amount) const {
        return getAmount(itemId) >= amount;
    }

    // Direkt Erişim (UI vs için)
    const std::vector<ItemStack>& getAllItems() const { return items; }
    
    // Temizle
    void clear() { items.clear(); }
    
    // Weight Management
    void setMaxWeight(double weight) { maxWeight = weight; }
    double getMaxWeight() const { return maxWeight; }

    // --- Serialization (En büyük rahatlık burada) ---
    friend void to_json(nlohmann::json& j, const Inventory& inv) {
        j["items"] = nlohmann::json::array();
        for (const auto& item : inv.items) {
            j["items"].push_back({
                {"id", item.id},
                {"amount", item.quantity}
            });
        }
        j["max_weight"] = inv.maxWeight;
    }

    friend void from_json(const nlohmann::json& j, Inventory& inv) {
        inv.items.clear();
        if (j.contains("items")) {
            for (const auto& itemJson : j["items"]) {
                std::string iId = itemJson.at("id").get<std::string>();
                double iAmt = itemJson.at("amount").get<double>();
                inv.add(iId, iAmt);
            }
        }
        inv.maxWeight = j.value("max_weight", 0.0);
    }

private:
    std::vector<ItemStack> items;
    double maxWeight = 0.0;
};