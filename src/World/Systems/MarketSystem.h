// src/Systems/MarketSystem.h
#pragma once
#include "Game/Gamestate.h"
#include "Core/Types.h"
#include "uuid/uuid.h"

struct BestMarket {
    uuids::uuid marketId;
    double effectivePrice;
    double bestAsk;
};

class MarketSystem {
public:
    static void placeOrder(Gamestate& gamestate, uuids::uuid marketId, MarketOrder order, bool isSystemOrder = false);

    // Cross-market yardımcı: en iyi alış marketini bul (vergi dahil)
    static BestMarket findBestBuyMarket(Gamestate& gamestate, const std::string& itemId, uuids::uuid localMarketId);

    // Tüm marketlerdeki açık BUY emirlerini topla (bir owner için)
    static float getTotalBuyVolume(Gamestate& gamestate, const std::string& itemId);
    static float getBuyOrderForOwner(Gamestate& gamestate, uuids::uuid ownerId, const std::string& itemId);

    // Bir entity'nin tüm marketlerdeki açık emirlerini getir
    static std::vector<MarketOrder> getBuyOrdersForOwner(Gamestate& gamestate, uuids::uuid ownerId);
    static std::vector<MarketOrder> getSellOrdersForOwner(Gamestate& gamestate, uuids::uuid ownerId);

    // Emir iptal: iadeyi otomatik yapar (para → wallet, mal → storage)
    static bool cancelOrder(Gamestate& gamestate, uuids::uuid marketId, uuids::uuid orderId);

private:
    static void executeTrade(Gamestate& gamestate, uuids::uuid buyerId, uuids::uuid sellerId, std::string itemId, double price, float qty, double refund, uuids::uuid marketId);
};

