// EntityMarket.h
#pragma once

#include "Registry/NameManager.h"
#include "Core/ECS/Entity.h"
#include "World/Components/MarketComponent.h"
#include "Game/IdUtils.h" // ID üretmek için şart

inline Entity* createMarket(const std::string& name = "", const std::string& templateId = "") {
    Entity* market = new Entity();
    
    market->SetId(IdUtils::generateUuid());
    
    auto n = (name != "") ? name : NameManager::getRandomName(templateId);
    market->SetName(n);
    market->SetType("market");

    market->AddComponent(new MarketComponent());
    
    return market;
}