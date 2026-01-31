/**
 * @file EconomyUtils.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Stateless utility functions for economic calculations and inventory management.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "../Core/Enums.h"
#include "../Core/Types.h"
#include "../Economy/Pipeline.h"
#include <string>
#include <vector>

/**
 * @namespace EconomyUtils
 * @brief Contains standalone algorithms for processing production chains and managing resource lists.
 */
namespace EconomyUtils {

/**
 * @brief Container for the output of a production cycle.
 */
struct ProductionResult {
  std::vector<Product> producedItems;
};

/**
 * @brief Executes the production logic for a set of pipelines against an inventory.
 * @details This function is stateless. It modifies the provided inventory directly
 * by consuming inputs and returns the produced items separately.
 * * @param inventory Reference to the resource list. Inputs will be subtracted from here.
 * @param pipelines Vector of production recipes (Pipelines) to attempt.
 * @param globalEfficiency Multiplier for output quantity (Technological or moral modifiers).
 * @return ProductionResult Object containing the newly created products.
 */
ProductionResult processProduction(
    std::vector<Resource> &inventory, 
    const std::vector<Pipeline> &pipelines, 
    double globalEfficiency = 1.0           
);

/**
 * @brief Adds a raw resource to an inventory list.
 * @details If the item exists, increases quantity. If not, adds a new entry.
 */
void addToInventory(std::vector<Resource> &inventory, ItemType type, float qty);

/**
 * @brief Adds a product to an inventory list WITHOUT updating price.
 * @details Useful for simple quantity adjustments where value calculation isn't needed.
 */
void addToInventory(std::vector<Product> &inventory, ItemType type, float qty);

/**
 * @brief Adds a product to an inventory list and recalculates the weighted average price.
 * @details Formula: NewPrice = ((OldQty * OldPrice) + (AddQty * AddPrice)) / TotalQty
 */
void addToInventoryAmount(std::vector<Product> &inventory, ItemType type, float qty, double price);

} // namespace EconomyUtils