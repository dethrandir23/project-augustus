/**
 * @file Market.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of the Market class logic.
 * @details Contains the core algorithms for price discovery, pressure decay, 
 * and demand optimization using logistic functions (S-curve).
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "Market.h"

// Core Includes
#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Constants.h"
#include "../Core/Types.h"
#include "../Economy/EconomyUtils.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

// ==========================================
// ID & Name Management
// ==========================================

void Market::setId(const uuids::uuid &newId) { id = newId; }
uuids::uuid Market::getId() const { return id; }

const std::string &Market::getName() const { return name; }
void Market::setName(const std::string &newName) { name = newName; }

// ==========================================
// Node Management
// ==========================================

void Market::addNode(const uuids::uuid &nodeId) {
  // Prevent duplicate node registration
  if (std::find(registeredNodes.begin(), registeredNodes.end(), nodeId) ==
      registeredNodes.end()) {
    registeredNodes.push_back(nodeId);
  }
}

void Market::removeNode(const uuids::uuid &nodeId) {
  registeredNodes.erase(
      std::remove(registeredNodes.begin(), registeredNodes.end(), nodeId),
      registeredNodes.end());
}

// ==========================================
// Price Logic
// ==========================================

/**
 * @brief Safely retrieves a price.
 * @return double The recorded price or MIN_PRICE if not found.
 */
double Market::getPrice(ItemType type) const {
  if (ItemPrices.count(type)) {
    return ItemPrices.at(type);
  }
  return static_cast<double>(Constants::MIN_PRICE); 
}

/**
 * @brief Calculates a smoothing coefficient based on market volatility.
 * @details High volatility (ratio far from 1.0) results in a lower smoothing factor 
 * to prevent prices from snapping too wildly.
 */
float Market::getDynamicSmoothing(float ratio) {
  float v = std::abs(ratio - 1.0f);
  return std::clamp(0.2f / (1.0f + v), 0.02f, 0.2f);
}

void Market::decayPressures(float factor) {
  for (auto &[k, v] : buyPressure)
    v *= factor;
  for (auto &[k, v] : sellPressure)
    v *= factor;
}

/**
 * @brief Core pricing algorithm.
 * @details Uses a Sigmoid (Logistic) function to determine the target price based on Supply/Demand ratio.
 * Price = Current + Smoothing * (Target - Current)
 */
void Market::calculateMarketPrices() {
  for (const auto &pair : ItemPrices) { // Iterate only over known items
    ItemType type = pair.first;
    double currentPrice = pair.second;

    float totalSupply = sellPressure[type];

    // Include physical market inventory in the Total Supply calculation
    for (const auto &r : availableResources) {
      if (r.type == type)
        totalSupply += r.quantity;
    }
    for (const auto &p : availableProducts) {
      if (p.type == type)
        totalSupply += p.quantity;
    }

    float totalDemand = buyPressure[type];

    // Calculate Supply/Demand Ratio
    float ratio =
        (totalSupply > 0.0f) ? (totalDemand / totalSupply) : totalDemand;
    
    // Apply Logarithmic scaling to dampen extreme ratios
    ratio = std::log1p(ratio);

    // Logistic Function Parameters
    float k = 1.8f;          // Steepness of the curve
    float equilibrium = 1.0f; // The ratio where price stabilizes

    // Sigmoid Function: Maps the ratio to a price between MIN and MAX
    float target = Constants::MIN_PRICE +
                   (Constants::MAX_PRICE - Constants::MIN_PRICE) /
                       (1.0f + std::exp(-k * (ratio - equilibrium)));

    // Calculate new price with smoothing (Linear Interpolation)
    double newPrice =
        currentPrice + getDynamicSmoothing(ratio) * (target - currentPrice);

    // Hard limit: Price cannot fall below MIN_PRICE
    ItemPrices[type] =
        std::max(newPrice, static_cast<double>(Constants::MIN_PRICE));
  }

  // Fade out old pressures to represent time passing
  decayPressures();
}

// ==========================================
// Transaction Logic
// ==========================================

void Market::addResource(const Resource &resource) {
  EconomyUtils::addToInventory(availableResources, resource.type,
                               resource.quantity);
}

void Market::addProduct(const Product &product) {
  EconomyUtils::addToInventoryAmount(availableProducts, product.type,
                                     product.price, product.quantity);
  
  // Ensure the product exists in the price list to prevent 0.0 price issues later
  if (ItemPrices.find(product.type) == ItemPrices.end()) {
    ItemPrices[product.type] = product.price;
  }
}

void Market::createDemandOnProduct(ItemType productType, float quantity) {
  demandProducts.push_back({productType, 0.0, quantity});
}

void Market::createDemandOnResource(ItemType resourceType, float quantity) {
  demandResources.push_back({resourceType, quantity});
}

/**
 * @brief Merges duplicate entries in the demand buffers.
 */
void Market::optimiseMarket() {
  // 1. Optimize Product Demand
  std::vector<Product> mergedDemandProducts;
  for (const auto &demand : demandProducts) {
    bool found = false;
    for (auto &merged : mergedDemandProducts) {
      if (merged.type == demand.type) {
        merged.quantity += demand.quantity;
        found = true;
        break;
      }
    }
    if (!found) {
      mergedDemandProducts.push_back(demand);
    }
  }
  demandProducts = mergedDemandProducts;

  // 2. Optimize Resource Demand
  std::vector<Resource> mergedDemandResources;
  for (const auto &demand : demandResources) {
    bool found = false;
    for (auto &merged : mergedDemandResources) {
      if (merged.type == demand.type) {
        merged.quantity += demand.quantity;
        found = true;
        break;
      }
    }
    if (!found) {
      mergedDemandResources.push_back(demand);
    }
  }
  demandResources = mergedDemandResources;
}

// ==========================================
// JSON Serialization
// ==========================================

// Note: 'inline' removed here as this is the implementation file.
void to_json(nlohmann::json& j, const Market& m) {
    j = nlohmann::json{
        { "id",   uuids::to_string(m.getId()) },
        { "name", m.getName() }
    };

    // Registered TradeNodes (UUID -> string)
    nlohmann::json nodesJson = nlohmann::json::array();
    // Assuming registeredNodes is accessible via friend declaration
    for (const auto& nid : m.registeredNodes) { // Direct access to private member
        nodesJson.push_back(uuids::to_string(nid));
    }
    j["registeredNodes"] = nodesJson;

    // ItemPrices (ItemType -> double)
    nlohmann::json pricesJson;
    for (const auto& [type, price] : m.ItemPrices) {
        pricesJson[std::to_string(static_cast<int>(type))] = price;
    }
    j["itemPrices"] = pricesJson;

    // Available inventories
    j["availableResources"] = m.availableResources;
    j["availableProducts"]  = m.availableProducts;

    // Demand
    j["demandResources"] = m.demandResources;
    j["demandProducts"]  = m.demandProducts;

    // Buy / Sell pressure
    nlohmann::json buyPressureJson;
    for (const auto& [type, value] : m.buyPressure) {
        buyPressureJson[std::to_string(static_cast<int>(type))] = value;
    }
    j["buyPressure"] = buyPressureJson;

    nlohmann::json sellPressureJson;
    for (const auto& [type, value] : m.sellPressure) {
        sellPressureJson[std::to_string(static_cast<int>(type))] = value;
    }
    j["sellPressure"] = sellPressureJson;
}