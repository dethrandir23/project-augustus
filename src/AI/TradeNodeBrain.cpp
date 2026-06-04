#include "TradeNodeBrain.h"
#include "Core/Components/InventoryComponent.h"
#include "Core/ECS/Entity.h"
#include "DevTools/Console.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/EconomyUtils.h"
#include "Economy/Orderbook.h"
#include "Game/Gamestate.h"
#include "Game/InputHandler.h"
#include "Registry/PipelineManager.h"
#include "World/Components/DemographicsComponent.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/MarketMemberComponent.h"
#include "World/Systems/MarketSystem.h"

TradeNodeBrain::TradeNodeBrain()
    : rng(seed) {}

TradeNodeBrain::TradeNodeBrain(unsigned int s)
    : seed(s), rng(s) {}

uuids::uuid TradeNodeBrain::getMarketId(Entity& node) const {
    auto* member = node.GetComponent<MarketMemberComponent>("MarketMemberComponent");
    return member ? member->marketId : uuids::uuid{};
}

nlohmann::json TradeNodeBrain::makeInput(const std::string& type,
                                          const nlohmann::json& payload,
                                          const Entity& node) const {
    nlohmann::json j = payload;
    j["companyId"] = uuids::to_string(node.GetId());
    return {{"type", type}, {"payload", j}};
}

void TradeNodeBrain::execute(Entity& entity, Gamestate& gamestate) {
    buyConsumptionNeeds(entity, gamestate);
    sellSurplus(entity, gamestate);
    handleEvents(entity, gamestate);
}

void TradeNodeBrain::sellSurplus(Entity& node, Gamestate& gamestate) {
    auto* storage = node.GetComponent<InventoryComponent>("Storage");
    auto* prod    = node.GetComponent<ProductionComponent>("LocalProduction");
    if (!storage || !prod) return;
    uuids::uuid localMarketId = getMarketId(node);
    if (localMarketId.is_nil()) return;
    Entity* marketEntity = gamestate.getEntity(localMarketId);
    if (!marketEntity) return;
    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    if (!marketComp) return;

    for (const auto& pipeId : prod->activePipelineIds) {
        if (!PipelineManager::pipelines.count(pipeId)) continue;
        const auto& pipe = PipelineManager::pipelines.at(pipeId);
        for (const auto& output : pipe.outputs) {
            float inStorage = storage->GetInternalInventory().getAmount(output.id);
            float toSell    = inStorage * sellRatio;
            if (toSell < GameConstants::MIN_SELL_UNIT) continue;
            double price = marketComp->getPrice(output.id);
            if (price <= 0.0) price = 1.0;

            // Önce bu item için eski satış emirlerini temizle (güncel fiyattan)
            OrderBook* book = marketComp->getBook(output.id);
            if (book) {
                for (auto it = book->sellOrders.begin(); it != book->sellOrders.end(); ) {
                    if (it->ownerId == node.GetId()) {
                        storage->Add(it->itemId, it->remaining());
                        it = book->sellOrders.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            InputHandler::handleInput(gamestate,
                makeInput("MARKET_SELL_ITEM", {
                    {"marketId", uuids::to_string(localMarketId)},
                    {"itemId",   output.id},
                    {"amount",   toSell},
                    {"price",    price}
                }, node));
        }
    }
}

void TradeNodeBrain::buyConsumptionNeeds(Entity& node, Gamestate& gamestate) {
    auto* storage = node.GetComponent<InventoryComponent>("Storage");
    auto* wallet  = node.GetComponent<WalletComponent>("WalletComponent");
    auto* demo    = node.GetComponent<DemographicsComponent>("DemographicsComponent");
    auto* cons    = node.GetComponent<ProductionComponent>("consumption");
    if (!storage || !wallet || !demo || !cons) return;
    uuids::uuid localMarketId = getMarketId(node);
    if (localMarketId.is_nil()) return;

    float budgetRatio = budgetBaseRatio + (1.0f - demo->happiness) * budgetHappinessWeight;
    double budget     = wallet->balance * budgetRatio;
    if (budget < GameConstants::MIN_BUDGET) return;

    for (const auto& pipeId : cons->activePipelineIds) {
        if (!PipelineManager::pipelines.count(pipeId)) continue;
        const auto& pipe = PipelineManager::pipelines.at(pipeId);
        for (const auto& input : pipe.inputs) {
            if (input.id == "core_none_000") continue;
            float inStorage = storage->GetInternalInventory().getAmount(input.id);
            float target = input.quantity * GameConstants::TRADE_INPUT_BUFFER;
            float needed = target - inStorage;
            if (needed <= 0.0f) continue;

            // Mevcut açık emirleri kontrol et (tüm marketlerde)
            float alreadyOrdered = MarketSystem::getBuyOrderForOwner(gamestate, node.GetId(), input.id);
            needed -= alreadyOrdered;
            if (needed <= 0.0f) continue;

            // En iyi marketi bul (vergi dahil efektif fiyat)
            auto best = MarketSystem::findBestBuyMarket(gamestate, input.id, localMarketId);
            if (best.marketId.is_nil()) continue; // hiç arz yok

            float affordable = static_cast<float>(budget / best.effectivePrice);
            float toBuy = std::min(needed, affordable);
            if (toBuy < GameConstants::MIN_BUY_UNIT) continue;

            InputHandler::handleInput(gamestate,
                makeInput("MARKET_BUY_ITEM", {
                    {"marketId", uuids::to_string(best.marketId)},
                    {"itemId",   input.id},
                    {"amount",   toBuy},
                    {"price",    best.bestAsk}
                }, node));
            budget -= toBuy * best.effectivePrice;
        }
    }
}

void TradeNodeBrain::handleEvents(Entity& node, Gamestate& gamestate) {
    const auto& queue = gamestate.getEventHandler().getQueue();
    for (const auto& ev : queue) {
        if (ev.handled || !ev.tmpl) continue;
        const auto& scope = ev.tmpl->scope;
        bool relevant = std::find(scope.begin(), scope.end(), "TRADE_NODE") != scope.end()
                     || std::find(scope.begin(), scope.end(), "ALL")        != scope.end();
        if (!relevant) continue;
        if (ev.tmpl->auto_handle) continue;

        size_t optionCount = ev.tmpl->options.size();
        if (optionCount == 0) continue;

        auto* wallet = node.GetComponent<WalletComponent>("WalletComponent");
        size_t chosen = (wallet && wallet->balance > richThreshold) ? 0 : (optionCount - 1);

        const auto& option = ev.tmpl->options[chosen];
        for (const auto& input : option.inputs) {
            nlohmann::json modified = input;
            modified["payload"]["companyId"] = uuids::to_string(node.GetId());
            InputHandler::handleInput(gamestate, modified);
        }
        gamestate.getEventHandler().markAsHandled(ev.id);
    }
}

nlohmann::json TradeNodeBrain::toJson() const {
    return {
        {"type", getType()},
        {"sellRatio", sellRatio},
        {"budgetBaseRatio", budgetBaseRatio},
        {"budgetHappinessWeight", budgetHappinessWeight},
        {"richThreshold", richThreshold},
        {"seed", seed}
    };
}

void TradeNodeBrain::fromJson(const nlohmann::json& j) {
    if (j.contains("sellRatio")) sellRatio = j["sellRatio"].get<float>();
    if (j.contains("budgetBaseRatio")) budgetBaseRatio = j["budgetBaseRatio"].get<float>();
    if (j.contains("budgetHappinessWeight")) budgetHappinessWeight = j["budgetHappinessWeight"].get<float>();
    if (j.contains("richThreshold")) richThreshold = j["richThreshold"].get<float>();
    if (j.contains("seed")) { seed = j["seed"].get<unsigned int>(); rng = std::mt19937(seed); }
}
