#pragma once
#include "Core/Types.h"
#include "Core/GameConstants.h"
#include <algorithm>
#include <functional>
#include <set>
#include <vector>

struct BuyOrderCompare {
    bool operator()(const MarketOrder& a, const MarketOrder& b) const {
        if (std::abs(a.price - b.price) > 0.0001)
            return a.price > b.price;
        return a.timestamp < b.timestamp;
    }
};

struct SellOrderCompare {
    bool operator()(const MarketOrder& a, const MarketOrder& b) const {
        if (std::abs(a.price - b.price) > 0.0001)
            return a.price < b.price;
        return a.timestamp < b.timestamp;
    }
};

class OrderBook {
public:
  std::string itemId;

  std::multiset<MarketOrder, BuyOrderCompare> buyOrders;
  std::multiset<MarketOrder, SellOrderCompare> sellOrders;

  double lastTradedPrice = 0.0;

  using TradeCallback = std::function<void(uuids::uuid, uuids::uuid,
                                           std::string, double, float, double)>;
  TradeCallback onTrade;

  void setTradeCallback(TradeCallback cb) { onTrade = cb; }

  void addOrder(MarketOrder order) {
    if (order.type == OrderType::BUY) {
      auto it = std::find_if(buyOrders.begin(), buyOrders.end(),
        [&](const MarketOrder& existing) {
          return existing.ownerId == order.ownerId
              && existing.itemId == order.itemId
              && std::abs(existing.price - order.price) < 0.0001;
        });
      if (it != buyOrders.end()) {
        MarketOrder merged = *it;
        merged.quantity += order.quantity;
        buyOrders.erase(it);
        buyOrders.insert(std::move(merged));
      } else {
        buyOrders.insert(std::move(order));
      }
    } else {
      auto it = std::find_if(sellOrders.begin(), sellOrders.end(),
        [&](const MarketOrder& existing) {
          return existing.ownerId == order.ownerId
              && existing.itemId == order.itemId
              && std::abs(existing.price - order.price) < 0.0001;
        });
      if (it != sellOrders.end()) {
        MarketOrder merged = *it;
        merged.quantity += order.quantity;
        sellOrders.erase(it);
        sellOrders.insert(std::move(merged));
      } else {
        sellOrders.insert(std::move(order));
      }
    }
    matchOrders();
  }

  void matchOrders() {
    while (!buyOrders.empty() && !sellOrders.empty()) {
      auto bestBuyIt = buyOrders.begin();
      auto bestSellIt = sellOrders.begin();

      if (bestBuyIt->ownerId == bestSellIt->ownerId) {
        ++bestSellIt;
        if (bestSellIt == sellOrders.end()) break;
        if (bestBuyIt->ownerId == bestSellIt->ownerId) break;
      }

      const MarketOrder& bestBuy = *bestBuyIt;
      const MarketOrder& bestSell = *bestSellIt;

      if (bestBuy.price >= bestSell.price) {
        double executionPrice = bestBuy.timestamp < bestSell.timestamp
                                    ? bestBuy.price
                                    : bestSell.price;

        float quantity = std::min(bestBuy.remaining(), bestSell.remaining());

        double refundPerUnit = bestBuy.price - executionPrice;
        double totalRefund = refundPerUnit * quantity;

        if (onTrade) {
          onTrade(bestBuy.ownerId, bestSell.ownerId, itemId, executionPrice,
                  quantity, totalRefund);
        }

        MarketOrder buyCopy = bestBuy;
        MarketOrder sellCopy = bestSell;
        buyOrders.erase(bestBuyIt);
        sellOrders.erase(bestSellIt);

        buyCopy.filledQuantity += quantity;
        sellCopy.filledQuantity += quantity;

        lastTradedPrice = executionPrice;

        if (!buyCopy.isComplete())
          buyOrders.insert(std::move(buyCopy));
        if (!sellCopy.isComplete())
          sellOrders.insert(std::move(sellCopy));
      } else {
        break;
      }
    }
  }

  double getBestBid() const {
    return buyOrders.empty() ? 0.0 : buyOrders.begin()->price;
  }

  double getBestAsk() const {
    return sellOrders.empty() ? 0.0 : sellOrders.begin()->price;
  }

  bool cancelOrder(const uuids::uuid& orderId, MarketOrder& outRemoved) {
    for (auto it = buyOrders.begin(); it != buyOrders.end(); ++it) {
      if (it->id == orderId) {
        outRemoved = *it;
        buyOrders.erase(it);
        return true;
      }
    }
    for (auto it = sellOrders.begin(); it != sellOrders.end(); ++it) {
      if (it->id == orderId) {
        outRemoved = *it;
        sellOrders.erase(it);
        return true;
      }
    }
    return false;
  }

  struct ExpiredOrders {
    std::vector<MarketOrder> buyOrders;
    std::vector<MarketOrder> sellOrders;
  };

  ExpiredOrders ageAndExpire() {
    ExpiredOrders expired;
    std::vector<MarketOrder> survivingBuys;
    std::vector<MarketOrder> survivingSells;

    for (auto it = buyOrders.begin(); it != buyOrders.end(); ) {
      MarketOrder order = *it;
      order.age++;
      it = buyOrders.erase(it);
      if (order.isExpired()) {
        expired.buyOrders.push_back(order);
      } else {
        survivingBuys.push_back(std::move(order));
      }
    }
    for (auto it = sellOrders.begin(); it != sellOrders.end(); ) {
      MarketOrder order = *it;
      order.age++;
      it = sellOrders.erase(it);
      if (order.isExpired()) {
        expired.sellOrders.push_back(order);
      } else {
        survivingSells.push_back(std::move(order));
      }
    }

    for (auto& o : survivingBuys) buyOrders.insert(std::move(o));
    for (auto& o : survivingSells) sellOrders.insert(std::move(o));

    return expired;
  }
};
