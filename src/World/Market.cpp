#include "Market.h"
#include "../Game/IdUtils.h"
#include <iostream>

// --- Node Management ---
void Market::addNode(const uuids::uuid &nodeId) {
    if (std::find(registeredNodes.begin(), registeredNodes.end(), nodeId) == registeredNodes.end()) {
        registeredNodes.push_back(nodeId);
    }
}

void Market::removeNode(const uuids::uuid &nodeId) {
    registeredNodes.erase(std::remove(registeredNodes.begin(), registeredNodes.end(), nodeId), registeredNodes.end());
}

// --- Price Logic ---
double Market::getPrice(const std::string& itemId) const {
    if (prices.count(itemId)) {
        return prices.at(itemId);
    }
    // Eğer fiyat kaydı yoksa, ItemManager'dan taban fiyatı öğren
    if (ItemManager::items.count(itemId)) {
        return static_cast<double>(ItemManager::items.at(itemId).base_price);
    }
    return static_cast<double>(EconomyManager::data.price_min);
}

void Market::calculateMarketPrices() {
    // Tüm bilinen itemlar üzerinden geç (Fiyat listesindekiler + Stoktakiler)
    // Not: Sadece fiyat listesini gezmek yeterli olmayabilir, o yüzden
    // ItemManager'daki tüm itemları gezmek en garantisidir (veya bilinenleri).
    // Performans için şimdilik 'prices' ve 'storage' birleşimi üzerinden gidelim.
    
    // Geçici set ile tüm unique item ID'lerini topla
    std::vector<std::string> allItems;
    for(const auto& [id, _] : prices) allItems.push_back(id);
    for(const auto& item : storage) allItems.push_back(item.id);
    
    // Unique hale getir
    std::sort(allItems.begin(), allItems.end());
    allItems.erase(std::unique(allItems.begin(), allItems.end()), allItems.end());

    for (const auto& itemId : allItems) {
        // Abstract (Soyut) itemların fiyatı hesaplanmaz (Mutluluk vb.)
        // ItemManager'dan kontrol edebilirsin: if (item.type == "abstract") continue;

        double currentPrice = getPrice(itemId);

        // Arz Hesabı (Market Stoğu + Satış Baskısı)
        float totalSupply = sellPressure[itemId] + EconomyUtils::getItemAmount(storage, itemId);

        // Talep Hesabı (Alış Baskısı)
        float totalDemand = buyPressure[itemId];

        // Oran
        float ratio = (totalSupply > 0.001f) ? (totalDemand / totalSupply) : totalDemand;
        ratio = std::log1p(ratio); // Logarithmic scaling

        // Sigmoid Curve (Senin algoritman)
        float k = 1.8f;
        float equilibrium = 1.0f;
        
        // Min/Max değerleri EconomyManager'dan alıyoruz
        double minP = static_cast<double>(EconomyManager::data.price_min);
        double maxP = static_cast<double>(EconomyManager::data.price_max);

        double target = minP + (maxP - minP) / (1.0 + std::exp(-static_cast<double>(k * (ratio - equilibrium))));

        // Smoothing
        float smoothing = getDynamicSmoothing(ratio);
        double newPrice = currentPrice + smoothing * (target - currentPrice);

        // Kaydet
        prices[itemId] = std::max(newPrice, minP);
    }

    // Basıncı azalt (Memory Decay)
    decayPressures();
}

float Market::getDynamicSmoothing(float ratio) {
    float v = std::abs(ratio - 1.0f);
    return std::clamp(0.2f / (1.0f + v), 0.02f, 0.2f);
}

void Market::decayPressures(float factor) {
    for (auto &[k, v] : buyPressure) v *= factor;
    for (auto &[k, v] : sellPressure) v *= factor;
}

// --- Transactions ---

bool Market::buyItems(const std::vector<ItemStack>& itemsToBuy, std::vector<ItemStack>& buyerInventory, double& buyerBudget) {
    for (const auto& req : itemsToBuy) {
        // Market stoğunda var mı?
        float availableQty = EconomyUtils::getItemAmount(storage, req.id);
        
        // Alınabilecek miktar (İstenen vs Var Olan)
        float quantityToBuy = std::min(req.quantity, availableQty);
        
        if (quantityToBuy > 0) {
            double unitPrice = getPrice(req.id);
            double totalCost = unitPrice * quantityToBuy;

            if (buyerBudget >= totalCost) {
                // 1. Marketten düş
                EconomyUtils::removeFromInventory(storage, req.id, quantityToBuy);
                
                // 2. Alıcıya ekle
                EconomyUtils::addToInventory(buyerInventory, req.id, quantityToBuy);
                
                // 3. Para transferi
                buyerBudget -= totalCost;
                
                // 4. Baskı (Demand) oluştur
                buyPressure[req.id] += quantityToBuy;
            }
        }
    }
    return true; // Kısmi alım olsa bile true döner
}

bool Market::sellItems(const std::vector<ItemStack>& itemsToSell, std::vector<ItemStack>& sellerInventory, double& sellerBudget) {
    for (const auto& offer : itemsToSell) {
        // Satıcıda var mı?
        float hasQty = EconomyUtils::getItemAmount(sellerInventory, offer.id);
        
        if (hasQty >= offer.quantity) {
            double unitPrice = getPrice(offer.id);
            double totalEarnings = unitPrice * offer.quantity;

            // 1. Satıcıdan düş
            EconomyUtils::removeFromInventory(sellerInventory, offer.id, offer.quantity);
            
            // 2. Markete ekle
            EconomyUtils::addToInventory(storage, offer.id, offer.quantity);
            
            // 3. Para transferi
            sellerBudget += totalEarnings;
            
            // 4. Baskı (Supply) oluştur
            sellPressure[offer.id] += offer.quantity;
            
            // Fiyat listesinde yoksa ekle
            if (prices.find(offer.id) == prices.end()) {
                prices[offer.id] = unitPrice;
            }
        }
    }
    return true;
}

// --- Utilities ---
void Market::addStock(const std::string& itemId, float amount) {
    EconomyUtils::addToInventory(storage, itemId, amount);
}

void Market::createArtificialDemand(const std::string& itemId, float amount) {
    buyPressure[itemId] += amount;
}

void to_json(nlohmann::json& j, const Market& m) {
    j = nlohmann::json{
        {"id", uuids::to_string(m.id)},
        {"name", m.name},
        {"prices", m.prices},
        {"storage", m.storage},
        {"buyPressure", m.buyPressure},
        {"sellPressure", m.sellPressure}
    };
    
    // Registered Nodes
    std::vector<std::string> nodes;
    for(const auto& uid : m.registeredNodes) nodes.push_back(uuids::to_string(uid));
    j["nodes"] = nodes;
}