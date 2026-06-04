// src/AI/Evaluators/EconomyEvaluator.cpp
#include "EconomyEvaluator.h"
#include "Registry/FactoryManager.h"
#include "Registry/ItemManager.h"
#include "Registry/PipelineManager.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Orderbook.h"
#include "World/Components/MarketComponent.h"
#include "World/Systems/MarketSystem.h"
#include "Core/ECS/Entity.h"
#include <unordered_set>

namespace EconomyEvaluator {

    float scoreFactoryProfitability(const std::string& templateId, Gamestate& gamestate) {
        if (FactoryManager::factories.find(templateId) == FactoryManager::factories.end()) {
            return -9999.0f;
        }
        const auto& fData = FactoryManager::factories.at(templateId);

        float expectedCost = 0.0f;
        float expectedRevenue = 0.0f;
        float demandBonus = 1.0f;

        auto getAveragePrice = [&gamestate](const std::string& itemId) -> float {
            float total = 0.0f;
            int count = 0;
            for (auto* entity : gamestate.getEntitiesByType("market")) {
                auto* mComp = entity->GetComponent<MarketComponent>("MarketComponent");
                if (mComp) {
                    double price = mComp->getPrice(itemId);
                    if (price <= 0.0 && ItemManager::items.count(itemId)) {
                        price = static_cast<double>(ItemManager::items.at(itemId).base_price);
                    }
                    total += price;
                    count++;
                }
            }
            return count > 0 ? (total / count) : 1.0f;
        };

        for (const auto& pipeId : fData.pipeline_ids) {
            if (PipelineManager::pipelines.count(pipeId)) {
                const auto& pipe = PipelineManager::pipelines.at(pipeId);

                for (const auto& in : pipe.inputs) {
                    if (in.id == "core_none_000") continue;
                    float price = getAveragePrice(in.id);
                    expectedCost += price * in.quantity;
                }

                for (const auto& out : pipe.outputs) {
                    float rev = getAveragePrice(out.id) * out.quantity;
                    expectedRevenue += rev;

                    // Talep bonusu: output'a ne kadar BUY order varsa
                    float buyVol = MarketSystem::getTotalBuyVolume(gamestate, out.id);
                    if (buyVol > 0.0f) {
                        demandBonus += buyVol * 0.1f; // her 10 birim talep +1.0 bonus
                    }
                }
            }
        }

        return expectedRevenue * demandBonus - expectedCost;
    }

    float scoreInvestmentDesire(Entity& aiEntity) {
        auto* wallet = aiEntity.GetComponent<WalletComponent>("WalletComponent");
        if (!wallet) return -1.0f;

        float desire = (wallet->balance - 5000.0f) / 1000.0f;

        if (wallet->debt > 0) {
            desire -= (wallet->debt / 500.0f);
        }

        return desire;
    }
}
