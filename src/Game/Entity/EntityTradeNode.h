// EntityTradeNode.h
#pragma once

#include "Core/ECS/Entity.h"
#include "Registry/TradeNodeManager.h"
#include "Registry/NameManager.h"
#include "Economy/Components/WalletComponent.h"
#include "World/Components/MarketMemberComponent.h"
#include "Economy/Components/ProductionComponent.h"
#include "Core/Components/InventoryComponent.h"
#include "World/Components/DemographicsComponent.h"
#include "World/Components/MarketMemberComponent.h"
#include "Game/IdUtils.h"


inline Entity* createTradeNode(const TradeNodeTemplate& tmpl, uuids::uuid marketId) {
    Entity* node = new Entity();
    node->SetType("trade_node");
    
    node->SetId(IdUtils::generateUuid());
    node->SetName(NameManager::getRandomName(tmpl.name_pool_id));

    node->AddComponent(new WalletComponent(tmpl.initial_capital));

    node->AddComponent(new InventoryComponent(10000.0), "Storage"); 

    size_t startPop = tmpl.min_pop + (rand() % (tmpl.max_pop - tmpl.min_pop));
    node->AddComponent(new DemographicsComponent(startPop));

    node->AddComponent(new MarketMemberComponent(marketId));

    auto* localProd = new ProductionComponent("local_production"); 
    localProd->activePipelineIds = tmpl.local_pipelines;
    node->AddComponent(localProd, "LocalProduction");

    auto* consumption = new ProductionComponent("consumption");
    consumption->activePipelineIds = tmpl.consumption_pipelines;
    node->AddComponent(consumption, "consumption");

    for (const auto& item : tmpl.start_inventory) {
        node->GetComponent<InventoryComponent>("Storage")->Add(item.id, item.quantity);
    }

    return node;
}