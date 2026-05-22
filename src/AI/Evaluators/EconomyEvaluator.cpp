// src/AI/Evaluators/EconomyEvaluator.cpp
#include "EconomyEvaluator.h"
#include "Registry/FactoryManager.h"
#include "Registry/ItemManager.h"
#include "Registry/PipelineManager.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Orderbook.h"
#include "World/Components/MarketComponent.h"
#include "Core/ECS/Entity.h"
#include <unordered_set>

namespace EconomyEvaluator {

    // Check if any market has sell orders for this item
    static bool isItemAvailableOnMarket(Gamestate& gamestate, const std::string& itemId) {
        for (auto* entity : gamestate.getEntitiesByType("market")) {
            auto* mComp = entity->GetComponent<MarketComponent>("MarketComponent");
            if (!mComp) continue;
            OrderBook* book = mComp->getBook(itemId);
            if (book && book->getBestAsk() > 0.0) return true;
        }
        return false;
    }

    float scoreFactoryProfitability(const std::string& templateId, Gamestate& gamestate) {
        if (FactoryManager::factories.find(templateId) == FactoryManager::factories.end()) {
            return -9999.0f;
        }
        const auto& fData = FactoryManager::factories.at(templateId);

        float expectedCost = 0.0f;
        float expectedRevenue = 0.0f;
        bool hasUnavailableInput = false;

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
                    // Penalize inputs that have no sell orders on any market
                    if (!isItemAvailableOnMarket(gamestate, in.id)) {
                        hasUnavailableInput = true;
                        expectedCost += price * in.quantity * 5.0f;
                    } else {
                        expectedCost += price * in.quantity;
                    }
                }
                
                for (const auto& out : pipe.outputs) {
                    expectedRevenue += getAveragePrice(out.id) * out.quantity;
                }
            }
        }
        
        // Heavy penalty if any input is completely unavailable on market
        if (hasUnavailableInput) expectedCost *= 2.0f;
        
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