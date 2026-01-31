/**
 * @file TradeNode.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of TradeNode logic.
 * @details Handles the daily lifecycle of a settlement: consuming resources, 
 * growing/shrinking population, and trading surpluses.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "TradeNode.h"
#include "../Economy/Pipeline.h"
#include "../../lib/uuid/uuid.h"
#include "../Core/Types.h"
#include "../Economy/Pipeline.h"
#include "../Game/IdUtils.h"
#include "Market.h"

#include <string>
#include <utility>
#include <vector>

// ==========================================
// Getters & Setters
// ==========================================

std::string TradeNode::getName() const { return name; }
uuids::uuid TradeNode::getId() const { return id; }
size_t TradeNode::getPopulation() const { return population; }
size_t TradeNode::getRecruitablePopulation() const { return recruitablePopulation; }
size_t TradeNode::getCapital() const { return capital; }
size_t TradeNode::getDebt() const { return debt; }

void TradeNode::setName(const std::string &newName) { name = newName; }
void TradeNode::setId(const uuids::uuid &newId) { id = newId; }
void TradeNode::setMarket(uuids::uuid newMarketId) { currentMarketId = newMarketId; }
uuids::uuid TradeNode::getMarketId() const { return currentMarketId; }

void TradeNode::setPopulation(size_t pop) { population = pop; }

std::vector<std::pair<Product, float>> TradeNode::getLikelyDemandedProducts() const {
  return likelyDemandedProducts;
}
std::vector<std::pair<Resource, float>> TradeNode::getLikelyDemandedResources() const {
  return likelyDemandedResources;
}

std::vector<Pipeline> TradeNode::listProductionPipelines() const {
  return productionPipelines;
}

void TradeNode::addPipeline(const Pipeline &pipeline) {
  productionPipelines.push_back(pipeline);
}

void TradeNode::setFreeJobPositions(size_t jobs) { freeJobPositions = jobs; }
size_t TradeNode::getFreeJobPositions() const { return freeJobPositions; }
void TradeNode::addFreeJobPositions(size_t jobs) { freeJobPositions += jobs; }

void TradeNode::setCapital(size_t cap) { capital = cap; }
void TradeNode::setDebt(size_t d) { debt = d; }
void TradeNode::setProfit(size_t p) { profit = p; }

// ==========================================
// Lifecycle & Consumption Logic
// ==========================================

/**
 * @brief Processes population needs and adjusts demographics.
 * @details 
 * 1. Calculates total demand based on population size.
 * 2. Consumes available items from local storage.
 * 3. Attempts to buy remaining deficit from the Market.
 * 4. Adjusts population (Growth vs Starvation) based on fulfillment.
 * 5. Automates trade by selling surplus and restocking basics.
 */
void TradeNode::ConsumePopulationResources(Market &market) {
  
  // --- Step 1: Calculate Total Demand ---
  std::vector<Resource> resourcesToConsume;
  for (const auto &demand : likelyDemandedResources) {
    size_t totalNeeded = static_cast<size_t>(demand.second * population);
    resourcesToConsume.push_back(
        Resource{demand.first.type, static_cast<float>(totalNeeded)});
  }

  std::vector<Product> productsToConsume;
  for (const auto &demand : likelyDemandedProducts) {
    size_t totalNeeded = static_cast<size_t>(demand.second * population);
    // Price set to 0 initially as it's an internal consumption check
    productsToConsume.push_back(
        Product{demand.first.type, 0.0, static_cast<float>(totalNeeded)});
  }

  // --- Step 2: Consume Local Inventory ---
  for (auto &resource : resourcesToConsume) {
    bool found = false;
    for (auto &invResource : ResourceInventory) {
      if (invResource.type == resource.type) {
        if (invResource.quantity >= resource.quantity) {
          // We have enough locally
          invResource.quantity -= resource.quantity;
          resource.quantity = 0; // Deficit is now 0
        } else {
          // We have some, but not enough
          resource.quantity -= invResource.quantity; // Remaining deficit
          invResource.quantity = 0;
        }
        found = true;
        break;
      }
    }
  }

  for (auto &product : productsToConsume) {
    bool found = false;
    for (auto &invProduct : ProductInventory) {
      if (invProduct.type == product.type) {
        if (invProduct.quantity >= product.quantity) {
          invProduct.quantity -= product.quantity;
          product.quantity = 0;
        } else {
          product.quantity -= invProduct.quantity;
          invProduct.quantity = 0;
        }
        found = true;
        break;
      }
    }
  }

  // --- Step 3: Buy Deficit from Market ---
  // Try to buy whatever we couldn't find in local inventory
  bool success = buyFromMarket(market, resourcesToConsume, productsToConsume);

  // --- Step 4: Demographic Adjustments ---
  if (success) {
    // All demands met (or market transaction succeeded efficiently)
    // Growth: 1 new person per 100 existing population
    size_t growth = (population / 100); 
    population += growth;
  } else {
    // Starvation Logic: Reduce population based on unmet needs
    for (const auto &res : resourcesToConsume) {
      if (res.quantity > 0) {
        size_t deficit = static_cast<size_t>(res.quantity);
        // Starvation Rate: 1 death per 10 missing resource units
        size_t popReduction = deficit / 10; 
        
        if (popReduction > population) {
          population = 0;
        } else {
          population -= popReduction;
        }
      }
    }
    for (const auto &prod : productsToConsume) {
      if (prod.quantity > 0) {
        size_t deficit = static_cast<size_t>(prod.quantity);
        // Starvation Rate: 1 death per 5 missing product units (Products are more critical)
        size_t popReduction = deficit / 5; 
        
        if (popReduction > population) {
          population = 0;
        } else {
          population -= popReduction;
        }
      }
    }
  }

  // --- Step 5: Automated Trading ---
  // Sell any remaining local inventory to generate capital
  sellToMarket(market, ResourceInventory, ProductInventory);
  
  // Note: The logic below seems to re-buy immediately. 
  // In a complex simulation, AI agents should decide this, but for now, 
  // it ensures the node tries to restock for the next tick.
  buyFromMarket(market, ResourceInventory, ProductInventory);
}

// ==========================================
// Factory Functions
// ==========================================

/**
 * @brief Creates a new TradeNode instance from a template.
 * @param tmpl Template defining the node's characteristics.
 * @param marketId ID of the market this node will interact with.
 * @return TradeNode Initialized node object.
 */
TradeNode makeTradeNode(const TradeNodeTemplate &tmpl, uuids::uuid marketId) {

  // 1. Generate unique ID (Requires IdUtils)
  uuids::uuid newId = IdUtils::generateUuid();

  // 2. Initialize Node
  TradeNode node(
      tmpl.name, 
      newId, 
      tmpl.population,
      static_cast<size_t>(tmpl.population * tmpl.recruitablePopulationRatio)
  );

  node.setMarket(marketId); 

  // 3. Set Economic Parameters
  node.setFreeJobPositions(tmpl.freeJobPositions);
  node.setCapital(tmpl.capital);
  node.setDebt(tmpl.debt);
  node.setProfit(tmpl.profit);

  // 4. Assign Pipelines
  node.setProductionPipelines(tmpl.productionPipelines);
  node.setConsumptionPipelines(tmpl.consumptionPipelines);

  return node;
}

void to_json(nlohmann::json& j, const TradeNode& n) {
    j = nlohmann::json{
        { "id",        uuids::to_string(n.id) },
        { "name",      n.name },
        { "marketId",  uuids::to_string(n.currentMarketId) },

        { "population",             n.population },
        { "recruitablePopulation",  n.recruitablePopulation },
        { "freeJobPositions",       n.freeJobPositions },

        { "capital", n.capital },
        { "debt",    n.debt },
        { "profit",  n.profit }
    };

    // Inventories
    j["resourceInventory"] = n.ResourceInventory;
    j["productInventory"]  = n.ProductInventory;

    // Serialize Demand Profiles
    nlohmann::json demandedResources = nlohmann::json::array();
    for (const auto& [res , ratio] : n.likelyDemandedResources) {
        demandedResources.push_back({
            { "resource", res },
            { "ratio",    ratio }
        });
    }
    j["likelyDemandedResources"] = demandedResources;

    nlohmann::json demandedProducts = nlohmann::json::array();
    for (const auto& [prod, ratio] : n.likelyDemandedProducts) {
        demandedProducts.push_back({
            { "product", prod },
            { "ratio",   ratio }
        });
    }
    j["likelyDemandedProducts"] = demandedProducts;

    // Pipelines
    j["productionPipelines"]  = n.productionPipelines;
    j["consumptionPipelines"] = n.consumptionPipelines;
}