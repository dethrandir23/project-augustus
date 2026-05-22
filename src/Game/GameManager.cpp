// GameManager.cpp
#include "GameManager.h"
#include "Core/Components/InventoryComponent.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/PerkComponent.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "Economy/Components/ManpowerPoolComponent.h"
#include "Economy/EconomyUtils.h"
#include "World/Components/DemographicsComponent.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/MarketMemberComponent.h"
#include "World/Systems/MarketSystem.h"
#include "AI/AIManager.h"
#include "AI/Components/AIControllerComponent.h"

void GameManager::tick(Gamestate &gamestate) {
    gamestate.advanceDate();

    processPerks(gamestate);

    processCompanyLogistics(gamestate);

    processFactories(gamestate);

    processTradeNodes(gamestate);
    processDemographics(gamestate);
    processAI(gamestate);

    // DEBUG: TO CHECK IF GAME REALLY TICKS
    // Entity* playerCompany = gamestate.getPlayerCompany();
    // if (playerCompany) {
    //     auto* mp = playerCompany->GetComponent<ManpowerPoolComponent>("ManpowerPoolComponent");
    //     if (mp) mp->add(100);
    // }

    gamestate.getEventHandler().tickEvents(gamestate);
}

void GameManager::update(Gamestate &gamestate, float deltaTime) {
    if (gamestate.paused) return;

    gamestate.accumulator += deltaTime;
    float secondsPerTurn = 1.0f / static_cast<float>(gamestate.gameSpeed);

    while (gamestate.accumulator >= secondsPerTurn) {
        tick(gamestate);
        gamestate.accumulator -= secondsPerTurn;
    }
}

// ==========================================
// SYSTEM IMPLEMENTATIONS (Sistem Gövdeleri)
// ==========================================

void GameManager::processPerks(Gamestate& gamestate) {
    for (auto& [id, entity] : gamestate.getEntities()) {
        auto* perks = entity->GetComponent<PerkComponent>("PerkComponent");
        if (perks) {
            perks->activePerks.erase(
                std::remove_if(perks->activePerks.begin(), perks->activePerks.end(),
                [](ActivePerk &p) {
                    if (p.remainingDuration == -1) return false;
                    p.remainingDuration--;
                    return p.remainingDuration <= 0;
                }), perks->activePerks.end()
            );
        }
    }
}

void GameManager::processDemographics(Gamestate& gamestate) {
    for (auto& [id, entity] : gamestate.getEntities()) {
        auto* demo = entity->GetComponent<DemographicsComponent>("DemographicsComponent");
        if (!demo) continue;

        float growthFactor = (demo->happiness - 0.5f) * 0.1f;
        int change = static_cast<int>(demo->population * growthFactor);
        
        if (change > 0) {
            demo->population += change;
            demo->recruitablePop += static_cast<size_t>(change * 0.4);
        } else {
            size_t loss = static_cast<size_t>(-change);
            demo->population = (demo->population > loss) ? (demo->population - loss) : 0;
            size_t workerLoss = static_cast<size_t>(loss * 0.4);
            demo->recruitablePop = (demo->recruitablePop > workerLoss) ? (demo->recruitablePop - workerLoss) : 0;
        }
    }
}

void GameManager::processCompanyLogistics(Gamestate& gamestate) {
    for (auto& [id, entity] : gamestate.getEntities()) {
        if (entity->GetType() != "company") continue;

        auto* mainStorage = entity->GetComponent<InventoryComponent>("Storage");
        auto* manpower = entity->GetComponent<ManpowerPoolComponent>("ManpowerPoolComponent");
        auto* assets = entity->GetComponent<AssetOwnerComponent>("AssetOwnerComponent");

        if (!mainStorage || !manpower || !assets) continue;

        for (const auto& factoryId : assets->ownedAssets) {
            Entity* factory = gamestate.getEntity(factoryId);
            if (!factory) continue;

            auto* facWorkforce = factory->GetComponent<WorkforceComponent>("WorkforceComponent");
            auto* facInput = factory->GetComponent<InventoryComponent>("inputStorage");
            auto* facOutput = factory->GetComponent<InventoryComponent>("outputStorage");
            auto* facProd = factory->GetComponent<ProductionComponent>("ProductionComponent");

            if (!facWorkforce || !facInput || !facOutput || !facProd) continue;

            // 1. İşçi Gönder
            if (manpower->availableWorkers > 0) {
                size_t space = (facWorkforce->maxWorkers > facWorkforce->currentWorkers) 
                               ? (facWorkforce->maxWorkers - facWorkforce->currentWorkers) : 0;
                size_t toAdd = std::min(static_cast<size_t>(100), space);
                toAdd = std::min(toAdd, manpower->availableWorkers);
                facWorkforce->currentWorkers += toAdd;
                manpower->availableWorkers -= toAdd;
            }

            // 2. Hammadde Gönder
            auto missingInputs = EconomyUtils::getMissingItems(facProd->templateId, facInput->GetInternalInventory());
            for (const auto& req : missingInputs) {
                float availableInCompany = mainStorage->GetInternalInventory().getAmount(req.id);
                if (availableInCompany > 0) {
                    float amountToSend = std::min(availableInCompany, req.quantity);
                    mainStorage->Remove(req.id, amountToSend);
                    facInput->Add(req.id, amountToSend);
                }
            }

            // 3. Ürünleri Topla
            for (const auto& item : facOutput->GetAllItems()) {
                mainStorage->Add(item.id, item.quantity);
            }
            facOutput->Clear();
        }
    }
}

void GameManager::processFactories(Gamestate& gamestate) {
    for (auto& [id, entity] : gamestate.getEntities()) {
        if (entity->GetType() != "factory") continue;
        // Demin Utility'e yazdığımız genel üretimi çağırıyoruz
        // NOT: EconomyUtils içindeki o produce kodunu buraya entegre ettiğini varsayıyorum
        EconomyUtils::produce(*entity, 1.0); // 1.0 global modifier
    }
}

void GameManager::processTradeNodes(Gamestate& gamestate) {
    for (auto& [id, entity] : gamestate.getEntities()) {
        if (entity->GetType() != "trade_node") continue;

        auto* wallet = entity->GetComponent<WalletComponent>("WalletComponent");
        auto* storage = entity->GetComponent<InventoryComponent>("Storage");
        auto* consumption = entity->GetComponent<ProductionComponent>("consumption");
        auto* marketMember = entity->GetComponent<MarketMemberComponent>("MarketMemberComponent");
        auto* demo = entity->GetComponent<DemographicsComponent>("DemographicsComponent");

        if (!wallet || !storage || !consumption || !marketMember || !demo) continue;

        // 0. YEREL ÜRETİM (Örn: tarım, kumaş vb.)
        EconomyUtils::executeProduction(*entity, "LocalProduction", "Storage");

        // 1. OTOMATİK TEDARİK (Pazara Alış Emri Gir)
        // auto missing = EconomyUtils::getMissingItems(consumption->templateId, storage->GetInternalInventory());
        // for (const auto& req : missing) {
        //     // Fiyatı piyasadan öğrenelim (Marketi bulup sormamız lazım)
        //     Entity* marketEntity = gamestate.getEntity(marketMember->marketId);
        //     if (marketEntity) {
        //         auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
        //         if (marketComp) {
        //             double currentPrice = marketComp->getPrice(req.id);
                    
        //             // Alış Emri Oluştur (MarketSystem üzerinden)
        //             MarketOrder buyOrder(entity->GetId(), req.id, OrderType::BUY, currentPrice, req.quantity);
        //             MarketSystem::placeOrder(gamestate, marketEntity->GetId(), buyOrder);
        //         }
        //     }
        // }
        // NO MORE REQUIRED AFTER AI COMPONENT.

        // 2. TÜKETİM VE MUTLULUK
        EconomyUtils::executeProduction(*entity, "consumption", "Storage");

        float currentHappinessItems = storage->GetInternalInventory().getAmount("core_happiness_043");
        float requiredHappiness = demo->population * 0.01f;
        if (requiredHappiness > 0.001f) {
            demo->happiness = std::clamp(currentHappinessItems / requiredHappiness, 0.0f, 1.0f);
        }
        storage->GetInternalInventory().remove("core_happiness_043", currentHappinessItems);

        // 3. BÜTÇE YÖNETİMİ (Fazlalıkları Satış Emri Olarak Gir)
        // İleride yapay zeka buraya satış emirlerini (SELL) girecek.
    }
}

void GameManager::processAI(Gamestate& gamestate) {
    AIManager::processAll(gamestate);
}