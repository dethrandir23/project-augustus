/**
 * @file EconomyUtils.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of economic utility functions.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "EconomyUtils.h"
#include "../Core/Enums.h"
#include "../Core/Types.h"
#include "../Economy/Pipeline.h"
#include <string>
#include <vector>

EconomyUtils::ProductionResult EconomyUtils::processProduction(
    std::vector<Resource> &inventory, 
    const std::vector<Pipeline> &pipelines, 
    double globalEfficiency         
) {
  ProductionResult result;

  /** * @todo Optimization Idea: Instead of checking if (req.type == NONE), 
   * define NONE type coefficient as 0. This would replace branching (if/else) 
   * with pure mathematical operations for better performance.
   */

  for (const auto &pipe : pipelines) {

    // --- STEP 1: INPUT VALIDATION ---
    bool resourcesAvailable = true;

    // Check inputs if the pipeline has requirements
    if (!pipe.inputResources.empty()) {
      for (const auto &req : pipe.inputResources) {

        // Key Logic: specific check for NONE type. It's free/null, so skip.
        if (req.type == ItemType::NONE)
          continue;

        bool found = false;
        for (const auto &item : inventory) {
          if (item.type == req.type && item.quantity >= req.quantity) {
            found = true;
            break;
          }
        }
        if (!found) {
          resourcesAvailable = false;
          break;
        }
      }
    }

    // --- STEP 2: CONSUMPTION AND PRODUCTION ---
    if (resourcesAvailable) {
      // A) Consume Inputs (Again, skipping NONE)
      for (const auto &req : pipe.inputResources) {
        if (req.type == ItemType::NONE)
          continue;

        for (auto &item : inventory) {
          if (item.type == req.type) {
            item.quantity -= req.quantity;
            /** @note Future improvement: Implement cleanup for items reaching 0 quantity here. */
            break;
          }
        }
      }

      // B) Produce Outputs
      for (const auto &out : pipe.outputProducts) {
        float qty = out.quantity * pipe.efficiency * globalEfficiency;
        result.producedItems.push_back({out.type, out.price, qty});
      }
    }
  }

  return result;
}

void EconomyUtils::addToInventory(std::vector<Resource> &inventory, ItemType type, float qty) {
  for (auto &item : inventory) {
    if (item.type == type) {
      item.quantity += qty;
      return;
    }
  }
  // If not found, add new
  inventory.push_back({type, qty});
}

void EconomyUtils::addToInventory(std::vector<Product> &inventory, ItemType type, float qty) {
  for (auto &item : inventory) {
    if (item.type == type) {
      item.quantity += qty;
      return;
    }
  }
  // If not found, add new (price is implicitly 0 or default in this overload)
  inventory.push_back({type, 0.0, qty}); 
}

void EconomyUtils::addToInventoryAmount(std::vector<Product> &inventory, ItemType type, float qty, double price) {
  for (auto &item : inventory) {
    if (item.type == type) {
      // Update weighted average price
      item.price = ((item.price * item.quantity) + (price * qty)) /
                   (item.quantity + qty);
      // Update quantity
      item.quantity += qty;
      return;
    }
  }
  // If not found, add new entry
  inventory.push_back({type, price, qty});
}