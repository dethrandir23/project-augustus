// src/AI/Logic/TradeNodeLogic.h
#pragma once
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

namespace TradeNodeLogic {

// --- YARDIMCI ---
// TradeNode, bağlı olduğu marketi bilir (MarketMemberComponent).
// Bulamazsa işlem yapmaz.
inline uuids::uuid getMarketId(Entity& node) {
    auto* member = node.GetComponent<MarketMemberComponent>("MarketMemberComponent");
    return member ? member->marketId : uuids::uuid{};
}

inline nlohmann::json makeInput(const std::string& type, nlohmann::json payload,
                                 const Entity& node) {
    payload["companyId"] = uuids::to_string(node.GetId());
    return {{"type", type}, {"payload", payload}};
}

// 1. MANTIK: Üretim Fazlasını Sat
// Şehrin yerel üretimi (LocalProduction) tüketimden fazlasını markete sürer.
inline void sellSurplus(Entity& node, Gamestate& gamestate) {
    auto* storage = node.GetComponent<InventoryComponent>("Storage");
    auto* prod    = node.GetComponent<ProductionComponent>("LocalProduction");
    if (!storage || !prod) return;

    uuids::uuid marketId = getMarketId(node);
    if (marketId.is_nil()) return;

    Entity* marketEntity = gamestate.getEntity(marketId);
    if (!marketEntity) return;
    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    if (!marketComp) return;

    // Her üretilen çıktı türü için: depolananın yarısını sat, yarısını tüketim için sakla.
    // Oran ileride DemographicsComponent'teki happiness'a göre ayarlanabilir.
    for (const auto& pipeId : prod->activePipelineIds) {
        if (!PipelineManager::pipelines.count(pipeId)) continue;
        const auto& pipe = PipelineManager::pipelines.at(pipeId);

        for (const auto& output : pipe.outputs) {
            float inStorage = storage->GetInternalInventory().getAmount(output.id);
            float toSell    = inStorage * 0.5f; // Yarısını sat
            if (toSell < 1.0f) continue;

            double price = marketComp->getPrice(output.id);
            if (price <= 0.0) price = 1.0; // Piyasada fiyat yoksa taban fiyat

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

// 2. MANTIK: Tüketim İçin Alım
// Şehrin consumption pipeline'ı için eksik malları piyasadan al.
// GameManager::processTradeNodes zaten bunu yapıyor ama bu versiyon
// bütçeye duyarlı: çok pahalıysa almaz.
inline void buyConsumptionNeeds(Entity& node, Gamestate& gamestate) {
    auto* storage  = node.GetComponent<InventoryComponent>("Storage");
    auto* wallet   = node.GetComponent<WalletComponent>("WalletComponent");
    auto* demo     = node.GetComponent<DemographicsComponent>("DemographicsComponent");
    auto* cons     = node.GetComponent<ProductionComponent>("consumption");
    if (!storage || !wallet || !demo || !cons) return;

    uuids::uuid marketId = getMarketId(node);
    if (marketId.is_nil()) return;

    Entity* marketEntity = gamestate.getEntity(marketId);
    if (!marketEntity) return;
    auto* marketComp = marketEntity->GetComponent<MarketComponent>("MarketComponent");
    if (!marketComp) return;

    // Bütçenin ne kadarını tüketime ayırabiliriz?
    // Mutluluk düştükçe daha fazla harca (hayatta kalma modu).
    float budgetRatio = 0.3f + (1.0f - demo->happiness) * 0.4f; // %30-%70 arası
    double budget     = wallet->balance * budgetRatio;
    if (budget < 1.0) return;

    auto missing = EconomyUtils::getMissingItems(cons->templateId, storage->GetInternalInventory());

    for (const auto& req : missing) {
        if (budget <= 0.0) break;

        double unitPrice = marketComp->getPrice(req.id);
        if (unitPrice <= 0.0) unitPrice = 1.0;

        // Satın alabildiğim kadar al, bütçeyi aşma
        float affordable = static_cast<float>(budget / unitPrice);
        float toBuy      = std::min(req.quantity, affordable);
        if (toBuy < 0.01f) continue;

        InputHandler::handleInput(gamestate,
            makeInput("MARKET_BUY_ITEM", {
                {"marketId", uuids::to_string(marketId)},
                {"itemId",   req.id},
                {"amount",   toBuy},
                {"price",    unitPrice}
            }, node));

        budget -= toBuy * unitPrice;
    }
}

// 3. MANTIK: Event Kararı
// Şehri etkileyen eventlere (deprem, yangın, kıtlık) cevap ver.
inline void handleEvents(Entity& node, Gamestate& gamestate) {
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

        // V1: Bütçe varsa ilk seçeneği, yoksa en ucuz seçeneği seç.
        // İleride EconomyEvaluator ile option scoring eklenebilir.
        auto* wallet = node.GetComponent<WalletComponent>("WalletComponent");
        size_t chosen = (wallet && wallet->balance > 5000.0) ? 0 : (optionCount - 1);

        const auto& option = ev.tmpl->options[chosen];
        for (const auto& input : option.inputs) {
            nlohmann::json modified = input;
            modified["payload"]["companyId"] = uuids::to_string(node.GetId());
            InputHandler::handleInput(gamestate, modified);
        }

        gamestate.getEventHandler().markAsHandled(ev.id);
        Console::log("[NODE] " + node.GetName() + " karari: " + option.text, LogType::INFO);
    }
}

} // namespace TradeNodeLogic