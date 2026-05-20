#pragma once

#include "../Core/Types.h"
#include "../Registry/PipelineManager.h"
#include "Core/ECS/Entity.h"
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

    void produce(Entity &f, double globalModifiers = 1.0f);
    void addInput(const std::string& itemId, float amount);
    std::vector<ItemStack> collectOutputs(const std::string &fId);
    std::vector<ItemStack> getMissingItems(const std::string &definitionId, const Inventory &storage);
    void executeProduction(Entity& entity, const std::string& prodKey, const std::string& invKey);
    float calculateEntityModifier(Entity& entity, const std::string& modifierType, float baseValue);

} // namespace EconomyUtils