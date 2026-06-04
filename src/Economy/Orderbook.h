#pragma once
#include "Core/Types.h"
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
      buyOrders.insert(std::move(order));
    } else {
      sellOrders.insert(std::move(order));
    }
    matchOrders();
  }

  void matchOrders() {
    while (!buyOrders.empty() && !sellOrders.empty()) {
      auto bestBuyIt = buyOrders.begin();
      auto bestSellIt = sellOrders.begin();
      const MarketOrder& bestBuy = *bestBuyIt;
      const MarketOrder& bestSell = *bestSellIt;

      if (bestBuy.ownerId == bestSell.ownerId) {
        buyOrders.erase(bestBuyIt);
        sellOrders.erase(bestSellIt);
        continue;
      }

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
};
