#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

#include "nlohmann/json.hpp"
#include "uuid/uuid.h"
#include "Core/Types.h"
#include "Economy/EconomyUtils.h"
#include "Registry/ItemManager.h"
#include "Registry/EconomyManager.h"
#include "Economy/Orderbook.h"

class Gamestate;

class Market {
public:
    // --- Constructors ---
    Market(const std::string &name) : name(name) {}
    Market(const uuids::uuid &id, const std::string &name) : id(id), name(name) {}

    // --- Core Properties ---
    void setId(const uuids::uuid &newId) { id = newId; }
    uuids::uuid getId() const { return id; }

    const std::string &getName() const { return name; }
    void setName(const std::string &newName) { name = newName; }

    // --- Node Management ---
    void addNode(const uuids::uuid &nodeId);
    void removeNode(const uuids::uuid &nodeId);

    // --- Price Logic ---
    double getPrice(const std::string& itemId) const;
    void calculateMarketPrices();

    // --- Transaction Logic ---
    /**
     * @brief Marketten ürün satın alma işlemi.
     * @param itemsToBuy İstenen eşyalar ve miktarları.
     * @param buyerInventory Alıcının deposu (Ürünler buraya eklenir).
     * @param buyerBudget Alıcının parası (Referans, buradan düşülür).
     * @return bool İşlem başarılıysa true. (Parası yetmezse veya stok yoksa false döner/kısmi alır)
     */
    bool buyItems(const std::vector<ItemStack>& itemsToBuy, Inventory& buyerInventory, double& buyerBudget);

    /**
     * @brief Markete ürün satma işlemi.
     * @param itemsToSell Satılacak eşyalar.
     * @param sellerInventory Satıcının deposu (Ürünler buradan silinir).
     * @param sellerBudget Satıcının parası (Kazanç buraya eklenir).
     */
    bool sellItems(const std::vector<ItemStack>& itemsToSell, Inventory& sellerInventory, double& sellerBudget);

    // --- Utilities ---
    void addStock(const std::string& itemId, float amount);
    void createArtificialDemand(const std::string& itemId, float amount);
    
    // JSON Serialization
    friend void to_json(nlohmann::json &j, const Market &m);

    std::unordered_map<std::string, double> getPrices() {
        return prices;
    }

    std::unordered_map<std::string, float> getBuyPressure() {
        return buyPressure;
    }

    std::unordered_map<std::string, float> getSellPressure() {
        return sellPressure;
    }

    Inventory getStorage() {
        return storage;
    }

    void placeOrder(Gamestate& gamestate, MarketOrder order);

    std::unordered_map<std::string, OrderBook> orderBooks;

private:

void executeTrade(Gamestate &gamestate, uuids::uuid buyerId, uuids::uuid sellerId, std::string itemId, double price, float qty, double refund);

    // --- Helpers ---
    float getDynamicSmoothing(float ratio);
    void decayPressures(float factor = 0.85f);

    // --- Data ---
    uuids::uuid id;
    std::string name;
    std::vector<uuids::uuid> registeredNodes; // Bu markete bağlı şehirler/fabrikalar

    // Stok ve Fiyatlar
    Inventory storage;
    
    std::unordered_map<std::string, double> prices;      // Güncel Fiyatlar
    std::unordered_map<std::string, float> buyPressure;  // Talep Basıncı
    std::unordered_map<std::string, float> sellPressure; // Arz Basıncı
};