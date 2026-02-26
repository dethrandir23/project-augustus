#include "InputHandler.h"
#include "GameManager.h"
#include "Entity/EntityFactory.h" // createFactory için
#include "World/Systems/MarketSystem.h" // placeOrder için
#include "Core/Components/InventoryComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "DevTools/Console.h"
#include <iostream>

// Statik değişkeni tanımlama
std::unordered_map<std::string, InputHandler::ActionCallback> InputHandler::actionRegistry;

void InputHandler::registerAction(const std::string& actionName, ActionCallback callback) {
    actionRegistry[actionName] = callback;
}

bool InputHandler::handleInput(Gamestate& gamestate, const nlohmann::json& input) {
    try {
        if (!input.contains("type") || !input.contains("payload")) return false;
        
        std::string type = input.at("type").get<std::string>();
        const auto& payload = input.at("payload");

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
uuids::uuid getTargetCompanyId(Gamestate& gamestate, const nlohmann::json& payload) {
    if (payload.contains("companyId")) {
        return uuids::uuid::from_string(payload.at("companyId").get<std::string>()).value();
    }
    return gamestate.getPlayerCompanyId();
}

// --- KOMUTLARIN KAYDEDİLMESİ (INIT) ---
// Motor başlarken bu fonksiyon 1 kere çağrılacak.
void InputHandler::init() {

    // 1. STEP_GAME
    registerAction("STEP_GAME", [](Gamestate& gamestate, const nlohmann::json& payload) {
        size_t times = payload.value("times", 1);
        for (size_t i = 0; i < times; ++i) {
            GameManager::tick(gamestate); // stepGamestate -> tick oldu
        }
        return true;
    });

    // 2. ADD_MONEY (ECS Mantığıyla)
    registerAction("ADD_MONEY", [](Gamestate& gamestate, const nlohmann::json& payload) {
        uuids::uuid targetId = getTargetCompanyId(gamestate, payload);
        Entity* company = gamestate.getEntity(targetId);
        if (!company) return false;

        auto* wallet = company->GetComponent<WalletComponent>("WalletComponent");
        if (wallet) {
            wallet->addMoney(payload.at("amount").get<double>());
            return true;
        }
        return false;
    });

    // 3. ADD_ITEM (ECS Mantığıyla)
    registerAction("ADD_ITEM", [](Gamestate& gamestate, const nlohmann::json& payload) {
        uuids::uuid targetId = getTargetCompanyId(gamestate, payload);
        Entity* company = gamestate.getEntity(targetId);
        if (!company) return false;

        auto* storage = company->GetComponent<InventoryComponent>("MainStorage");
        if (storage) {
            storage->Add(payload.at("itemId").get<std::string>(), payload.at("amount").get<float>());
            return true;
        }
        return false;
    });

    // 4. BUILD_FACTORY (ECS Mantığıyla)
    registerAction("BUILD_FACTORY", [](Gamestate& gamestate, const nlohmann::json& payload) {
        std::string templateId = payload.at("templateId").get<std::string>();
        
        if (!FactoryManager::factories.count(templateId)) return false;
        const auto& fData = FactoryManager::factories.at(templateId);

        uuids::uuid ownerId = getTargetCompanyId(gamestate, payload);
        Entity* ownerCompany = gamestate.getEntity(ownerId);
        if (!ownerCompany) return false;

        auto* wallet = ownerCompany->GetComponent<WalletComponent>("WalletComponent");
        auto* assets = ownerCompany->GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
        
        if (!wallet || !assets) return false;

        // Maliyet kontrolü buraya eklenebilir
        // if (!wallet->trySpend(fData.buildCost)) return false;

        std::string customName = payload.value("customName", "");
        
        // Factory Yarat (Demin yazdığımız Builder/Factory metodu)
        //Entity* newFactory = EntityBuilder::createFactory(fData, ownerId, customName); // pointer döndürdüğünü varsaydım
        Entity* newFactory = createFactory(fData, ownerId, customName);

        // Gamestate'e kaydet
        gamestate.addEntity(newFactory);
        
        // Şirketin varlıklarına ekle
        assets->addAsset(newFactory->GetId());

        return true;
    });

    // 5. SCRAP_FACTORY (ECS Mantığıyla)
    registerAction("SCRAP_FACTORY", [](Gamestate& gamestate, const nlohmann::json& payload) {
        uuids::uuid fId = uuids::uuid::from_string(payload.at("factoryId").get<std::string>()).value();
        Entity* factory = gamestate.getEntity(fId);
        if (!factory) return false;

        // Sahibini bulup listesinden çıkaralım
        auto* ownerComp = factory->GetComponent<OwnerComponent>("OwnerComponent");
        if (ownerComp) {
            Entity* owner = gamestate.getEntity(ownerComp->getOwnerId());
            if (owner) {
                auto* assets = owner->GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
                if (assets) assets->removeAsset(fId);
            }
        }

        // Gamestate'ten tamamen sil
        // Not: std::unordered_map'ten silmek için Gamestate'e bir removeEntity(uuid) metodu eklemelisin.
        gamestate.removeEntity(fId); 
        return true;
    });

    // 6. MARKET_BUY_ITEM (Yeni MarketSystem ile)
    registerAction("MARKET_BUY_ITEM", [](Gamestate& gamestate, const nlohmann::json& payload) {
        uuids::uuid ownerId = getTargetCompanyId(gamestate, payload);
        uuids::uuid mId = uuids::uuid::from_string(payload.at("marketId").get<std::string>()).value();
        
        MarketOrder order(
            ownerId,
            payload.at("itemId").get<std::string>(),
            OrderType::BUY,
            payload.at("price").get<double>(),
            payload.at("amount").get<float>()
        );

        MarketSystem::placeOrder(gamestate, mId, order);
        return true;
    });

    // 7. MARKET_SELL_ITEM (Yeni MarketSystem ile)
    registerAction("MARKET_SELL_ITEM", [](Gamestate& gamestate, const nlohmann::json& payload) {
        uuids::uuid ownerId = getTargetCompanyId(gamestate, payload);
        uuids::uuid mId = uuids::uuid::from_string(payload.at("marketId").get<std::string>()).value();
        
        MarketOrder order(
            ownerId,
            payload.at("itemId").get<std::string>(),
            OrderType::SELL,
            payload.at("price").get<double>(),
            payload.at("amount").get<float>()
        );

        MarketSystem::placeOrder(gamestate, mId, order);
        return true;
    });
    
    // Diğerleri RENAME_FACTORY, CHANGE_WORKFORCE...
    // Onları da benzer mantıkla (Entity üzerinden component çekerek) buraya eklersin.
}