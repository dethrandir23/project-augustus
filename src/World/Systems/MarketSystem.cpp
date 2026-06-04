// src/Systems/MarketSystem.cpp
#include "MarketSystem.h"
#include "Core/Components/InventoryComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Orderbook.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/MarketMemberComponent.h"
#include <iostream>

struct TraderAccess {
  InventoryComponent *invComp = nullptr;
  WalletComponent *walletComp = nullptr;
};

TraderAccess getTrader(Gamestate &gamestate, const uuids::uuid &traderId) {
  Entity *entity = gamestate.getEntity(traderId);
  if (!entity)
    return {nullptr, nullptr};

  auto *inv = entity->GetComponent<InventoryComponent>("Storage");
  if (!inv)
    inv = entity->GetComponent<InventoryComponent>("MainStorage");

  auto *wallet = entity->GetComponent<WalletComponent>("WalletComponent");

  return {inv, wallet};
}

BestMarket MarketSystem::findBestBuyMarket(Gamestate& gamestate, const std::string& itemId, uuids::uuid localMarketId) {
    BestMarket result{uuids::uuid{}, 0.0, 0.0};

    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* marketComp = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!marketComp) continue;

        OrderBook* book = marketComp->getBook(itemId);
        double bestAsk = book->getBestAsk();
        if (bestAsk <= 0.0) continue;

        uuids::uuid mId = entity->GetId();
        double tariff = (mId == localMarketId) ? 0.0 : static_cast<double>(marketComp->tariffRate);
        double effectivePrice = bestAsk * (1.0 + tariff);

        if (result.marketId.is_nil() || effectivePrice < result.effectivePrice) {
            result = {mId, effectivePrice, bestAsk};
        }
    }

    return result;
}

float MarketSystem::getTotalBuyVolume(Gamestate& gamestate, const std::string& itemId) {
    float total = 0.0f;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* marketComp = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!marketComp) continue;
        OrderBook* book = marketComp->getBook(itemId);
        for (const auto& order : book->buyOrders) {
            total += order.remaining();
        }
    }
    return total;
}

std::vector<MarketOrder> MarketSystem::getBuyOrdersForOwner(Gamestate& gamestate, uuids::uuid ownerId) {
    std::vector<MarketOrder> result;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* marketComp = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!marketComp) continue;
        for (auto& [itemId, book] : marketComp->books) {
            (void)itemId;
            for (const auto& order : book.buyOrders) {
                if (order.ownerId == ownerId) result.push_back(order);
            }
        }
    }
    return result;
}

std::vector<MarketOrder> MarketSystem::getSellOrdersForOwner(Gamestate& gamestate, uuids::uuid ownerId) {
    std::vector<MarketOrder> result;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* marketComp = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!marketComp) continue;
        for (auto& [itemId, book] : marketComp->books) {
            (void)itemId;
            for (const auto& order : book.sellOrders) {
                if (order.ownerId == ownerId) result.push_back(order);
            }
        }
    }
    return result;
}

float MarketSystem::getBuyOrderForOwner(Gamestate& gamestate, uuids::uuid ownerId, const std::string& itemId) {
    float total = 0.0f;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* marketComp = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!marketComp) continue;
        OrderBook* book = marketComp->getBook(itemId);
        for (const auto& order : book->buyOrders) {
            if (order.ownerId == ownerId) {
                total += order.remaining();
            }
        }
    }
    return total;
}

bool MarketSystem::cancelOrder(Gamestate& gamestate, uuids::uuid marketId, uuids::uuid orderId) {
    Entity* marketEntity = gamestate.getEntity(marketId);
    if (!marketEntity) return false;

    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    if (!marketComp) return false;

    MarketOrder cancelled;
    bool found = false;
    for (auto& [itemId, book] : marketComp->books) {
        if (book.cancelOrder(orderId, cancelled)) {
            found = true;
            break;
        }
    }
    if (!found) return false;

    // İade: alıcıya parasını, satıcıya malını geri ver
    auto trader = getTrader(gamestate, cancelled.ownerId);
    if (!trader.invComp || !trader.walletComp) return false;

    if (cancelled.type == OrderType::BUY) {
        double refund = cancelled.price * cancelled.remaining();
        trader.walletComp->addMoney(refund);
    } else {
        trader.invComp->Add(cancelled.itemId, cancelled.remaining());
    }

    return true;
}

void MarketSystem::placeOrder(Gamestate &gamestate, uuids::uuid marketId,
                              MarketOrder order, bool isSystemOrder) {
  Entity *marketEntity = gamestate.getEntity(marketId);
  if (!marketEntity)
    return;

  auto *marketComp =
      marketEntity->GetComponent<MarketComponent>("MarketComponent");
  if (!marketComp)
    return;

  OrderBook *book = marketComp->getBook(order.itemId);

  if (!isSystemOrder) {

    auto trader = getTrader(gamestate, order.ownerId);
    if (!trader.invComp || !trader.walletComp) {
      std::cout << "HATA: Trader bilesenleri eksik! ID: "
                << uuids::to_string(order.ownerId) << std::endl;
      return;
    }

    // --- A) ESCROW (Bloke) İŞLEMİ ---
    if (order.type == OrderType::BUY) {
      double tariffMultiplier = 1.0;
      Entity* buyerEntity = gamestate.getEntity(order.ownerId);
      if (buyerEntity) {
        auto* marketMember = buyerEntity->GetComponent<MarketMemberComponent>("MarketMemberComponent");
        if (marketMember && marketMember->marketId != marketId) {
          tariffMultiplier = 1.0 + static_cast<double>(marketComp->tariffRate);
        }
      }
      double totalCost = order.price * order.quantity * tariffMultiplier;
      if (trader.walletComp->balance >= totalCost) {
        trader.walletComp->balance -= totalCost;
      } else {
        return;
      }
    } else { // SELL
      if (trader.invComp->Has(order.itemId, order.quantity)) {
        trader.invComp->Remove(order.itemId, order.quantity);
      } else {
        return;
      }
    }

    // Callback: trade gerçekleşince çağrılır
    book->setTradeCallback([&gamestate, marketId](uuids::uuid b, uuids::uuid s,
                                        std::string i, double p, float q,
                                        double refund) {
      MarketSystem::executeTrade(gamestate, b, s, i, p, q, refund, marketId);
    });
  }

  book->addOrder(order);
}

void MarketSystem::executeTrade(Gamestate &gamestate, uuids::uuid buyerId,
                                uuids::uuid sellerId, std::string itemId,
                                double price, float qty, double refund,
                                uuids::uuid marketId) {
  auto buyer = getTrader(gamestate, buyerId);
  auto seller = getTrader(gamestate, sellerId);

  if (!buyer.invComp || !seller.invComp)
    return;

  double totalEarnings = price * qty;

  // 1. SATICIYA PARASINI VER
  if (seller.walletComp) {
    seller.walletComp->addMoney(totalEarnings);
  }

  // 2. ALICIYA MALINI VER
  buyer.invComp->Add(itemId, qty);

  // 3. İADE (REFUND)
  if (refund > 0.0001 && buyer.walletComp) {
    buyer.walletComp->addMoney(refund);
  }

  // 4. GÜMRÜK VERGİSİ (cross-market)
  Entity* marketEntity = gamestate.getEntity(marketId);
  Entity* buyerEntity = gamestate.getEntity(buyerId);
  if (marketEntity && buyerEntity) {
    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    auto* buyerMarketMember = buyerEntity->GetComponent<MarketMemberComponent>("MarketMemberComponent");
    if (marketComp && buyerMarketMember && buyerMarketMember->marketId != marketId) {
      double tariff = price * qty * static_cast<double>(marketComp->tariffRate);
      if (buyer.walletComp && buyer.walletComp->balance >= tariff) {
        buyer.walletComp->balance -= tariff;
        auto* marketWallet = marketEntity->GetComponent<WalletComponent>("WalletComponent");
        if (marketWallet) marketWallet->addMoney(tariff);
      }
    }
  }
}
