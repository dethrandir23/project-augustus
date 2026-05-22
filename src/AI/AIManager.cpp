#include "AIManager.h"
#include "AI/AIBrain.h"
#include "AI/CompanyBrain.h"
#include "AI/TradeNodeBrain.h"
#include "AI/Components/AIControllerComponent.h"
#include "Core/ECS/Entity.h"
#include "Game/Gamestate.h"
#include <unordered_map>

static std::unordered_map<std::string, AIManager::BrainFactory> brainFactories;

void AIManager::init() {
    registerBrainType("company", []() -> std::unique_ptr<AIBrain> {
        return std::make_unique<CompanyBrain>();
    });
    registerBrainType("trade_node", []() -> std::unique_ptr<AIBrain> {
        return std::make_unique<TradeNodeBrain>();
    });
}

void AIManager::registerBrainType(const std::string& entityType, BrainFactory factory) {
    brainFactories[entityType] = factory;
}

std::unique_ptr<AIBrain> AIManager::createBrain(const std::string& entityType) {
    auto it = brainFactories.find(entityType);
    if (it != brainFactories.end()) {
        return it->second();
    }
    return nullptr;
}

void AIManager::processAll(Gamestate& gamestate) {
    for (auto& [id, entity] : gamestate.getEntities()) {
        auto* aiComp = entity->GetComponent<AIControllerComponent>("AIControllerComponent");
        if (!aiComp || !aiComp->hasBrain()) continue;
        aiComp->getBrain()->execute(*entity, gamestate);
    }
}
