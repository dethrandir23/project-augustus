#include "CompanyBrain.h"
#include "AI/AIUtils/AIPicker.h"
#include "AI/Evaluators/EconomyEvaluator.h"
#include "Core/Components/InventoryComponent.h"
#include "Core/ECS/Entity.h"
#include "DevTools/Console.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/ManpowerPoolComponent.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "Economy/EconomyUtils.h"
#include "Economy/Orderbook.h"
#include "Game/Gamestate.h"
#include "Game/InputHandler.h"
#include "Registry/FactoryManager.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/MarketMemberComponent.h"
#include "World/Systems/MarketSystem.h"
#include <iostream>
#include <unordered_set>

CompanyBrain::CompanyBrain()
    : rng(seed) {}

CompanyBrain::CompanyBrain(unsigned int s)
    : seed(s), rng(s) {}

nlohmann::json CompanyBrain::makeInput(const std::string& type,
                                        const nlohmann::json& payload,
                                        const Entity& company) const {
    nlohmann::json j = payload;
    j["companyId"] = uuids::to_string(company.GetId());
    return {{"type", type}, {"payload", j}};
}

void CompanyBrain::execute(Entity& entity, Gamestate& gamestate) {
    manageDebt(entity, gamestate);
    buildFactories(entity, gamestate);
    hireWorkers(entity, gamestate);
    buyInputs(entity, gamestate);
    sellSurplus(entity, gamestate);
    handleEvents(entity, gamestate);
}

void CompanyBrain::manageDebt(Entity& company, Gamestate& gamestate) {
    auto* wallet = company.GetComponent<WalletComponent>("WalletComponent");
    if (!wallet || wallet->debt <= 0 || wallet->balance < wallet->debt) return;
    InputHandler::handleInput(gamestate, makeInput("PAY_DEBT", {}, company));
}

void CompanyBrain::hireWorkers(Entity& company, Gamestate& gamestate) {
    auto* wallet = company.GetComponent<WalletComponent>("WalletComponent");
    auto* manpower = company.GetComponent<ManpowerPoolComponent>("ManpowerPoolComponent");
    auto* assets = company.GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
    if (!wallet || !manpower || !assets) return;
    if (assets->ownedAssets.empty()) return;

    for (const auto& fId : assets->ownedAssets) {
        Entity* factory = gamestate.getEntity(fId);
        if (!factory) continue;
        auto* workforce = factory->GetComponent<WorkforceComponent>("WorkforceComponent");
        if (!workforce) continue;

        size_t openSlots = workforce->maxWorkers - workforce->currentWorkers;
        if (openSlots == 0) continue;

        // Bütçenin en fazla %50'sini işçi alımına harca (kalanı input almaya gitsin)
        double hiringBudget = wallet->balance * 0.5;
        double maxHire = hiringBudget / static_cast<double>(hiringCostPerWorker);
        size_t canHire = static_cast<size_t>(std::min<double>(static_cast<double>(openSlots), maxHire));
        if (canHire == 0) continue;

        // Manpower havuzundan kontrol
        size_t fromPool = std::min(canHire, manpower->availableWorkers);
        if (fromPool == 0) continue;

        manpower->allocate(fromPool);
        workforce->recruit(fromPool);
        double cost = static_cast<double>(fromPool) * hiringCostPerWorker;
        wallet->balance -= cost;
    }
}

void CompanyBrain::buyInputs(Entity& company, Gamestate& gamestate) {
    auto* storage = company.GetComponent<InventoryComponent>("Storage");
    auto* wallet  = company.GetComponent<WalletComponent>("WalletComponent");
    auto* assets  = company.GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
    if (!storage || !wallet || !assets) return;

    if (assets->ownedAssets.empty()) return;

    // Şirketlerin bir "local market"i yok, tüm marketlere eşit uzaktalar
    uuids::uuid noMarket;

    for (const auto& fId : assets->ownedAssets) {
        Entity* factory = gamestate.getEntity(fId);
        if (!factory) continue;
        auto* prod = factory->GetComponent<ProductionComponent>("ProductionComponent");
        if (!prod) continue;
        auto* facInput = factory->GetComponent<InventoryComponent>("inputStorage");
        if (!facInput) continue;

        auto missing = EconomyUtils::getMissingItems(prod->templateId,
                                                       facInput->GetInternalInventory());
        if (missing.empty()) continue;

        for (const auto& req : missing) {
            float inStorage = storage->GetInternalInventory().getAmount(req.id);
            float stillNeeded = req.quantity - inStorage;
            if (stillNeeded <= 0.0f) continue;

            // Cross-market: en iyi fiyatı bul
            auto best = MarketSystem::findBestBuyMarket(gamestate, req.id, noMarket);
            if (best.marketId.is_nil()) continue; // hiç arz yok

            double cost = best.effectivePrice * stillNeeded;
            if (cost > wallet->balance * 0.3f) {
                stillNeeded = static_cast<float>((wallet->balance * 0.3) / best.effectivePrice);
            }
            if (stillNeeded < 0.1f) continue;

            std::cout << "[BUY] " << company.GetName() << " buying " << req.id << " x" << stillNeeded << " @ " << best.bestAsk << " (eff=" << best.effectivePrice << ") (bal=" << wallet->balance << ")" << std::endl;

            InputHandler::handleInput(gamestate,
                makeInput("MARKET_BUY_ITEM", {
                    {"marketId", uuids::to_string(best.marketId)},
                    {"itemId",   req.id},
                    {"amount",   stillNeeded},
                    {"price",    best.bestAsk}
                }, company));
        }
    }
}

void CompanyBrain::buildFactories(Entity& company, Gamestate& gamestate) {
    float investDesire = EconomyEvaluator::scoreInvestmentDesire(company, investThreshold, investDivisor);
    if (investDesire <= 0.0f) return;

    std::vector<AIPicker::Candidate> candidates;
    for (const auto& [templateId, fData] : FactoryManager::factories) {
        (void)fData;
        float score = EconomyEvaluator::scoreFactoryProfitability(templateId, gamestate);
        if (score > investMinScore)
            candidates.push_back({templateId, score});
    }
    if (candidates.empty()) return;

    std::string chosen = AIPicker::selectWithNoise(candidates, pickerTopK, pickerTemperature);
    if (chosen.empty()) return;

    InputHandler::handleInput(gamestate,
        makeInput("BUILD_FACTORY", {{"templateId", chosen}}, company));
}

void CompanyBrain::sellSurplus(Entity& company, Gamestate& gamestate) {
    auto* storage = company.GetComponent<InventoryComponent>("Storage");
    auto* assets  = company.GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
    auto* wallet  = company.GetComponent<WalletComponent>("WalletComponent");
    if (!storage || !assets || !wallet) return;

    uuids::uuid marketId;
    double bestPrice = -1.0;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* mc = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!mc) continue;
        double avgPrice = 0.0;
        int count = 0;
        for (const auto& item : storage->GetAllItems()) {
            double p = mc->getPrice(item.id);
            if (p > 0.0) { avgPrice += p; count++; }
        }
        double mean = count > 0 ? avgPrice / count : 0.0;
        if (mean > bestPrice) {
            bestPrice = mean;
            marketId = entity->GetId();
        }
    }
    if (marketId.is_nil()) return;

    // Önce bu şirketin marketteki eski satış emirlerini temizle
    Entity* targetMarket = gamestate.getEntity(marketId);
    if (targetMarket) {
        auto* mc = targetMarket->GetComponent<MarketComponent>("MarketComponent");
        if (mc) {
            for (auto& [itemId, book] : mc->books) {
                for (auto it = book.sellOrders.begin(); it != book.sellOrders.end(); ) {
                    if (it->ownerId == company.GetId()) {
                        storage->Add(it->itemId, it->remaining());
                        it = book.sellOrders.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    }

    // Sahip olunan fabrikalarin inputlarini satma
    std::unordered_set<std::string> factoryInputs;
    for (const auto& fId : assets->ownedAssets) {
        Entity* factory = gamestate.getEntity(fId);
        if (!factory) continue;
        auto* prod = factory->GetComponent<ProductionComponent>("ProductionComponent");
        if (!prod) continue;
        if (FactoryManager::factories.count(prod->templateId)) {
            const auto& fData = FactoryManager::factories.at(prod->templateId);
            for (const auto& pipeId : fData.pipeline_ids) {
                if (!PipelineManager::pipelines.count(pipeId)) continue;
                const auto& pipe = PipelineManager::pipelines.at(pipeId);
                for (const auto& in : pipe.inputs) {
                    if (in.id != "core_none_000")
                        factoryInputs.insert(in.id);
                }
            }
        }
    }

    for (const auto& item : storage->GetAllItems()) {
        if (factoryInputs.count(item.id)) continue;
        float surplus = item.quantity - sellThreshold;
        if (surplus <= 0.0f) continue;

        auto* marketEntity = gamestate.getEntity(marketId);
        if (!marketEntity) continue;
        auto* mc = marketEntity->GetComponent<MarketComponent>("MarketComponent");
        double price = mc ? mc->getPrice(item.id) : 1.0;
        if (price <= 0.0) price = 1.0;

        InputHandler::handleInput(gamestate,
            makeInput("MARKET_SELL_ITEM", {
                {"marketId", uuids::to_string(marketId)},
                {"itemId",   item.id},
                {"amount",   surplus},
                {"price",    price}
            }, company));
    }
}

void CompanyBrain::handleEvents(Entity& company, Gamestate& gamestate) {
    const auto& queue = gamestate.getEventHandler().getQueue();
    for (const auto& ev : queue) {
        if (ev.handled || !ev.tmpl) continue;
        const auto& scope = ev.tmpl->scope;
        bool relevant = std::find(scope.begin(), scope.end(), "COMPANY") != scope.end()
                     || std::find(scope.begin(), scope.end(), "ALL")     != scope.end();
        if (!relevant) continue;
        if (ev.tmpl->auto_handle) continue;

        size_t optionCount = ev.tmpl->options.size();
        if (optionCount == 0) continue;

        std::uniform_int_distribution<size_t> dist(0, optionCount - 1);
        size_t chosen = dist(rng);
        const auto& option = ev.tmpl->options[chosen];
        for (const auto& input : option.inputs) {
            nlohmann::json modified = input;
            modified["payload"]["companyId"] = uuids::to_string(company.GetId());
            InputHandler::handleInput(gamestate, modified);
        }
        gamestate.getEventHandler().markAsHandled(ev.id);
    }
}

nlohmann::json CompanyBrain::toJson() const {
    return {
        {"type", getType()},
        {"sellThreshold", sellThreshold},
        {"investThreshold", investThreshold},
        {"investDivisor", investDivisor},
        {"investMinScore", investMinScore},
        {"hiringCostPerWorker", hiringCostPerWorker},
        {"pickerTopK", pickerTopK},
        {"pickerTemperature", pickerTemperature},
        {"seed", seed}
    };
}

void CompanyBrain::fromJson(const nlohmann::json& j) {
    if (j.contains("sellThreshold")) sellThreshold = j["sellThreshold"].get<float>();
    if (j.contains("investThreshold")) investThreshold = j["investThreshold"].get<float>();
    if (j.contains("investDivisor")) investDivisor = j["investDivisor"].get<float>();
    if (j.contains("investMinScore")) investMinScore = j["investMinScore"].get<float>();
    if (j.contains("hiringCostPerWorker")) hiringCostPerWorker = j["hiringCostPerWorker"].get<float>();
    if (j.contains("pickerTopK")) pickerTopK = j["pickerTopK"].get<int>();
    if (j.contains("pickerTemperature")) pickerTemperature = j["pickerTemperature"].get<double>();
    if (j.contains("seed")) { seed = j["seed"].get<unsigned int>(); rng = std::mt19937(seed); }
}
