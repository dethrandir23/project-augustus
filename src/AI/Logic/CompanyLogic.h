// src/AI/Logic/CompanyLogic.h
#pragma once
#include "AI/AIUtils/AIPicker.h"
#include "AI/Evaluators/EconomyEvaluator.h"
#include "Core/Components/InventoryComponent.h"
#include "Core/ECS/Entity.h"
#include "DevTools/Console.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Game/Gamestate.h"
#include "Game/InputHandler.h"
#include "Registry/FactoryManager.h"
#include <vector>

namespace CompanyLogic {

// --- YARDIMCI ---
// AI'ın kendi adına input göndermesi için her seferinde companyId'yi payload'a ekler.
inline nlohmann::json makeInput(const std::string& type, nlohmann::json payload,
                                 const Entity& company) {
    payload["companyId"] = uuids::to_string(company.GetId());
    return {{"type", type}, {"payload", payload}};
}

// 1. MANTIK: Borç Yönetimi
// PAY_DEBT action'ı InputHandler'da kayıtlı olmalı.
inline void manageDebt(Entity& company, Gamestate& gamestate) {
    auto* wallet = company.GetComponent<WalletComponent>("WalletComponent");
    if (!wallet || wallet->debt <= 0 || wallet->balance < wallet->debt) return;

    InputHandler::handleInput(gamestate, makeInput("PAY_DEBT", {}, company));
}

// 2. MANTIK: Fabrika İnşası
inline void buildFactories(Entity& company, Gamestate& gamestate) {
    // Yatırım yapacak durumda mıyım?
    float investDesire = EconomyEvaluator::scoreInvestmentDesire(company);
    if (investDesire <= 0.0f) return;

    // Karlı fabrikaları skorla
    std::vector<AIPicker::Candidate> candidates;
    for (const auto& [templateId, fData] : FactoryManager::factories) {
        float score = EconomyEvaluator::scoreFactoryProfitability(templateId, gamestate);
        if (score > 0.5f)
            candidates.push_back({templateId, score});
    }

    if (candidates.empty()) return;

    std::string chosen = AIPicker::selectWithNoise(candidates, 3, 0.7);
    if (chosen.empty()) return;

    // BUILD_FACTORY zaten maliyet kontrolü ve parayı kesmeyi hallediyor.
    // AI hile yapamaz — kurallar InputHandler'da.
    InputHandler::handleInput(gamestate,
        makeInput("BUILD_FACTORY", {{"templateId", chosen}}, company));
}

// 3. MANTIK: Market Satışı
// Şirkette biriken malları markete sat.
inline void sellSurplus(Entity& company, Gamestate& gamestate) {
    auto* storage  = company.GetComponent<InventoryComponent>("MainStorage");
    auto* assets   = company.GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
    auto* wallet   = company.GetComponent<WalletComponent>("WalletComponent");
    if (!storage || !assets || !wallet) return;

    // Şirketin bulunduğu marketi bul (İlk fabrikasının sahibi olduğu market)
    // V1: Sadece ilk market entity'sini al, ileride en yakın / en likit marketi seç.
    uuids::uuid marketId;
    bool found = false;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        marketId = entity->GetId();
        found = true;
        break;
    }
    if (!found) return;

    for (const auto& item : storage->GetAllItems()) {
        // Eşiğin üzerindeyse sat (500 birimden fazlasını sat, buffer'ı koru)
        float surplus = item.quantity - 500.0f;
        if (surplus <= 0.0f) continue;

        InputHandler::handleInput(gamestate,
            makeInput("MARKET_SELL_ITEM", {
                {"marketId", uuids::to_string(marketId)},
                {"itemId",   item.id},
                {"amount",   surplus},
                {"price",    1.0}  // V1: sabit taban fiyat, ileride MarketComponent'ten çekilir
            }, company));
    }
}

// 4. MANTIK: Event Kararı
inline void handleEvents(Entity& company, Gamestate& gamestate) {
    const auto& queue = gamestate.getEventHandler().getQueue();

    for (const auto& ev : queue) {
        if (ev.handled || !ev.tmpl) continue;

        // Bu event şirketi ilgilendiriyor mu?
        const auto& scope = ev.tmpl->scope;
        bool relevant = std::find(scope.begin(), scope.end(), "COMPANY") != scope.end()
                     || std::find(scope.begin(), scope.end(), "ALL")     != scope.end();
        if (!relevant) continue;

        // auto_handle açıksa EventHandler zaten halleder, biz karışmayalım.
        if (ev.tmpl->auto_handle) continue;

        size_t optionCount = ev.tmpl->options.size();
        if (optionCount == 0) continue;

        // V1: Rastgele seçim. İleride EconomyEvaluator ile option scoring eklenebilir.
        size_t chosen = rand() % optionCount;
        const auto& option = ev.tmpl->options[chosen];

        for (const auto& input : option.inputs) {
            // Input'u AI'ın kendi ID'siyle gönder
            nlohmann::json modified = input;
            modified["payload"]["companyId"] = uuids::to_string(company.GetId());
            InputHandler::handleInput(gamestate, modified);
        }

        gamestate.getEventHandler().markAsHandled(ev.id);
        Console::log("[AI] " + company.GetName() + " karari: " + option.text, LogType::INFO);
    }
}

} // namespace CompanyLogic