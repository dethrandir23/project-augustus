// src/Systems/MarketSystem.cpp
#include "MarketSystem.h"
#include "Core/Components/InventoryComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "World/Components/MarketComponent.h"
#include <iostream>

struct TraderAccess {
    InventoryComponent* invComp = nullptr;
    WalletComponent* walletComp = nullptr;
};

// BAK KANKA NE KADAR KISA! Şirketmiş, Şehirmiş umurumuzda değil.
// "Cüzdanı ve Deposu var mı?" diye soruyoruz sadece.
TraderAccess getTrader(Gamestate& gamestate, const uuids::uuid& traderId) {
    Entity* entity = gamestate.getEntity(traderId);
    if (!entity) return {nullptr, nullptr};

    // İstersen "MainStorage", istersen "Storage" kullanmış olabilirler.
    // Şirket ve Şehirlerdeki depo ismine göre arıyoruz.
    auto* inv = entity->GetComponent<InventoryComponent>("Storage");
    if (!inv) inv = entity->GetComponent<InventoryComponent>("MainStorage");

    auto* wallet = entity->GetComponent<WalletComponent>("WalletComponent");

    return {inv, wallet};
}

void MarketSystem::placeOrder(Gamestate& gamestate, uuids::uuid marketId, MarketOrder order) {
    Entity* marketEntity = gamestate.getEntity(marketId);
    if (!marketEntity) return;

    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    if (!marketComp) return;

    auto trader = getTrader(gamestate, order.ownerId);
    if (!trader.invComp || !trader.walletComp) {
        std::cout << "HATA: Trader bilesenleri eksik! ID: " << uuids::to_string(order.ownerId) << std::endl;
        return; 
    }

    // --- A) ESCROW (Bloke) İŞLEMİ ---
    if (order.type == OrderType::BUY) {
        double totalCost = order.price * order.quantity;
        if (trader.walletComp->balance >= totalCost) {
            trader.walletComp->balance -= totalCost; 
        } else {
            // std::cout << "Yetersiz Bakiye!" << std::endl;
            return; 
        }
    } 
    else { // SELL
        if (trader.invComp->Has(order.itemId, order.quantity)) {
            trader.invComp->Remove(order.itemId, order.quantity);
        } else {
            // std::cout << "Yetersiz Stok!" << std::endl;
            return;
        }
    }

    // --- B) DEFTER HAZIRLIĞI VE CALLBACK ---
    OrderBook* book = marketComp->getBook(order.itemId);
    
    // Callback'i bağlıyoruz. Defterde eşleşme olursa bu fonksiyon tetiklenecek.
    book->setTradeCallback(
        [&gamestate](uuids::uuid b, uuids::uuid s, std::string i, double p, float q, double refund) {
            MarketSystem::executeTrade(gamestate, b, s, i, p, q, refund);
        }
    );
    
    // --- C) EMRİ EKLE (Bu işlem içeride matchOrders'ı tetikler) ---
    book->addOrder(order);
}

void MarketSystem::executeTrade(Gamestate& gamestate, uuids::uuid buyerId, uuids::uuid sellerId, std::string itemId, double price, float qty, double refund) {
    auto buyer = getTrader(gamestate, buyerId);
    auto seller = getTrader(gamestate, sellerId);

    if (!buyer.invComp || !seller.invComp) return; // Güvenlik kontrolü

    double totalEarnings = price * qty;

    // 1. SATICIYA PARASINI VER (Alıcı escrowda ödemişti)
    if (seller.walletComp) {
        seller.walletComp->addMoney(totalEarnings);
    }

    // 2. ALICIYA MALINI VER (Satıcı escrowda vermişti)
    buyer.invComp->Add(itemId, qty);

    // 3. İADE (REFUND) YAP (Alıcı fazla teklif verdiyse)
    if (refund > 0.0001 && buyer.walletComp) {
        buyer.walletComp->addMoney(refund);
    }

    // Loglama
    // std::cout << "[MARKET] Islem: " << qty << "x " << itemId << " @ " << price << "$ (Refund: " << refund << ")" << std::endl;
}