// src/AI/Evaluators/EconomyEvaluator.cpp
#include "EconomyEvaluator.h"
#include "Core/GameConstants.h"
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

    float scoreFactoryProfitability(const std::string& templateId, Gamestate& gamestate,
                                    const std::unordered_map<std::string, float>* categoryPrefs) {
        if (FactoryManager::factories.find(templateId) == FactoryManager::factories.end()) {
            return GameConstants::MISSING_TEMPLATE_SCORE;
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

                    float buyVol = MarketSystem::getTotalBuyVolume(gamestate, out.id);
                    if (buyVol > 0.0f) {
                        demandMultiplier += std::min(buyVol * GameConstants::DEMAND_BONUS_SCALE, GameConstants::DEMAND_BONUS_CAP);
                    }
                }
            }
        }

        float netProfit = expectedRevenue * demandMultiplier - expectedCost;
        if (netProfit <= 0.0f) return netProfit;
        float score = (netProfit / static_cast<float>(fData.buildCost)) * GameConstants::ROI_SCALE;

        // Category preference multiplier
        if (categoryPrefs) {
            for (const auto& cat : fData.categories) {
                auto it = categoryPrefs->find(cat);
                if (it != categoryPrefs->end()) {
                    score *= it->second;
                }
            }
        }

        return score;
    }

    float scoreInvestmentDesire(Entity& aiEntity, float investThreshold, float investDivisor) {
        auto* wallet = aiEntity.GetComponent<WalletComponent>("WalletComponent");
        if (!wallet) return GameConstants::NO_WALLET_SCORE;

        float desire = (wallet->balance - investThreshold) / investDivisor;

        if (wallet->debt > 0) {
            desire -= (wallet->debt / GameConstants::DEBT_PENALTY_DIVISOR);
        }

        auto* assets = aiEntity.GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
        if (assets && assets->ownedAssets.empty()) {
            desire += GameConstants::FIRST_FACTORY_BONUS;
        }

        return desire;
    }
}
