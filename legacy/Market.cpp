#include "Market.h"
#include "../Game/IdUtils.h"
#include "DevTools/Console.h"
#include "Economy/TradeUtils.h"
#include "Game/Gamestate.h"
#include <iostream>

struct TraderAccess {
    Inventory* inv = nullptr;
    double* wallet = nullptr;
};

TraderAccess getTrader(Gamestate &gamestate, uuids::uuid id) {
    if (auto* c = gamestate.getCompany(id)) { 
        // Not: Company.h'a 'double& getCapitalRef()' eklediğini varsayıyorum!
        return { &(c->getStorage()), &(c->getCapitalRef()) }; 
    }
    if (auto* n = gamestate.getTradeNode(id)) {
        // Not: TradeNode.h'a 'double& getCapitalRef()' eklediğini varsayıyorum!
        return { &(n->getStorage()), &(n->getCapitalRef()) };
    }
    return {nullptr, nullptr};
}

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
    // 1. Önce OrderBook'a bak (Gerçek Piyasa Fiyatı)
    // Eğer işlem olmuşsa 'lastTradedPrice' en doğru veridir.
    if (orderBooks.count(itemId) && orderBooks.at(itemId).lastTradedPrice > 0.0001) {
         return orderBooks.at(itemId).lastTradedPrice;
    }

    // 2. İşlem yoksa eski 'prices' mapine bak
    if (prices.count(itemId)) {
        return prices.at(itemId);
    }

    // 3. O da yoksa Taban Fiyat
    if (ItemManager::items.count(itemId)) {
        return static_cast<double>(ItemManager::items.at(itemId).base_price);
    }
    return 1.0; // Fallback
}

void Market::calculateMarketPrices() {
    // Inventory class'ından tüm itemları çekiyoruz
    std::vector<ItemStack> allItems = storage.getAllItems();
    
    // Fiyat listesindekileri de ekle
    for(const auto& [id, _] : prices) allItems.push_back({id, 0});
    
    // Unique ID mantığı (Basitleştirilmiş loop)
    std::vector<std::string> uniqueIds;
    for(const auto& item : allItems) uniqueIds.push_back(item.id);
    std::sort(uniqueIds.begin(), uniqueIds.end());
    uniqueIds.erase(std::unique(uniqueIds.begin(), uniqueIds.end()), uniqueIds.end());

    for (const auto& itemId : uniqueIds) {
        double currentPrice = getPrice(itemId);

        // --- DÜZELTME: storage.getAmount() ---
        float totalSupply = sellPressure[itemId] + storage.getAmount(itemId);
        float totalDemand = buyPressure[itemId];

        float ratio = (totalSupply > 0.001f) ? (totalDemand / totalSupply) : totalDemand;
        ratio = std::log1p(ratio);

        float k = 1.8f;
        float equilibrium = 1.0f;
        double minP = static_cast<double>(EconomyManager::data.price_min);
        double maxP = static_cast<double>(EconomyManager::data.price_max);

        double target = minP + (maxP - minP) / (1.0 + std::exp(-static_cast<double>(k * (ratio - equilibrium))));
        float smoothing = getDynamicSmoothing(ratio);
        double newPrice = currentPrice + smoothing * (target - currentPrice);

        prices[itemId] = std::max(newPrice, minP);
    }
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


bool Market::buyItems(const std::vector<ItemStack>& itemsToBuy, Inventory& buyerInventory, double& buyerBudget) {
    Console::log("Do not use buyItems function anymore, use TradeUtils::performTrade instead.", LogType::INFO);
    // Market burada "Satıcı" (Source), Oyuncu/Şirket "Alıcı" (Target)
    
    // Fiyatı belirleyen lambda fonksiyonu:
    auto pricer = [this](const std::string& id) { 
        return this->getPrice(id); 
    };

    // Market'in kendi cüzdanı yok varsayıyoruz (sonsuz kaynak veya simüle edilmeyen para), 
    // o yüzden dummy (geçici) bir double veriyoruz.
    double marketDummyWallet = 0.0; 

    bool success = TradeUtils::performTrade(
        this->storage,      // Satıcı: Market Deposu
        marketDummyWallet,  // Satıcı Cüzdanı (Markete para girer ama önemsemiyorsak dummy)
        buyerInventory,     // Alıcı: Oyuncu Deposu
        buyerBudget,        // Alıcı Cüzdanı
        itemsToBuy,         // İstenenler
        pricer              // Fiyatlandırma mantığı
    );

    if (success) {
        // Marketin ekstra mantığı (Talep oluşturma)
        for(const auto& item : itemsToBuy) {
            this->buyPressure[item.id] += item.quantity;
        }
    }

    return success;
}

bool Market::sellItems(const std::vector<ItemStack>& itemsToSell, Inventory& sellerInventory, double& sellerBudget) {
    Console::log("Do not use sellItems function anymore, use TradeUtils::performTrade instead.", LogType::INFO);
    for (const auto& offer : itemsToSell) {
        // --- DÜZELTME: sellerInventory.getAmount() ---
        float hasQty = sellerInventory.getAmount(offer.id);
        
        if (hasQty >= offer.quantity) {
            double unitPrice = getPrice(offer.id);
            double totalEarnings = unitPrice * offer.quantity;

            // 1. Satıcıdan düş (sellerInventory.remove)
            sellerInventory.remove(offer.id, offer.quantity);
            
            // 2. Markete ekle (storage.add)
            storage.add(offer.id, offer.quantity);
            
            // 3. Para ve Baskı
            sellerBudget += totalEarnings;
            sellPressure[offer.id] += offer.quantity;
            
            if (prices.find(offer.id) == prices.end()) {
                prices[offer.id] = unitPrice;
            }
        }
    }
    return true;
}

// --- Utilities ---
void Market::addStock(const std::string& itemId, float amount) {
    storage.add(itemId, amount);
}

void Market::createArtificialDemand(const std::string& itemId, float amount) {
    buyPressure[itemId] += amount;
}

// --- 1. EMİR VERME (ESCROW BURADA YAPILIR) ---
void Market::placeOrder(Gamestate &gamestate, MarketOrder order) {
    auto trader = getTrader(gamestate, order.ownerId);
    
    // Hata kontrolü
    if (!trader.inv || !trader.wallet) {
        std::cout << "HATA: Trader bulunamadi ID: " << uuids::to_string(order.ownerId) << std::endl;
        return; 
    }

    // --- A) ESCROW (Bloke) İŞLEMİ ---
    // Emri deftere yazmadan önce parayı/malı peşin alıyoruz.
    if (order.type == OrderType::BUY) {
        // ALIŞ: Para bloke et
        double totalCost = order.price * order.quantity;
        if (*trader.wallet >= totalCost) {
            *trader.wallet -= totalCost; 
        } else {
            std::cout << "Yetersiz Bakiye!" << std::endl;
            return; 
        }
    } 
    else {
        // SATIŞ: Mal bloke et
        if (trader.inv->has(order.itemId, order.quantity)) {
            trader.inv->remove(order.itemId, order.quantity);
        } else {
            std::cout << "Yetersiz Stok!" << std::endl;
            return;
        }
    }

    // --- B) DEFTER HAZIRLIĞI ---
    if (orderBooks.find(order.itemId) == orderBooks.end()) {
        orderBooks[order.itemId].itemId = order.itemId;
    }
    
    // Callback'i her seferinde tazeliyoruz ki 'gamestate' referansı güncel kalsın.
    // Lambda içinde 'this' ve '&gamestate' yakalıyoruz.
    orderBooks[order.itemId].setTradeCallback(
        [&gamestate, this](uuids::uuid b, uuids::uuid s, std::string i, double p, float q, double refund) {
            this->executeTrade(gamestate, b, s, i, p, q, refund);
        }
    );
    
    // --- C) EMRİ EKLE ---
    orderBooks[order.itemId].addOrder(order);
}
// --- 2. TAKAS (EXECUTE TRADE) ---
void Market::executeTrade(Gamestate& gamestate, uuids::uuid buyerId, uuids::uuid sellerId, std::string itemId, double price, float qty, double refund) {
    auto buyer = getTrader(gamestate, buyerId);
    auto seller = getTrader(gamestate, sellerId);

    if (!buyer.inv || !seller.inv) return; // Güvenlik kontrolü

    double totalEarnings = price * qty;

    // 1. SATICIYA PARASINI VER
    // (Alıcı parayı placeOrder'da ödemişti)
    *seller.wallet += totalEarnings;

    // 2. ALICIYA MALINI VER
    // (Satıcı malı placeOrder'da vermişti)
    buyer.inv->add(itemId, qty);

    // 3. İADE (REFUND) VARSA YAP
    // (Alıcı yüksek teklif verdiyse artan parayı iade et)
    if (refund > 0.0001) {
        *buyer.wallet += refund;
    }

    std::cout << "[MARKET] Islem: " << qty << "x " << itemId << " @ " << price << "$ (Refund: " << refund << ")" << std::endl;
    
    // Talep/Arz basıncını güncelle (Eski sistemle uyumluluk için)
    buyPressure[itemId] += qty;
    sellPressure[itemId] += qty;
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