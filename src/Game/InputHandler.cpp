#include "InputHandler.h"
#include "Core/Components/InventoryComponent.h"
#include "DevTools/Console.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/ManpowerPoolComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "Entity/EntityFactory.h" // createFactory için
#include "GameManager.h"
#include "World/Components/MarketComponent.h"
#include "World/Systems/MarketSystem.h" // placeOrder için
#include <iostream>

// Statik değişkeni tanımlama
std::unordered_map<std::string, InputHandler::ActionCallback>
    InputHandler::actionRegistry;

void InputHandler::registerAction(const std::string &actionName,
                                  ActionCallback callback) {
  actionRegistry[actionName] = callback;
}

bool InputHandler::handleInput(Gamestate &gamestate,
                               const nlohmann::json &input) {
  try {
    if (!input.contains("type") || !input.contains("payload"))
      return false;

    std::string type = input.at("type").get<std::string>();
    const auto &payload = input.at("payload");

    // Komut sözlükte var mı bak
    auto it = actionRegistry.find(type);
    if (it != actionRegistry.end()) {
      // Varsa fonksiyonu çalıştır
      return it->second(gamestate, payload);
    } else {
      std::cout << "Bilinmeyen Komut: " << type << std::endl;
      return false;
    }
  } catch (...) {
    return false;
  }
}

// --- YARDIMCI FONKSİYONLAR ---
uuids::uuid getTargetCompanyId(Gamestate &gamestate,
                               const nlohmann::json &payload) {
  if (payload.contains("companyId")) {
    return uuids::uuid::from_string(payload.at("companyId").get<std::string>())
        .value();
  }
  return gamestate.getPlayerCompanyId();
}

// --- KOMUTLARIN KAYDEDİLMESİ (INIT) ---
// Motor başlarken bu fonksiyon 1 kere çağrılacak.
void InputHandler::init() {

 // 0. BATCH_EXECUTE
  registerAction("BATCH_EXECUTE", [](Gamestate &gs, const nlohmann::json &payload) {
    if (!payload.is_array()) return false;
    for (const auto &subInput : payload) {
      if (!gs.handleInput(subInput)) return false;
    }
    return true;
  });

  // 1. STEP_GAME
  registerAction("STEP_GAME",
                 [](Gamestate &gamestate, const nlohmann::json &payload) {
                   size_t times = payload.value("times", 1);
                   for (size_t i = 0; i < times; ++i) {
                     GameManager::tick(gamestate);
                   }
                   return true;
                 });

  // 2. SET_PROPERTY (Generic Entity Property Setter)
  registerAction("SET_PROPERTY", [](Gamestate &gs, const nlohmann::json &payload) {
    uuids::uuid targetId = getTargetCompanyId(gs, payload);
    Entity *entity = gs.getEntity(targetId);
    if (!entity) return false;

    std::string componentName = payload.at("component").get<std::string>();
    std::string propertyName = payload.at("property").get<std::string>();
    auto newValue = payload.at("value");

    if (componentName == "WalletComponent") {
      auto *w = entity->GetComponent<WalletComponent>("WalletComponent");
      if (w) {
        if (propertyName == "balance") w->balance = newValue.get<double>();
        else if (propertyName == "debt") w->debt = newValue.get<double>();
        else return false;
        return true;
      }
    } else if (componentName == "InventoryComponent") {
       auto *inv = entity->GetComponent<InventoryComponent>("MainStorage");
       if (inv) {
         if (propertyName == "maxWeight") inv->setMaxWeight(newValue.get<double>());
         else return false;
         return true;
       }
    }
    return false;
  });

  // 3. SET_TICK_RATE
  registerAction("SET_TICK_RATE", [](Gamestate &gs, const nlohmann::json &payload) {
    // Placeholder for rate adjustment logic
    return true;
  });

  // 4. SCHEDULE_EVENT
  registerAction("SCHEDULE_EVENT", [](Gamestate &gs, const nlohmann::json &payload) {
    // Placeholder for event scheduling
    return true;
  });

  // 5. ADD_MONEY (ECS Mantığıyla)
  registerAction("ADD_MONEY", [](Gamestate &gamestate,
                                 const nlohmann::json &payload) {
    uuids::uuid targetId = getTargetCompanyId(gamestate, payload);
    Entity *company = gamestate.getEntity(targetId);
    if (!company)
      return false;

    auto *wallet = company->GetComponent<WalletComponent>("WalletComponent");
    if (wallet) {
      wallet->addMoney(payload.at("amount").get<double>());
      return true;
    }
    return false;
  });

  // 3. ADD_ITEM (ECS Mantığıyla)
  registerAction("ADD_ITEM", [](Gamestate &gamestate,
                                const nlohmann::json &payload) {
    uuids::uuid targetId = getTargetCompanyId(gamestate, payload);
    Entity *company = gamestate.getEntity(targetId);
    if (!company)
      return false;

    auto *storage = company->GetComponent<InventoryComponent>("MainStorage");
    if (storage) {
      storage->Add(payload.at("itemId").get<std::string>(),
                   payload.at("amount").get<float>());
      return true;
    }
    return false;
  });

  // 4. BUILD_FACTORY (ECS Mantığıyla)
  registerAction("BUILD_FACTORY", [](Gamestate &gamestate,
                                     const nlohmann::json &payload) {
    std::string templateId = payload.at("templateId").get<std::string>();
    if (!FactoryManager::factories.count(templateId))
      return false;
    const auto &fData = FactoryManager::factories.at(templateId);

    uuids::uuid ownerId = getTargetCompanyId(gamestate, payload);
    Entity *ownerCompany = gamestate.getEntity(ownerId);
    if (!ownerCompany)
      return false;

    auto *wallet =
        ownerCompany->GetComponent<WalletComponent>("WalletComponent");
    auto *assets =
        ownerCompany->GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
    if (!wallet || !assets)
      return false;

    // --- MALİYET KONTROLÜ VE ÖDEME ---
    double cost = fData.buildCost; // Struct'a eklediğin özellik!

    // Enflasyon veya perk/teknoloji modifiyesi de ekleyebilirsin:
    // cost = EconomyUtils::calculateEntityModifier(*ownerCompany,
    // "factory_cost_multiplier", cost);

    if (wallet->balance < cost) {
      // Parası yetmiyor!
      return false;
    }

    wallet->balance -= cost; // Parayı tahsil et!
    // ---------------------------------

    std::string customName = payload.value("customName", "");
    Entity *newFactory =
        EntityBuilder::createFactory(fData, ownerId, customName);

    gamestate.addEntity(newFactory);
    assets->addAsset(newFactory->GetId());

    return true;
  });

  // 5. SCRAP_FACTORY (ECS Mantığıyla)
  registerAction("SCRAP_FACTORY", [](Gamestate &gamestate,
                                     const nlohmann::json &payload) {
    uuids::uuid fId =
        uuids::uuid::from_string(payload.at("factoryId").get<std::string>())
            .value();
    Entity *factory = gamestate.getEntity(fId);
    if (!factory)
      return false;

    // Sahibini bulup listesinden çıkaralım
    auto *ownerComp = factory->GetComponent<OwnerComponent>("OwnerComponent");
    if (ownerComp) {
      Entity *owner = gamestate.getEntity(ownerComp->getOwnerId());
      if (owner) {
        auto *assets =
            owner->GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
        if (assets)
          assets->removeAsset(fId);
      }
    }

    // Gamestate'ten tamamen sil
    // Not: std::unordered_map'ten silmek için Gamestate'e bir
    // removeEntity(uuid) metodu eklemelisin.
    gamestate.removeEntity(fId);
    return true;
  });

  // 6. MARKET_BUY_ITEM (Yeni MarketSystem ile)
  registerAction("MARKET_BUY_ITEM", [](Gamestate &gamestate,
                                       const nlohmann::json &payload) {
    uuids::uuid ownerId = getTargetCompanyId(gamestate, payload);
    uuids::uuid mId =
        uuids::uuid::from_string(payload.at("marketId").get<std::string>())
            .value();

    MarketOrder order(ownerId, payload.at("itemId").get<std::string>(),
                      OrderType::BUY, payload.at("price").get<double>(),
                      payload.at("amount").get<float>());

    MarketSystem::placeOrder(gamestate, mId, order);
    return true;
  });

  // 7. MARKET_SELL_ITEM (Yeni MarketSystem ile)
  registerAction("MARKET_SELL_ITEM", [](Gamestate &gamestate,
                                        const nlohmann::json &payload) {
    uuids::uuid ownerId = getTargetCompanyId(gamestate, payload);
    uuids::uuid mId =
        uuids::uuid::from_string(payload.at("marketId").get<std::string>())
            .value();

    MarketOrder order(ownerId, payload.at("itemId").get<std::string>(),
                      OrderType::SELL, payload.at("price").get<double>(),
                      payload.at("amount").get<float>());

    MarketSystem::placeOrder(gamestate, mId, order);
    return true;
  });

  // 7b. MARKET_CANCEL_ORDER (Emir iptal)
  registerAction("MARKET_CANCEL_ORDER", [](Gamestate &gamestate,
                                           const nlohmann::json &payload) {
    uuids::uuid mId =
        uuids::uuid::from_string(payload.at("marketId").get<std::string>())
            .value();
    uuids::uuid orderId =
        uuids::uuid::from_string(payload.at("orderId").get<std::string>())
            .value();
    return MarketSystem::cancelOrder(gamestate, mId, orderId);
  });

  // 8. REDUCE_MANPOWER (Event'ten gelir)
  registerAction("REDUCE_MANPOWER", [](Gamestate &gamestate,
                                       const nlohmann::json &payload) {
    uuids::uuid targetId = getTargetCompanyId(gamestate, payload);
    Entity *company = gamestate.getEntity(targetId);
    if (!company)
      return false;

    auto *mp =
        company->GetComponent<ManpowerPoolComponent>("ManpowerPoolComponent");
    if (mp) {
      size_t amount = payload.at("amount").get<size_t>();
      mp->availableWorkers =
          (mp->availableWorkers > amount) ? (mp->availableWorkers - amount) : 0;
      Console::log("[EVENT] " + company->GetName() + " sirketinin isgucu " +
                       std::to_string(amount) + " azaldi!",
                   LogType::WARNING);
      return true;
    }
    return false;
  });

  // 9. REMOVE_ITEMS (Yangın vs. Event'lerinden gelir)
  registerAction("REMOVE_ITEMS", [](Gamestate &gamestate,
                                    const nlohmann::json &payload) {
    uuids::uuid targetId = getTargetCompanyId(gamestate, payload);
    Entity *company = gamestate.getEntity(targetId);
    if (!company)
      return false;

    auto *storage = company->GetComponent<InventoryComponent>("MainStorage");
    if (!storage)
      return false;

    for (const auto &itemData : payload.at("items")) {
      std::string itemId = itemData.at("id").get<std::string>();
      float amount = itemData.at("amount").get<float>();

      if (itemId == "ALL") {
        // "ALL" geldiyse depodaki her şeyden eksilt (Yangın felaketi!)
        // Not: GetAllItems() ile listeyi alıp tek tek Remove yapıyoruz
        auto allItems = storage->GetAllItems();
        for (const auto &invItem : allItems) {
          // Depodakinin %'si kadar da sildirebilirsin ama JSON'da amount: 10
          // yazmışsın
          storage->Remove(invItem.id, amount);
        }
        Console::log("[EVENT] " + company->GetName() +
                         " deposunda yangin! Mallar zarar gordu.",
                     LogType::WARNING);
      } else {
        storage->Remove(itemId, amount);
      }
    }
    return true;
  });

  // 10. ADD_DEMAND (Kıtlık, Hard Times Event'i)
  registerAction("ADD_DEMAND", [](Gamestate &gamestate,
                                  const nlohmann::json &payload) {
    // V1 için: Markete hayali/suni bir Alış Emri (Buy Order) giriyoruz ki
    // fiyatlar fırlasın.
    for (const auto &itemData : payload.at("items")) {
      std::string itemId = itemData.at("id").get<std::string>();
      float amount = itemData.at("amount").get<float>();

      // Sistem adına bir hayali ID üret (veya Gamestate'te bir SystemID tut)
      uuids::uuid systemDummyId = IdUtils::generateUuid();

      // Tüm marketlere etki et ("market": "ALL" kuralı)
      for (auto *entity : gamestate.getEntitiesByType("market")) {
        auto *marketComp =
            entity->GetComponent<MarketComponent>("MarketComponent");
        if (marketComp) {
          double currentPrice = marketComp->getPrice(itemId);
          // Piyasayı yükseltmek için güncel fiyatın 2 katından devasa bir alım
          // emri gir
          MarketOrder fakeDemand(systemDummyId, itemId, OrderType::BUY,
                                 currentPrice * 2.0, amount);
          MarketSystem::placeOrder(gamestate, entity->GetId(), fakeDemand,
                                   true);
        }
      }
    }
    Console::log("[EVENT] Piyasada talep patlamasi yasandi!", LogType::INFO);
    return true;
  });

  registerAction("PAY_DEBT", [](Gamestate &gs, const nlohmann::json &payload) {
    uuids::uuid id = getTargetCompanyId(gs, payload);
    Entity *e = gs.getEntity(id);
    if (!e)
      return false;
    auto *w = e->GetComponent<WalletComponent>("WalletComponent");
    if (!w || w->debt <= 0 || w->balance < w->debt)
      return false;
    w->balance -= w->debt;
    w->debt = 0;
    return true;
  });

  // Escrow'u tersine çevir: BUY iptalinde para iade, SELL iptalinde mal iade
  registerAction("CANCEL_ORDER", [](Gamestate &gs,
                                    const nlohmann::json &payload) {
    uuids::uuid ownerId = getTargetCompanyId(gs, payload);
    uuids::uuid marketId =
        uuids::uuid::from_string(payload.at("marketId").get<std::string>())
            .value();
    std::string itemId = payload.at("itemId").get<std::string>();
    OrderType type = payload.at("orderType").get<std::string>() == "BUY"
                         ? OrderType::BUY
                         : OrderType::SELL;
    double price = payload.at("price").get<double>();
    float amount = payload.at("amount").get<float>();

    Entity *owner = gs.getEntity(ownerId);
    if (!owner)
      return false;

    Entity *market = gs.getEntity(marketId);
    if (!market)
      return false;
    auto *mc = market->GetComponent<MarketComponent>("MarketComponent");
    if (!mc)
      return false;

    // Orderbook'tan emri bul ve sil
    OrderBook *book = mc->getBook(itemId);
    auto &orders =
        (type == OrderType::BUY) ? book->buyOrders : book->sellOrders;
    auto it =
        std::find_if(orders.begin(), orders.end(), [&](const MarketOrder &o) {
          return o.ownerId == ownerId && std::abs(o.price - price) < 0.001;
        });
    if (it == orders.end())
      return false;

    float remaining = it->remaining();

    // Escrow iadesi
    if (type == OrderType::BUY) {
      auto *w = owner->GetComponent<WalletComponent>("WalletComponent");
      if (w)
        w->balance += price * remaining;
    } else {
      auto *inv = owner->GetComponent<InventoryComponent>("MainStorage");
      if (inv)
        inv->Add(itemId, remaining);
    }

    orders.erase(it);
    return true;
  });

  registerAction("HANDLE_EVENT", [](Gamestate& gs, const nlohmann::json& payload) {
    int evId = payload.at("eventId").get<int>();
    gs.getEventHandler().markAsHandled(evId);
    return true;
  });

  // Diğerleri RENAME_FACTORY, CHANGE_WORKFORCE...
  // Onları da benzer mantıkla (Entity üzerinden component çekerek) buraya
  // eklersin.
}