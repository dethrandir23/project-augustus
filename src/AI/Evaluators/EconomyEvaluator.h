#pragma once
#include "Core/ECS/Entity.h"
#include "Game/Gamestate.h"
#include <string>
#include <unordered_map>

namespace EconomyEvaluator {
    
    float scoreFactoryProfitability(const std::string& templateId, Gamestate& gamestate,
                                    const std::unordered_map<std::string, float>* categoryPrefs = nullptr);

    float scoreInvestmentDesire(Entity& aiEntity, float investThreshold, float investDivisor);

}