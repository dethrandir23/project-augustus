#include "InputHandler.h"
#include "DevTools/Console.h"
#include "Gamestate.h"
#include "GameManager.h"
#include "../Registry/FactoryManager.h"
#include "../Economy/EconomyUtils.h"
#include "Game/IdUtils.h"
#include "World/Market.h"

InputType InputHandler::stringToInputType(const std::string &str) {
    static const std::unordered_map<std::string, InputType> mapping = {
        {"STEP_GAME", InputType::STEP_GAME},
        {"RESET_GAME", InputType::RESET_GAME},
        {"BUILD_FACTORY", InputType::BUILD_FACTORY},
        {"SCRAP_FACTORY", InputType::SCRAP_FACTORY},
        {"RENAME_FACTORY", InputType::RENAME_FACTORY},
        {"CHANGE_WORKFORCE", InputType::CHANGE_WORKFORCE},
        {"MARKET_BUY_ITEM", InputType::MARKET_BUY_ITEM},
        {"MARKET_SELL_ITEM", InputType::MARKET_SELL_ITEM},
        {"ADD_MONEY", InputType::ADD_MONEY},
        {"ADD_ITEM", InputType::ADD_ITEM}
    };
    auto it = mapping.find(str);
    return (it != mapping.end()) ? it->second : InputType::UNKNOWN;
}

bool InputHandler::handleInput(Gamestate &gamestate, const nlohmann::json &input) {

    try {
        if (!input.contains("type") || !input.contains("payload")) return false;
        
        InputType type = stringToInputType(input.at("type").get<std::string>());
        const auto& payload = input.at("payload");

        switch (type) {
            case InputType::STEP_GAME:       return handleStepGame(gamestate, payload);
            case InputType::BUILD_FACTORY:    return handleBuildFactory(gamestate, payload);
            case InputType::SCRAP_FACTORY:    return handleScrapFactory(gamestate, payload);
            case InputType::RENAME_FACTORY:   return handleRenameFactory(gamestate, payload);
            case InputType::CHANGE_WORKFORCE: return handleChangeWorkforce(gamestate, payload);
            case InputType::MARKET_BUY_ITEM:  return handleMarketBuyItem(gamestate, payload);
            case InputType::MARKET_SELL_ITEM: return handleMarketSellItem(gamestate, payload);
            case InputType::ADD_MONEY:        return handleAddMoney(gamestate, payload);
            case InputType::ADD_ITEM:         return handleAddItem(gamestate, payload);
            default: return false;
        }
    } catch (...) { return false; }
}

// --- İşleyiciler ---

bool InputHandler::handleStepGame(Gamestate &gamestate, const nlohmann::json &payload) {
    size_t times = payload.value("times", 1);
    for (size_t i = 0; i < times; ++i) {
        GameManager::stepGamestate(gamestate);
    }
    return true;
}

bool InputHandler::handleBuildFactory(Gamestate &gamestate, const nlohmann::json &payload) {
    std::string templateId = payload.at("templateId").get<std::string>();
    
    if (!FactoryManager::factories.count(templateId)) return false;

    uuids::uuid playerId = gamestate.getPlayerCompanyId();
    Company* player = gamestate.getCompany(playerId);
    if (!player) return false;

    // Fabrika oluştur ve kaydet
    Factory newFactory(templateId, playerId);
    if (payload.contains("customName")) newFactory.setName(payload.at("customName"));

    uuids::uuid fId = newFactory.getId();
    gamestate.addFactory(newFactory); // Gamestate map'ine ekle
    player->addFactory(fId);          // Şirkete bağla
    
    return true;
}

bool InputHandler::handleScrapFactory(Gamestate &gamestate, const nlohmann::json &payload) {
    uuids::uuid fId = uuids::uuid::from_string(payload.at("factoryId").get<std::string>()).value();
    Factory* f = gamestate.getFactory(fId);
    if (!f) return false;

    Company* owner = gamestate.getCompany(f->getOwnerId());
    if (owner) owner->removeFactoryRef(fId);

    // Gamestate'ten tamamen sil (C++ map erase)
    gamestate.getFactories().erase(fId);
    return true;
}

bool InputHandler::handleRenameFactory(Gamestate &gamestate, const nlohmann::json &payload) {
    uuids::uuid fId = uuids::uuid::from_string(payload.at("factoryId").get<std::string>()).value();
    Factory* f = gamestate.getFactory(fId);
    if (!f) return false;

    f->setName(payload.at("newName"));
    return true;
}

bool InputHandler::handleChangeWorkforce(Gamestate &gamestate, const nlohmann::json &payload) {
    uuids::uuid fId = uuids::uuid::from_string(payload.at("factoryId").get<std::string>()).value();
    int delta = payload.at("delta").get<int>(); // +10 veya -5
    
    Factory* f = gamestate.getFactory(fId);
    if (!f) return false;

    if (delta > 0) f->addEmployees(static_cast<size_t>(delta));
    else f->setEmployeeCount(f->getEmployeeCount() > abs(delta) ? f->getEmployeeCount() - abs(delta) : 0);
    
    return true;
}

bool InputHandler::handleMarketBuyItem(Gamestate &gamestate, const nlohmann::json &payload) {
    // 1. Gerekli ID'leri ve Verileri Çek
    // Eğer payload'da "ownerId" yoksa, varsayılan olarak Player ID'si kullanılır (Fallback)
    uuids::uuid ownerId;
    if (payload.contains("ownerId")) {
        ownerId = uuids::uuid::from_string(payload.at("ownerId").get<std::string>()).value();
    } else {
        ownerId = gamestate.getPlayerCompanyId();
    }

    uuids::uuid mId = uuids::uuid::from_string(payload.at("marketId").get<std::string>()).value();
    std::string itemId = payload.at("itemId").get<std::string>();
    float amount = payload.at("amount").get<float>();
    
    // UI'dan fiyat gelmeli. Gelmiyorsa "Market Fiyatı" varsayılır (Bunu UI tarafında çözmek daha iyi)
    // Şimdilik zorunlu tutalım:
    double priceLimit = payload.at("price").get<double>(); 

    // 2. Objeleri Bul
    Market* market = gamestate.getMarket(mId);
    if (!market) return false;

    // Not: Şirket veya Node olup olmadığını kontrol etmemize gerek yok, 
    // Market::placeOrder içindeki 'getTrader' zaten bunu kontrol edip hata veriyor.
    
    // 3. Emri Oluştur (Constructor sayesinde tek satır)
    MarketOrder order(
        ownerId,
        itemId,
        OrderType::BUY,
        priceLimit,
        amount
    );

    // 4. Emri Markete İlet
    // placeOrder void dönüyor ama içeride hata olursa konsola basıyor.
    // Burada return true diyebiliriz çünkü emir başarıyla iletildi.
    market->placeOrder(gamestate, order);
    
    return true; 
}

bool InputHandler::handleMarketSellItem(Gamestate &gamestate, const nlohmann::json &payload) {
    // 1. Owner ID Çözümleme
    uuids::uuid ownerId;
    if (payload.contains("ownerId")) {
        ownerId = uuids::uuid::from_string(payload.at("ownerId").get<std::string>()).value();
    } else {
        ownerId = gamestate.getPlayerCompanyId();
    }

    uuids::uuid mId = uuids::uuid::from_string(payload.at("marketId").get<std::string>()).value();
    std::string itemId = payload.at("itemId").get<std::string>();
    float amount = payload.at("amount").get<float>();
    double priceLimit = payload.at("price").get<double>(); 

    Market* market = gamestate.getMarket(mId);
    if (!market) return false;

    // 2. Satış Emri Oluştur
    MarketOrder order(
        ownerId,
        itemId,
        OrderType::SELL,
        priceLimit,
        amount
    );

    // 3. Markete İlet
    market->placeOrder(gamestate, order);
    
    return true;
}

bool InputHandler::handleAddMoney(Gamestate &gamestate, const nlohmann::json &payload) {
    Company* player = gamestate.getCompany(gamestate.getPlayerCompanyId());
    if (!player) return false;
    player->setCapital(player->getCapital() + payload.at("amount").get<double>());
    return true;
}

bool InputHandler::handleAddItem(Gamestate &gamestate, const nlohmann::json &payload) {
    Company* player = gamestate.getCompany(gamestate.getPlayerCompanyId());
    if (!player) return false;
    
    player->getStorage().add(
        payload.at("itemId").get<std::string>(), 
        payload.at("amount").get<float>()
    );
    
    return true;
}