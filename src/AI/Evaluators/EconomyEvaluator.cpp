// src/AI/Evaluators/EconomyEvaluator.cpp
#include "EconomyEvaluator.h"
#include "Registry/FactoryManager.h"
#include "Registry/PipelineManager.h"
#include "Economy/Components/WalletComponent.h"
#include "World/Components/MarketComponent.h"
#include "Core/ECS/Entity.h"

namespace EconomyEvaluator {

    float scoreFactoryProfitability(const std::string& templateId, Gamestate& gamestate) {
        if (FactoryManager::factories.find(templateId) == FactoryManager::factories.end()) {
            return -9999.0f;
        }
        const auto& fData = FactoryManager::factories.at(templateId);

        float expectedCost = 0.0f;
        float expectedRevenue = 0.0f;

        auto getAveragePrice = [&gamestate](const std::string& itemId) -> float {
            float total = 0.0f;
            int count = 0;
            for (auto* entity : gamestate.getEntitiesByType("market")) {
                auto* mComp = entity->GetComponent<MarketComponent>("MarketComponent");
                if (mComp) {
                    total += mComp->getPrice(itemId);
                    count++;
                }
            }
            return count > 0 ? (total / count) : 1.0f;
        };

        for (const auto& pipeId : fData.pipeline_ids) {
            if (PipelineManager::pipelines.count(pipeId)) {
                const auto& pipe = PipelineManager::pipelines.at(pipeId);
                
                for (const auto& in : pipe.inputs) {
                    expectedCost += getAveragePrice(in.id) * in.quantity;
                }
                
                for (const auto& out : pipe.outputs) {
                    expectedRevenue += getAveragePrice(out.id) * out.quantity;
                }
            }
        }
        
        return expectedRevenue - expectedCost;
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