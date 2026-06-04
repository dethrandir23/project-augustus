#pragma once
#include "Core/Types.h"
#include <algorithm>
#include <functional>
#include <vector>

class OrderBook {
public:
  std::string itemId;

  // Sorted: buyOrders highest price first, then earliest timestamp first
  // sellOrders lowest price first, then earliest timestamp first
  // Sorted order is maintained via binary-search insertion (O(n) per insert)
  std::vector<MarketOrder> buyOrders;
  std::vector<MarketOrder> sellOrders;

  double lastTradedPrice = 0.0;

  using TradeCallback = std::function<void(uuids::uuid, uuids::uuid,
                                           std::string, double, float, double)>;
  TradeCallback onTrade;

  void setTradeCallback(TradeCallback cb) { onTrade = cb; }

  void addOrder(MarketOrder order) {
    if (order.type == OrderType::BUY) {
      auto it = std::lower_bound(buyOrders.begin(), buyOrders.end(), order,
        [](const MarketOrder& a, const MarketOrder& b) {
          if (std::abs(a.price - b.price) > 0.0001)
            return a.price > b.price;
          return a.timestamp < b.timestamp;
        });
      buyOrders.insert(it, order);
    } else {
      auto it = std::lower_bound(sellOrders.begin(), sellOrders.end(), order,
        [](const MarketOrder& a, const MarketOrder& b) {
          if (std::abs(a.price - b.price) > 0.0001)
            return a.price < b.price;
          return a.timestamp < b.timestamp;
        });
      sellOrders.insert(it, order);
    }
    matchOrders();
  }

  void matchOrders() {
    while (!buyOrders.empty() && !sellOrders.empty()) {
      MarketOrder& bestBuy = buyOrders.front();
      MarketOrder& bestSell = sellOrders.front();

      if (bestBuy.ownerId == bestSell.ownerId) {
        buyOrders.erase(buyOrders.begin());
        sellOrders.erase(sellOrders.begin());
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

        bestBuy.filledQuantity += quantity;
        bestSell.filledQuantity += quantity;
        lastTradedPrice = executionPrice;

        if (bestBuy.isComplete())
          buyOrders.erase(buyOrders.begin());
        if (bestSell.isComplete())
          sellOrders.erase(sellOrders.begin());
      } else {
        break;
      }
    }
  }

  double getBestBid() const {
    return buyOrders.empty() ? 0.0 : buyOrders.front().price;
  }

  double getBestAsk() const {
    return sellOrders.empty() ? 0.0 : sellOrders.front().price;
  }

  // Emir iptal: orderId'ye göre bul, vektörden sil, iade edilecek order'ı döndür
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
