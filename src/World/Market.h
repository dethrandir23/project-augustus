/**
 * @file Market.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Dynamic marketplace logic for the "Producer" game economy.
 * @details Handles price discovery, supply/demand tracking (pressure), and 
 * transactional exchanges between entities and the global/local market.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

// Core Includes
#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Constants.h"
#include "../Core/Types.h"
#include "../Economy/EconomyUtils.h"

/**
 * @class Market
 * @brief Represents a trade hub where resources and products are exchanged.
 */
class Market {
public:
  /**
   * @brief Constructor for a market with a name only.
   * @note ID must be set later or generated automatically if not provided.
   * @param name Display name of the market (e.g., "London Market").
   */
  Market(const std::string &name) : name(name) {}

  /**
   * @brief Full constructor.
   * @param id Unique identifier.
   * @param name Display name.
   */
  Market(const uuids::uuid &id, const std::string &name) : id(id), name(name) {}

  friend void to_json(nlohmann::json &, const Market &);

  /** @name Identity Management
   * @{
   */
  void setId(const uuids::uuid &newId);
  uuids::uuid getId() const;

  const std::string &getName() const;
  void setName(const std::string &newName);
  /** @} */

  /** @name Node Management
   * @brief Managing connected entities (Cities, Factories) to this market.
   * @{
   */
  void addNode(const uuids::uuid &nodeId);
  void removeNode(const uuids::uuid &nodeId);
  /** @} */

  /** @name Price Logic
   * @brief Algorithms for dynamic pricing based on Supply/Demand.
   * @{
   */

  /**
   * @brief Retrieves the current trading price of an item.
   * @details Includes safety checks to prevent returning 0.0 or invalid prices.
   * @param type The item type to query.
   * @return double The current market price.
   */
  double getPrice(ItemType type) const;

  /**
   * @brief Calculates a smoothing factor for price volatility.
   * @param ratio Supply/Demand ratio.
   * @return float Smoothing coefficient.
   */
  float getDynamicSmoothing(float ratio);

  /**
   * @brief Reduces the buy/sell pressure over time.
   * @details Simulates "market memory" fading. Old trades affect current price less than recent ones.
   * @param factor Decay rate (default 0.85).
   */
  void decayPressures(float factor = 0.85f);

  /**
   * @brief Updates all item prices based on current inventory levels and pressures.
   */
  void calculateMarketPrices();
  /** @} */

  /** @name Transaction Logic
   * @brief Inline functions handling the exchange of goods and money.
   * @{
   */

  /**
   * @brief Executes a purchase of Raw Resources by an entity.
   * @param resourcesToBuy List of desired resources.
   * @param otherInventory Reference to Buyer's inventory.
   * @param otherBudget Reference to Buyer's cash reserves.
   * @return bool True if processed (logic handles insufficient funds internally).
   */
  inline bool
  buyResourcesFromMarket(std::vector<Resource> &resourcesToBuy,
                         std::vector<Resource> &otherInventory,
                         size_t &otherBudget) { 

    for (auto &resource : resourcesToBuy) {
      for (auto &marketResource : availableResources) {
        if (marketResource.type == resource.type) {
          // Check availability
          if (marketResource.quantity >= resource.quantity) {

            double pricePerUnit = getPrice(resource.type); // Safe price lookup
            double totalPrice = pricePerUnit * resource.quantity;

            // Check affordability
            if (otherBudget >= totalPrice) {
              // 1. Remove from Market
              marketResource.quantity -= resource.quantity;
              
              // 2. Increase Buy Pressure (Demand)
              buyPressure[resource.type] += resource.quantity;

              // 3. Add to Buyer
              EconomyUtils::addToInventory(otherInventory, resource.type,
                                           resource.quantity);
              
              // 4. Handle Payment
              otherBudget -= static_cast<size_t>(totalPrice);
            }
          }
          break; // Resource found, move to next item in buy list
        }
      }
    }
    return true;
  }

  /**
   * @brief Executes a purchase of Finished Products by an entity.
   * @param productsToBuy List of desired products.
   * @param otherInventory Reference to Buyer's inventory.
   * @param otherBudget Reference to Buyer's cash reserves.
   * @return bool True if processed.
   */
  inline bool buyProductsFromMarket(std::vector<Product> &productsToBuy,
                                    std::vector<Product> &otherInventory,
                                    size_t &otherBudget) {

    for (auto &product : productsToBuy) {
      for (auto &marketProduct : availableProducts) {
        if (marketProduct.type == product.type) {
          if (marketProduct.quantity >= product.quantity) {

            double pricePerUnit = getPrice(product.type);
            double totalPrice = pricePerUnit * product.quantity;

            if (otherBudget >= totalPrice) {
              marketProduct.quantity -= product.quantity;
              buyPressure[product.type] += product.quantity;

              // Add to buyer (using average price calculation)
              EconomyUtils::addToInventoryAmount(
                  otherInventory, product.type, pricePerUnit, product.quantity);

              otherBudget -= static_cast<size_t>(totalPrice);
            }
          }
          break;
        }
      }
    }
    return true;
  }

  /**
   * @brief Executes a sale of Finished Products TO the market.
   * @details Seller gains money, Market gains stock.
   */
  inline bool sellProductsToMarket(std::vector<Product> &productsToSell,
                                   std::vector<Product> &otherInventory,
                                   size_t &otherBudget) {

    for (auto &product : productsToSell) {
      for (auto &invProduct : otherInventory) {
        if (invProduct.type == product.type) {
          if (invProduct.quantity >= product.quantity) {

            double pricePerUnit = getPrice(product.type);
            double totalPrice = pricePerUnit * product.quantity;

            // Remove from Seller
            invProduct.quantity -= product.quantity;
            
            // Increase Sell Pressure (Supply surplus)
            sellPressure[product.type] += product.quantity;

            // Add to Market Inventory
            EconomyUtils::addToInventoryAmount(availableProducts, product.type,
                                               pricePerUnit, product.quantity);

            // Register price if item is new to the market
            if (ItemPrices.find(product.type) == ItemPrices.end()) {
              ItemPrices[product.type] = pricePerUnit; 
            }

            // Pay the Seller
            otherBudget += static_cast<size_t>(totalPrice);
          }
          break;
        }
      }
    }
    return true;
  }

  /**
   * @brief Executes a sale of Raw Resources TO the market.
   */
  inline bool sellResourcesToMarket(std::vector<Resource> &resourcesToSell,
                                    std::vector<Resource> &otherInventory,
                                    size_t &otherBudget) {

    for (auto &resource : resourcesToSell) {
      for (auto &invResource : otherInventory) {
        if (invResource.type == resource.type) {
          if (invResource.quantity >= resource.quantity) {

            double pricePerUnit = getPrice(resource.type);
            double totalPrice = pricePerUnit * resource.quantity;

            invResource.quantity -= resource.quantity;
            sellPressure[resource.type] += resource.quantity;

            EconomyUtils::addToInventory(availableResources, resource.type,
                                         resource.quantity);

            if (ItemPrices.find(resource.type) == ItemPrices.end()) {
              ItemPrices[resource.type] = pricePerUnit;
            }

            otherBudget += static_cast<size_t>(totalPrice);
          }
          break;
        }
      }
    }
    return true;
  }
  /** @} */

  /** @name Utilities
   * @brief Helpers for simulation and initialization.
   * @{
   */
  inline void
  listAvailableResources(std::vector<Resource> &outResources) const {
    outResources = availableResources;
  }

  inline void listAvailableProducts(std::vector<Product> &outProducts) const {
    outProducts = availableProducts;
  }

  /** @brief Adds initial stock (e.g., from Map Editor). */
  void addResource(const Resource &resource);
  void addProduct(const Product &product);

  /** @brief Simulates artificial demand (e.g., population consumption). */
  void createDemandOnProduct(ItemType productType, float quantity);
  void createDemandOnResource(ItemType resourceType, float quantity);

  /** @brief Merges duplicate stacks of resources/products in market inventory. */
  void optimiseMarket();
  /** @} */

private:
  std::string name;
  uuids::uuid id;
  std::vector<uuids::uuid> registeredNodes; ///< IDs of Cities/Factories connected here.

  std::unordered_map<ItemType, double> ItemPrices; ///< Cache of current prices.

  std::vector<Resource> availableResources;
  std::vector<Product> availableProducts;

  std::vector<Product> demandProducts;   ///< Untracked demand buffer (future use).
  std::vector<Resource> demandResources; ///< Untracked demand buffer (future use).

  std::unordered_map<ItemType, float> buyPressure;  ///< Tracks recent demand volume.
  std::unordered_map<ItemType, float> sellPressure; ///< Tracks recent supply volume.
};

void to_json(nlohmann::json &j, const Market &m);