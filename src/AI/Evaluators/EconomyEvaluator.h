#pragma once
#include "Core/ECS/Entity.h"
#include "Game/Gamestate.h"
#include <string>

namespace EconomyEvaluator {
    
    float scoreFactoryProfitability(const std::string& templateId, Gamestate& gamestate);

    float scoreInvestmentDesire(Entity& aiEntity);

}