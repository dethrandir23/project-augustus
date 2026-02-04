#pragma once

#include "../Core/Types.h"
#include "../Registry/PipelineManager.h"
#include <string>
#include <vector>

namespace EconomyUtils {

    struct ProductionResult {
        std::vector<ItemStack> producedItems;
    };

    ProductionResult processProduction(
        std::vector<ItemStack> &inventory, 
        const std::vector<PipelineData> &pipelines, 
        double globalEfficiency = 1.0           
    );

    void addToInventory(std::vector<ItemStack> &inventory, const std::string& itemId, float qty);

    bool removeFromInventory(std::vector<ItemStack> &inventory, const std::string& itemId, float qty);

    float getItemAmount(const std::vector<ItemStack> &inventory, const std::string& itemId);

} // namespace EconomyUtils