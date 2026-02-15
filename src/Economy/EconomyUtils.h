#pragma once

#include "../Core/Types.h"
#include "../Registry/PipelineManager.h"
#include "Core/Inventory.h"
#include <string>
#include <vector>

namespace EconomyUtils {

    struct ProductionResult {
        std::vector<ItemStack> producedItems;
    };

    ProductionResult processProduction(
        Inventory &inventory, 
        const std::vector<PipelineData> &pipelines, 
        double globalEfficiency = 1.0           
    );

} // namespace EconomyUtils