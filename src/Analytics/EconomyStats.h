#pragma once

#include "Game/Gamestate.h"
#include "nlohmann/json.hpp"
#include <string>

namespace Analytics {

    // === Company Net Worth ===
    nlohmann::json calculateCompanyNetWorth(Gamestate& gs, const uuids::uuid& companyId);
    double calculateSimpleCompanyNetWorth(Gamestate& gs, const uuids::uuid& companyId);

    // === Market Activity ===
    nlohmann::json calculateMarketActivity(Gamestate& gs, const uuids::uuid& marketId);
    double calculateSimpleMarketGDP(Gamestate& gs, const uuids::uuid& marketId);

    // === Trade Node Stats ===
    nlohmann::json calculateTradeNodeStats(Gamestate& gs, const uuids::uuid& nodeId);
    double calculateSimpleNodeGDP(Gamestate& gs, const uuids::uuid& nodeId);

    // === Factory GDP (production output value per tick) ===
    nlohmann::json calculateFactoryOutput(Gamestate& gs, const uuids::uuid& factoryId);
    double calculateSimpleFactoryGDP(Gamestate& gs, const uuids::uuid& factoryId);

    // === Full Economy Report ===
    nlohmann::json calculateTotalEconomyStats(Gamestate& gs);

}
