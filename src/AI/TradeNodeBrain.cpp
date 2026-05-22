#include "TradeNodeBrain.h"
#include "Core/Components/InventoryComponent.h"
#include "Core/ECS/Entity.h"
#include "DevTools/Console.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/EconomyUtils.h"
#include "Game/Gamestate.h"
#include "Game/InputHandler.h"
#include "Registry/PipelineManager.h"
#include "World/Components/DemographicsComponent.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/MarketMemberComponent.h"

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
    uuids::uuid marketId = getMarketId(node);
    if (marketId.is_nil()) return;
    Entity* marketEntity = gamestate.getEntity(marketId);
    if (!marketEntity) return;
    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    if (!marketComp) return;

    for (const auto& pipeId : prod->activePipelineIds) {
        if (!PipelineManager::pipelines.count(pipeId)) continue;
        const auto& pipe = PipelineManager::pipelines.at(pipeId);
        for (const auto& output : pipe.outputs) {
            float inStorage = storage->GetInternalInventory().getAmount(output.id);
            float toSell    = inStorage * sellRatio;
            if (toSell < 1.0f) continue;
            double price = marketComp->getPrice(output.id);
            if (price <= 0.0) price = 1.0;
            InputHandler::handleInput(gamestate,
                makeInput("MARKET_SELL_ITEM", {
                    {"marketId", uuids::to_string(marketId)},
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
    uuids::uuid marketId = getMarketId(node);
    if (marketId.is_nil()) return;
    Entity* marketEntity = gamestate.getEntity(marketId);
    if (!marketEntity) return;
    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    if (!marketComp) return;

    float budgetRatio = budgetBaseRatio + (1.0f - demo->happiness) * budgetHappinessWeight;
    double budget     = wallet->balance * budgetRatio;
    if (budget < 1.0) return;

    // Walk all consumption pipelines to find missing inputs
    for (const auto& pipeId : cons->activePipelineIds) {
        if (!PipelineManager::pipelines.count(pipeId)) continue;
        const auto& pipe = PipelineManager::pipelines.at(pipeId);
        for (const auto& input : pipe.inputs) {
            if (input.id == "core_none_000") continue;
            float inStorage = storage->GetInternalInventory().getAmount(input.id);
            float target = input.quantity * 5.0f;
            float needed = target - inStorage;
            if (needed <= 0.0f) continue;

            double unitPrice = marketComp->getPrice(input.id);
            if (unitPrice <= 0.0) unitPrice = 1.0;
            float affordable = static_cast<float>(budget / unitPrice);
            float toBuy = std::min(needed, affordable);
            if (toBuy < 0.01f) continue;

            InputHandler::handleInput(gamestate,
                makeInput("MARKET_BUY_ITEM", {
                    {"marketId", uuids::to_string(marketId)},
                    {"itemId",   input.id},
                    {"amount",   toBuy},
                    {"price",    unitPrice}
                }, node));
            budget -= toBuy * unitPrice;
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
