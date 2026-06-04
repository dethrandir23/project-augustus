// src/AI/Evaluators/EconomyEvaluator.cpp
#include "EconomyEvaluator.h"
#include "Registry/FactoryManager.h"
#include "Registry/ItemManager.h"
#include "Registry/PipelineManager.h"
#include "Economy/Components/AssetOwnerComponent.h"
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
        float demandMultiplier = 1.0f;

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

                    // Talep bonusu: logaritmik, cap 100% (2.0x)
                    float buyVol = MarketSystem::getTotalBuyVolume(gamestate, out.id);
                    if (buyVol > 0.0f) {
                        demandMultiplier += std::min(buyVol * 0.01f, 1.0f);
                    }
                }
            }
        }

        // ROI bazli skor: (netKar / insaMaliyeti) * 100
        float netProfit = expectedRevenue * demandMultiplier - expectedCost;
        if (netProfit <= 0.0f) return netProfit; // negatifse direkt skor
        return (netProfit / static_cast<float>(fData.buildCost)) * 100.0f;
    }

    float scoreInvestmentDesire(Entity& aiEntity, float investThreshold, float investDivisor) {
        auto* wallet = aiEntity.GetComponent<WalletComponent>("WalletComponent");
        if (!wallet) return -1.0f;

        float desire = (wallet->balance - investThreshold) / investDivisor;

        if (wallet->debt > 0) {
            desire -= (wallet->debt / 500.0f);
        }

        // Ilk fabrikasini kursun diye bonus
        auto* assets = aiEntity.GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
        if (assets && assets->ownedAssets.empty()) {
            desire += 1.0f;
        }

        return desire;
    }
}
