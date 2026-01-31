/**
 * @file TradeNode.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Represents an economic entity (City, Town, Outpost) within the game world.
 * @details TradeNodes are the primary agents of the economy. They hold population,
 * manage local inventories, provide labor (jobs), and interact with the Market
 * to satisfy demands or sell surplus.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Types.h"
#include "../Economy/Pipeline.h"
#include "../Game/IdUtils.h"
#include "Market.h" // Required for inline Market method calls
#include <string>
#include <utility>
#include <vector>

/**
 * @class TradeNode
 * @brief A container for population, production capacity, and inventory.
 */
class TradeNode {
public:
  /**
   * @brief Construct a new Trade Node.
   * @param name Display name (e.g., "Istanbul").
   * @param id Unique identifier.
   * @param pop Total population.
   * @param man Initial manpower (recruitable population).
   */
  TradeNode(const std::string &name, const uuids::uuid &id, size_t pop,
            size_t man)
      : name(name), id(id), population(pop), recruitablePopulation(man),
        freeJobPositions(man * 0.5) {}

  friend void to_json(nlohmann::json &, const TradeNode &);

  /** @name Identity & Demographics
   * @{
   */
  std::string getName() const;
  void setName(const std::string &newName);

  uuids::uuid getId() const;
  void setId(const uuids::uuid &newId);

  size_t getPopulation() const;
  void setPopulation(size_t pop);

  size_t getRecruitablePopulation() const;
  /** @} */

  /** @name Market Affiliation
   * @{
   */
  void setMarket(uuids::uuid newMarketId);
  uuids::uuid getMarketId() const;
  /** @} */

  /** @name Economy & Budget
   * @{
   */
  size_t getCapital() const;
  void setCapital(size_t cap);

  size_t getDebt() const;
  void setDebt(size_t d);

  void setProfit(size_t p);
  /** @} */

  /** @name Demand & Pipelines
   * Methods defining what this node wants and what it produces.
   * @{
   */
  std::vector<std::pair<Product, float>> getLikelyDemandedProducts() const;
  std::vector<std::pair<Resource, float>> getLikelyDemandedResources() const;

  std::vector<Pipeline> listProductionPipelines() const;

  void addPipeline(const Pipeline &pipeline);

  inline void setProductionPipelines(const std::vector<Pipeline> &pipelines) {
    productionPipelines = pipelines;
  }

  inline void setConsumptionPipelines(const std::vector<Pipeline> &pipelines) {
    consumptionPipelines = pipelines;
  }
  /** @} */

  /** @name Labor Market
   * @{
   */
  void setFreeJobPositions(size_t jobs);
  size_t getFreeJobPositions() const;
  void addFreeJobPositions(size_t jobs);
  /** @} */

  /** @name Market Interactions
   * Helper functions to execute trades directly with the assigned Market.
   * @{
   */

  /**
   * @brief Sells local surplus resources/products to the global Market.
   * @param market Reference to the Market instance.
   * @param resourcesToSell List of raw materials to sell.
   * @param productsToSell List of finished goods to sell.
   * @return bool True if the transaction was successful.
   */
  inline bool sellToMarket(Market &market, std::vector<Resource> &resourcesToSell,
                    std::vector<Product> &productsToSell) {

    return market.sellResourcesToMarket(resourcesToSell, ResourceInventory,
                                        capital) &&
           market.sellProductsToMarket(productsToSell, ProductInventory,
                                       capital);
  }

  /**
   * @brief Buys required resources/products from the global Market.
   * @param market Reference to the Market instance.
   * @param resourcesToBuy List of raw materials needed.
   * @param productsToBuy List of finished goods needed.
   * @return bool True if goods were found and afford.
   */
  inline bool buyFromMarket(Market &market, std::vector<Resource> &resourcesToBuy,
                     std::vector<Product> &productsToBuy) {
    return market.buyResourcesFromMarket(resourcesToBuy, ResourceInventory,
                                         capital) &&
           market.buyProductsFromMarket(productsToBuy, ProductInventory,
                                        capital);
  }
  /** @} */

  /**
   * @brief Simulation tick for population consumption.
   * @details Decreases inventory based on consumption pipelines. 
   * Representing the daily needs of the population.
   * @param market Reference for dynamic price checking (if needed for substitution).
   */
  void ConsumePopulationResources(Market &market);

private:
  std::string name;
  uuids::uuid id;
  uuids::uuid currentMarketId; ///< The Market ID this node belongs to.

  // Demographics
  size_t population = 0;
  size_t recruitablePopulation = 0;
  size_t freeJobPositions = 0;
  
  // Economy
  size_t capital = 0;
  size_t debt = 0;
  size_t profit = 0;

  // Demand Profiles (AI Hints)
  std::vector<std::pair<Product, float>> likelyDemandedProducts;
  std::vector<std::pair<Resource, float>> likelyDemandedResources;

  // Local Storage
  std::vector<Resource> ResourceInventory;
  std::vector<Product> ProductInventory;

  // Logic
  std::vector<Pipeline> productionPipelines;  ///< Pipelines for local industry.
  std::vector<Pipeline> consumptionPipelines; ///< Pipelines for population needs.
};

/**
 * @struct TradeNodeTemplate
 * @brief Blueprint for creating standard TradeNodes (e.g., "Small Village", "Metropolis").
 */
struct TradeNodeTemplate {
  std::string name;
  size_t population = 0;
  float recruitablePopulationRatio = 0.5f;
  size_t freeJobPositions = 0;

  std::vector<std::pair<Resource, float>> likelyDemandedResources;
  std::vector<std::pair<Product, float>> likelyDemandedProducts;
  std::vector<Pipeline> productionPipelines;
  std::vector<Pipeline> consumptionPipelines;

  size_t capital = 0;
  size_t debt = 0;
  size_t profit = 0;
};

/**
 * @brief Factory function to instantiate a TradeNode from a template.
 * @param tmpl The template to use.
 * @param marketId The ID of the market this node will join.
 * @return TradeNode A fully initialized object.
 */
TradeNode makeTradeNode(const TradeNodeTemplate &tmpl, uuids::uuid marketId);


// ==========================================
// JSON Serialization
// ==========================================