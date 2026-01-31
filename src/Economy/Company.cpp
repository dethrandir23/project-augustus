/**
 * @file Company.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of the Company class logic.
 * @details Handles the interactions between the company entity, its workforce, 
 * and its factories within the global Gamestate.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "Company.h"
#include "../Game/Gamestate.h" // Full definition required here
#include <vector>

// ==========================================
// Getters & Setters
// ==========================================

void Company::setName(const std::string &newName) { name = newName; }
std::string Company::getName() const { return name; }

void Company::setId(const uuids::uuid &newId) { id = newId; }
uuids::uuid Company::getId() const { return id; }

void Company::setCapital(size_t cap) { capital = cap; }
size_t Company::getCapital() const { return capital; }

void Company::setDebt(size_t d) { debt = d; }
size_t Company::getDebt() const { return debt; }

void Company::setProfit(size_t p) { profit = p; }
size_t Company::getProfit() const { return profit; }

// ==========================================
// Manpower Logic
// ==========================================

void Company::addManpower(size_t count) { manpower += count; }

void Company::removeManpower(size_t count) {
  if (manpower >= count)
    manpower -= count;
  else
    manpower = 0; // Prevent underflow
}

void Company::reduceManpower(float ratio) {
  setManpower(static_cast<size_t>(manpower * ratio));
}

void Company::setManpower(size_t count) { manpower = count; }
size_t Company::getManpower() const { return manpower; }

// ==========================================
// Factory Management
// ==========================================

void Company::addFactory(const uuids::uuid &factoryId) {
  factories.push_back(factoryId);
}

std::vector<uuids::uuid> Company::getFactoryIds() const { return factories; }

/**
 * @brief Removes a factory reference from the company's list.
 * @details This uses the erase-remove idiom. It does NOT delete the factory 
 * object itself from memory or the Gamestate, only the company's ownership record.
 */
void Company::removeFactoryRef(const uuids::uuid &factoryID) {
  factories.erase(std::remove(factories.begin(), factories.end(), factoryID),
                  factories.end());
}

// ==========================================
// Gameplay Loops
// ==========================================

/**
 * @brief Distributes available manpower to owned factories.
 * @details Algorithm:
 * 1. Cleans up invalid factory IDs.
 * 2. Calculates total manpower deficit across all factories.
 * 3. Distributes available company manpower evenly among factories with needs.
 * @param gamestate Reference to the global state to access Factory objects.
 */
void Company::fillEmployeesOfFactories(Gamestate &gamestate) {
  size_t requestedEmployees = 0;
  size_t unfilledFactories = 0;

  // Step 1: Sanitation
  cleanDeadFactories(gamestate);

  // Step 2: Needs Assessment
  for (const auto &fid : factories) {
    Factory *f = gamestate.getFactory(fid);
    
    // Safety check (redundant after cleanDeadFactories but good practice)
    if (!f) continue;

    if (f->getEmployeeCount() < Constants::MAX_EMPLOYEES_PER_FACTORY) {
      unfilledFactories++;
      requestedEmployees +=
          Constants::MAX_EMPLOYEES_PER_FACTORY - f->getEmployeeCount();
    }
  }

  if (requestedEmployees == 0 || unfilledFactories == 0)
    return;

  // Calculate fair share per factory
  size_t manpowerPerFactory = manpower / unfilledFactories;

  // Step 3: Assignment
  for (const auto &fid : factories) {
    Factory *f = gamestate.getFactory(fid);
    if (!f) continue;

    if (f->getEmployeeCount() < Constants::MAX_EMPLOYEES_PER_FACTORY) {
      size_t added = f->addEmployees(manpowerPerFactory);
      removeManpower(added); // Deduct from company pool
    }
  }
}

/**
 * @brief Supplies factories with required raw materials from company storage.
 * @details Transfers ownership of resources from Company -> Factory based on
 * factory requirements.
 */
void Company::fillResourcesOfFactories(Gamestate &gamestate) {
  cleanDeadFactories(gamestate);

  for (const auto &fid : factories) {
    Factory *factory = gamestate.getFactory(fid);
    if (!factory) continue;

    auto requiredResources = factory->listRequiredResources();
    for (const auto &res : requiredResources) {
      // Find matching resource in company storage
      for (auto &companyRes : resources) {
        if (companyRes.type == res.type) {
          // Transfer only what is needed or what is available
          float quantityToTransfer = std::min(companyRes.quantity, res.quantity);

          factory->addToResources({{res.type, quantityToTransfer}});
          companyRes.quantity -= quantityToTransfer;
          
          break; // Move to next required resource
        }
      }
    }
  }
}

/**
 * @brief Triggers the production tick for all owned factories.
 */
void Company::processFactories(Gamestate &gamestate) {
  cleanDeadFactories(gamestate);

  for (const auto &fid : factories) {
    Factory *factory = gamestate.getFactory(fid);
    if (factory) {
      factory->processPipelines();
    }
  }
}

/**
 * @brief Collects finished goods from factories.
 * @details Implements a Weighted Average Price algorithm to merge new products 
 * with existing inventory.
 * Formula: ((OldPrice * OldQty) + (NewPrice * NewQty)) / TotalQty
 */
void Company::collectOutputs(Gamestate &gamestate) {
  cleanDeadFactories(gamestate);

  for (const auto &fid : factories) {
    Factory *factory = gamestate.getFactory(fid);
    if (!factory) continue;

    auto factoryOutputs = factory->collectOutputs();
    for (const auto &product : factoryOutputs) {
      bool found = false;
      for (auto &invProduct : inventory) {
        if (invProduct.type == product.type) {
          // Weighted Average Price Calculation
          invProduct.price = ((invProduct.price * invProduct.quantity) +
                              (product.price * product.quantity)) /
                             (invProduct.quantity + product.quantity);
          invProduct.quantity += product.quantity;
          found = true;
          break;
        }
      }
      // If product type not in inventory, add as new
      if (!found) {
        inventory.push_back(product);
      }
    }
  }
}

// ==========================================
// Resources & Inventory
// ==========================================

void Company::addResource(const Resource &resource) {
  for (auto &res : resources) {
    if (res.type == resource.type) {
      res.quantity += resource.quantity;
      return;
    }
  }
  resources.push_back(resource);
}

const std::vector<Resource> &Company::getResources() const { return resources; }
const std::vector<Product> &Company::getInventory() const { return inventory; }

/**
 * @brief Calculates net worth based on liquid assets (capital/profit/debt).
 * @note Currently excludes the valuation of inventory and factory assets.
 */
int Company::calculateNetWorth() const {
  return static_cast<int>(capital + profit - debt);
}

/**
 * @brief Validates the factory list against the Gamestate.
 * @details Removes IDs of factories that return nullptr from Gamestate (destroyed or invalid).
 * Uses std::remove_if to ensure safe deletion while iterating.
 */
void Company::cleanDeadFactories(Gamestate &gamestate) {
  factories.erase(std::remove_if(factories.begin(), factories.end(),
                                 [&gamestate](const uuids::uuid &id) {
                                   return gamestate.getFactory(id) == nullptr;
                                 }),
                  factories.end());
}

// ==========================================
// JSON Serialization
// ==========================================

void to_json(nlohmann::json &j, const Company &c) {
  j = nlohmann::json{
      {"id", uuids::to_string(c.getId())},
      {"name", c.getName()},
      {"manpower", c.getManpower()},
      {"capital", c.getCapital()},
      {"debt", c.getDebt()},
      {"profit", c.getProfit()},
  };

  // Serialize Factory IDs
  nlohmann::json factoriesJson = nlohmann::json::array();
  for (const auto &fid : c.getFactoryIds()) {
    factoriesJson.push_back(uuids::to_string(fid));
  }
  j["factories"] = factoriesJson;

  // Serialize Resources and Inventory
  j["resources"] = c.getResources();
  j["inventory"] = c.getInventory();
}