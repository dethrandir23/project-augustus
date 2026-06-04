#pragma once
#include "Core/Types.h"
#include "Game/Gamestate.h"

namespace MarketDebug {

struct OrderStats {
    int buyCount = 0;
    int sellCount = 0;
    int totalCount = 0;
    float buyVolume = 0.0f;
    float sellVolume = 0.0f;

    // Cumulative (tüm zamanlar)
    int totalBuyOrdersPlaced = 0;
    int totalSellOrdersPlaced = 0;
    int totalTradesExecuted = 0;
    float totalTradeVolume = 0.0f;
};

// Tüm marketlerin toplamı
OrderStats getGlobalStats(Gamestate& gamestate);

// Belirli bir market
OrderStats getMarketStats(Gamestate& gamestate, const uuids::uuid& marketId);

// Belirli bir entity (tüm marketlerde)
OrderStats getEntityStats(Gamestate& gamestate, const uuids::uuid& entityId);

// Belirli bir entity + belirli market
OrderStats getEntityStatsInMarket(Gamestate& gamestate, const uuids::uuid& entityId, const uuids::uuid& marketId);

// JSON dökümü: {"global": ..., "markets": {...}, "entities": {...}}
nlohmann::json getAllStatsAsJson(Gamestate& gamestate);

// Tek bir OrderStats'ı JSON'a çevir (Godot binding için public)
nlohmann::json statsToJson(const OrderStats& s);

}
