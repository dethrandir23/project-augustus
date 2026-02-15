#include "Core/Types.h"
#include <functional>
#include <queue>
#include <vector>

struct BuyOrderComparator {
  bool operator()(const MarketOrder &a, const MarketOrder &b) {
    if (std::abs(a.price - b.price) > 0.0001)
      return a.price < b.price;
    return a.timestamp > b.timestamp; // Fiyat aynıysa eskisi öncelikli
  }
};

// Satıcılar: En DÜŞÜK fiyat isteyen en üstte durur.
struct SellOrderComparator {
  bool operator()(const MarketOrder &a, const MarketOrder &b) {
    if (std::abs(a.price - b.price) > 0.0001)
      return a.price > b.price;
    return a.timestamp > b.timestamp;
  }
};

class OrderBook {
public:
  std::string itemId;

  // Defterler (Priority Queue: Her zaman en iyi teklifi tepede tutar)
  // Not: PriorityQueue içindeki elemanlara erişmek zordur, o yüzden genellikle
  // std::vector + std::push_heap/pop_heap kullanılır ama şimdilik vector ile
  // manuel sort yapalım, daha esnek olur.
  std::vector<MarketOrder> buyOrders;
  std::vector<MarketOrder> sellOrders;

  double lastTradedPrice = 0.0; // Son işlem fiyatı (Borsa fiyatı budur)

  // --- Core Functions ---

  void addOrder(MarketOrder order) {
    if (order.type == OrderType::BUY) {
      buyOrders.push_back(order);
      // Alış listesini sırala (Pahalıdan ucuza)
      std::sort(buyOrders.begin(), buyOrders.end(),
                [](const MarketOrder &a, const MarketOrder &b) {
                  if (std::abs(a.price - b.price) > 0.0001)
                    return a.price > b.price;
                  return a.timestamp < b.timestamp;
                });
    } else {
      sellOrders.push_back(order);
      // Satış listesini sırala (Ucuzdan pahalıya)
      std::sort(sellOrders.begin(), sellOrders.end(),
                [](const MarketOrder &a, const MarketOrder &b) {
                  if (std::abs(a.price - b.price) > 0.0001)
                    return a.price < b.price;
                  return a.timestamp < b.timestamp;
                });
    }

    // Eşleşme var mı bak?
    matchOrders();
  }

  // --- THE MATCHING ENGINE (Sihir Burada) ---
  // Callback: İşlem gerçekleşince parayı ve malı transfer etmek için dışarıya
  // haber verir. Fonksiyon imzası: (BuyerID, SellerID, ItemID, Price, Quantity)
  using TradeCallback = std::function<void(uuids::uuid, uuids::uuid,
                                           std::string, double, float, double)>;
  TradeCallback onTrade;

  void setTradeCallback(TradeCallback cb) { onTrade = cb; }

  void matchOrders() {
    while (!buyOrders.empty() && !sellOrders.empty()) {
      MarketOrder &bestBuy =
          buyOrders.front(); // Burada priority_queue yerine vector kullandığını
                             // varsayıyorum
      MarketOrder &bestSell = sellOrders.front();

      // Fiyatlar Uyuşuyor mu?
      if (bestBuy.price >= bestSell.price) {

        // İşlem Fiyatı: Genelde bekleyen emrin fiyatı (Maker Price)
        double executionPrice = bestBuy.timestamp < bestSell.timestamp
                                    ? bestBuy.price
                                    : bestSell.price;

        float quantity = std::min(bestBuy.remaining(), bestSell.remaining());

        // --- İADE HESABI (Sihirli Kısım) ---
        // Alıcı 'bestBuy.price' kadar ödemişti. Ama işlem 'executionPrice'tan
        // oldu.
        double refundPerUnit = bestBuy.price - executionPrice;
        double totalRefund = refundPerUnit * quantity;

        // Callback ile Market'e Haber Ver (+ Refund bilgisini de gönder)
        if (onTrade) {
          onTrade(bestBuy.ownerId, bestSell.ownerId, itemId, executionPrice,
                  quantity, totalRefund);
        }

        // Defteri Güncelle
        bestBuy.filledQuantity += quantity;
        bestSell.filledQuantity += quantity;
        lastTradedPrice = executionPrice;

        if (bestBuy.isComplete())
          buyOrders.erase(buyOrders.begin());
        if (bestSell.isComplete())
          sellOrders.erase(sellOrders.begin());

      } else {
        // Eşleşme yok, döngüyü kır
        break;
      }
    }
  }

  // --- AI / MLP İçin Veri Çekme Fonksiyonları ---

  // En iyi alış teklifi kaç? (Satmak isteyen buna bakar)
  double getBestBid() const {
    return buyOrders.empty() ? 0.0 : buyOrders.front().price;
  }

  // En iyi satış teklifi kaç? (Almak isteyen buna bakar)
  double getBestAsk() const {
    return sellOrders.empty()
               ? 0.0
               : sellOrders.front().price; // 0.0 ise satıcı yok demek
  }
};