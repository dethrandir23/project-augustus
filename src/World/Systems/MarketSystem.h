// src/Systems/MarketSystem.h
#pragma once
#include "Game/Gamestate.h"
#include "Core/Types.h"
#include "uuid/uuid.h"

class MarketSystem {
public:
    // 1. Emir Verme (Escrow burada yapılır)
    static void placeOrder(Gamestate& gamestate, uuids::uuid marketId, MarketOrder order);

private:
    // 2. Takas İşlemi (OrderBook içinden callback olarak çağrılır)
    static void executeTrade(Gamestate& gamestate, uuids::uuid buyerId, uuids::uuid sellerId, std::string itemId, double price, float qty, double refund);
};

