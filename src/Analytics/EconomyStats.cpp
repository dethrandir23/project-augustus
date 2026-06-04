#include "Analytics/EconomyStats.h"
#include "Core/Components/InventoryComponent.h"
#include "Core/Inventory.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "Economy/Orderbook.h"
#include "Registry/FactoryManager.h"
#include "Registry/ItemManager.h"
#include "Registry/PipelineManager.h"
#include "World/Components/DemographicsComponent.h"
#include "World/Components/MarketComponent.h"

namespace Analytics {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static double valueInventory(Inventory& inv) {
    double total = 0.0;
    for (const auto& stack : inv.getAllItems()) {
        if (ItemManager::items.count(stack.id)) {
            total += stack.quantity * static_cast<double>(ItemManager::items.at(stack.id).base_price);
        }
    }
    return total;
}

static double estimateFactoryPhysicalValue(Gamestate& gs, const uuids::uuid& factoryId) {
    Entity* f = gs.getEntity(factoryId);
    if (!f) return 0.0;

    auto* prodComp = f->GetComponent<ProductionComponent>("ProductionComponent");
    auto* workComp = f->GetComponent<WorkforceComponent>("WorkforceComponent");
    auto* inputInv = f->GetComponent<InventoryComponent>("inputStorage");
    auto* outputInv = f->GetComponent<InventoryComponent>("outputStorage");

    if (!prodComp) return 0.0;

    double buildCost = 5000.0;
    if (FactoryManager::factories.count(prodComp->templateId)) {
        buildCost = FactoryManager::factories.at(prodComp->templateId).buildCost;
    }

    double util = 0.2;
    if (workComp && workComp->maxWorkers > 0) {
        util = 0.2 + 0.8 * std::min(1.0, static_cast<double>(workComp->currentWorkers) / workComp->maxWorkers);
    }

    double assetValue = buildCost * util;
    double storageValue = 0.0;
    if (inputInv) storageValue += valueInventory(inputInv->GetInternalInventory());
    if (outputInv) storageValue += valueInventory(outputInv->GetInternalInventory());

    return assetValue + storageValue;
}

static double estimateProductionGDP(Entity& entity) {
    auto* prodComp = entity.GetComponent<ProductionComponent>("ProductionComponent");
    if (!prodComp) return 0.0;

    double efficiency = prodComp->efficiency;
    // For factories, also factor in worker utilization
    auto* workComp = entity.GetComponent<WorkforceComponent>("WorkforceComponent");
    if (workComp && workComp->maxWorkers > 0) {
        double workerRatio = static_cast<double>(workComp->currentWorkers) / workComp->maxWorkers;
        efficiency *= workerRatio;
    }

    if (efficiency <= 0.001) return 0.0;

    double totalOutputValue = 0.0;
    for (const auto& pipeId : prodComp->activePipelineIds) {
        if (!PipelineManager::pipelines.count(pipeId)) continue;
        const auto& pipe = PipelineManager::pipelines.at(pipeId);
        for (const auto& output : pipe.outputs) {
            if (ItemManager::items.count(output.id)) {
                totalOutputValue += output.quantity * static_cast<double>(ItemManager::items.at(output.id).base_price) * efficiency;
            }
        }
    }
    return totalOutputValue;
}

// ---------------------------------------------------------------------------
// Company Net Worth
// ---------------------------------------------------------------------------

nlohmann::json calculateCompanyNetWorth(Gamestate& gs, const uuids::uuid& companyId) {
    nlohmann::json result;
    Entity* company = gs.getEntity(companyId);
    if (!company) {
        result["error"] = "company not found";
        return result;
    }

    result["name"] = company->GetName();
    result["id"] = uuids::to_string(companyId);

    auto* wallet = company->GetComponent<WalletComponent>("WalletComponent");
    double balance = wallet ? wallet->balance : 0.0;
    double debt = wallet ? wallet->debt : 0.0;

    auto* storage = company->GetComponent<InventoryComponent>("Storage");
    double inventoryValue = storage ? valueInventory(storage->GetInternalInventory()) : 0.0;

    auto* assetOwner = company->GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
    double factoryValue = 0.0;
    int factoryCount = 0;
    if (assetOwner) {
        for (const auto& assetId : assetOwner->ownedAssets) {
            Entity* asset = gs.getEntity(assetId);
            if (asset && asset->GetType() == "factory") {
                factoryValue += estimateFactoryPhysicalValue(gs, assetId);
                factoryCount++;
            }
        }
    }

    result["balance"] = balance;
    result["debt"] = debt;
    result["inventoryValue"] = inventoryValue;
    result["factoryCount"] = factoryCount;
    result["factoryValue"] = factoryValue;
    result["netWorth"] = balance + inventoryValue + factoryValue - debt;

    return result;
}

double calculateSimpleCompanyNetWorth(Gamestate& gs, const uuids::uuid& companyId) {
    auto j = calculateCompanyNetWorth(gs, companyId);
    return j.value("netWorth", 0.0);
}

// ---------------------------------------------------------------------------
// Market Activity
// ---------------------------------------------------------------------------

nlohmann::json calculateMarketActivity(Gamestate& gs, const uuids::uuid& marketId) {
    nlohmann::json result;
    Entity* market = gs.getEntity(marketId);
    if (!market) {
        result["error"] = "market not found";
        return result;
    }

    result["name"] = market->GetName();
    result["id"] = uuids::to_string(marketId);

    auto* marketComp = market->GetComponent<MarketComponent>("MarketComponent");
    auto* wallet = market->GetComponent<WalletComponent>("WalletComponent");

    int buyCount = 0, sellCount = 0;
    double buyVolume = 0.0, sellVolume = 0.0;

    if (marketComp) {
        for (const auto& [itemId, book] : marketComp->books) {
            for (const auto& order : book.buyOrders) {
                if (order.type == OrderType::BUY) {
                    buyCount++;
                    buyVolume += order.price * order.remaining();
                }
            }
            for (const auto& order : book.sellOrders) {
                if (order.type == OrderType::SELL) {
                    sellCount++;
                    sellVolume += order.price * order.remaining();
                }
            }
        }
    }

    result["buyOrderCount"] = buyCount;
    result["sellOrderCount"] = sellCount;
    result["totalBuyVolume"] = buyVolume;
    result["totalSellVolume"] = sellVolume;
    result["totalVolume"] = buyVolume + sellVolume;
    result["walletBalance"] = wallet ? wallet->balance : 0.0;

    return result;
}

double calculateSimpleMarketGDP(Gamestate& gs, const uuids::uuid& marketId) {
    auto j = calculateMarketActivity(gs, marketId);
    return j.value("totalVolume", 0.0);
}

// ---------------------------------------------------------------------------
// Trade Node Stats
// ---------------------------------------------------------------------------

nlohmann::json calculateTradeNodeStats(Gamestate& gs, const uuids::uuid& nodeId) {
    nlohmann::json result;
    Entity* node = gs.getEntity(nodeId);
    if (!node) {
        result["error"] = "trade node not found";
        return result;
    }

    result["name"] = node->GetName();
    result["id"] = uuids::to_string(nodeId);

    auto* demo = node->GetComponent<DemographicsComponent>("DemographicsComponent");
    auto* storage = node->GetComponent<InventoryComponent>("Storage");

    result["population"] = demo ? demo->population : 0;
    result["happiness"] = demo ? demo->happiness : 0.0;
    result["recruitablePop"] = demo ? demo->recruitablePop : 0;
    result["storageValue"] = storage ? valueInventory(storage->GetInternalInventory()) : 0.0;
    result["productionGDP"] = estimateProductionGDP(*node);

    return result;
}

double calculateSimpleNodeGDP(Gamestate& gs, const uuids::uuid& nodeId) {
    auto j = calculateTradeNodeStats(gs, nodeId);
    return j.value("productionGDP", 0.0);
}

// ---------------------------------------------------------------------------
// Factory Output (GDP contribution per tick)
// ---------------------------------------------------------------------------

nlohmann::json calculateFactoryOutput(Gamestate& gs, const uuids::uuid& factoryId) {
    nlohmann::json result;
    Entity* f = gs.getEntity(factoryId);
    if (!f) {
        result["error"] = "factory not found";
        return result;
    }

    result["name"] = f->GetName();
    result["id"] = uuids::to_string(factoryId);

    auto* prodComp = f->GetComponent<ProductionComponent>("ProductionComponent");
    auto* workComp = f->GetComponent<WorkforceComponent>("WorkforceComponent");
    auto* inputInv = f->GetComponent<InventoryComponent>("inputStorage");
    auto* outputInv = f->GetComponent<InventoryComponent>("outputStorage");

    result["templateId"] = prodComp ? prodComp->templateId : "";
    result["workers"] = workComp ? workComp->currentWorkers : 0;
    result["maxWorkers"] = workComp ? workComp->maxWorkers : 0;
    if (result["maxWorkers"].get<size_t>() > 0) {
        result["utilization"] = static_cast<double>(result["workers"].get<size_t>()) / result["maxWorkers"].get<size_t>();
    } else {
        result["utilization"] = 0.0;
    }
    result["inputValue"] = inputInv ? valueInventory(inputInv->GetInternalInventory()) : 0.0;
    result["outputValue"] = outputInv ? valueInventory(outputInv->GetInternalInventory()) : 0.0;
    result["gdpPerTick"] = estimateProductionGDP(*f);

    return result;
}

double calculateSimpleFactoryGDP(Gamestate& gs, const uuids::uuid& factoryId) {
    auto j = calculateFactoryOutput(gs, factoryId);
    return j.value("gdpPerTick", 0.0);
}

// ---------------------------------------------------------------------------
// Full Economy Report
// ---------------------------------------------------------------------------

nlohmann::json calculateTotalEconomyStats(Gamestate& gs) {
    nlohmann::json report;

    // Companies
    nlohmann::json companies = nlohmann::json::array();
    double totalCompanyWealth = 0.0;
    for (auto* entity : gs.getEntitiesByType("company")) {
        auto stats = calculateCompanyNetWorth(gs, entity->GetId());
        totalCompanyWealth += stats["netWorth"].get<double>();
        companies.push_back(stats);
    }
    report["companies"] = companies;
    report["totalCompanyWealth"] = totalCompanyWealth;

    // Markets
    nlohmann::json markets = nlohmann::json::array();
    double totalMarketVolume = 0.0, totalMarketWallet = 0.0;
    int totalBuyOrders = 0, totalSellOrders = 0;
    for (auto* entity : gs.getEntitiesByType("market")) {
        auto stats = calculateMarketActivity(gs, entity->GetId());
        totalMarketVolume += stats["totalVolume"].get<double>();
        totalMarketWallet += stats["walletBalance"].get<double>();
        totalBuyOrders += stats["buyOrderCount"].get<int>();
        totalSellOrders += stats["sellOrderCount"].get<int>();
        markets.push_back(stats);
    }
    report["markets"] = markets;
    report["totalMarketVolume"] = totalMarketVolume;
    report["totalMarketWallet"] = totalMarketWallet;
    report["totalBuyOrders"] = totalBuyOrders;
    report["totalSellOrders"] = totalSellOrders;

    // Trade Nodes
    nlohmann::json nodes = nlohmann::json::array();
    double totalPopulation = 0, totalNodeGDP = 0.0, totalNodeStorage = 0.0;
    for (auto* entity : gs.getEntitiesByType("trade_node")) {
        auto stats = calculateTradeNodeStats(gs, entity->GetId());
        totalPopulation += stats["population"].get<size_t>();
        totalNodeGDP += stats["productionGDP"].get<double>();
        totalNodeStorage += stats["storageValue"].get<double>();
        nodes.push_back(stats);
    }
    report["tradeNodes"] = nodes;
    report["totalPopulation"] = totalPopulation;
    report["totalNodeGDP"] = totalNodeGDP;
    report["totalNodeStorage"] = totalNodeStorage;

    // Factories
    double totalFactoryGDP = 0.0;
    int totalFactoryCount = 0;
    for (auto* entity : gs.getEntitiesByType("factory")) {
        totalFactoryGDP += estimateProductionGDP(*entity);
        totalFactoryCount++;
    }
    report["totalFactoryCount"] = totalFactoryCount;
    report["totalFactoryGDP"] = totalFactoryGDP;

    // Overall
    report["estimatedGDPperTick"] = totalNodeGDP + totalFactoryGDP;
    report["totalEntityCount"] = gs.getEntities().size();

    return report;
}

} // namespace Analytics
