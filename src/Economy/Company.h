/**
 * @file Company.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Management logic for business entities in the "Producer" simulation.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "../../lib/nlohmann/json.hpp"
#include <algorithm>
#include <iostream> 
#include <string>
#include <vector>

#include "../../lib/uuid/uuid.h"
#include "../Core/Constants.h"
#include "Factory.h"

/**
 * @brief Forward declaration of the Gamestate class.
 * Crucial for avoiding circular dependencies while allowing the Company to interact with global game data.
 */
class Gamestate;

/**
 * @class Company
 * @brief Represents a player or AI-controlled business entity.
 * * Manages capital, manpower, resources, and a collection of factories.
 * The Company class handles the high-level orchestration of production cycles.
 */
class Company {
public:
  Company() = default;

  /**
   * @brief Explicit constructor to prevent implicit string conversions.
   * @param id Unique UUID for the company.
   */
  explicit Company(const uuids::uuid &id) : id(id) {}

  /** @param name Display name of the company. */
  Company(const std::string &name) : name(name) {}

  /**
   * @param id Unique UUID for the company.
   * @param name Display name of the company.
   */
  Company(const uuids::uuid &id, const std::string &name)
      : id(id), name(name) {}

  friend void to_json(nlohmann::json &j, const Company &c);

  /** @name Getters & Setters
   * Basic property management.
   * @{
   */
  void setName(const std::string &newName);
  std::string getName() const;

  void setId(const uuids::uuid &newId);
  uuids::uuid getId() const;

  void setCapital(size_t cap);
  size_t getCapital() const;

  void setDebt(size_t d);
  size_t getDebt() const;
  void setProfit(size_t p);
  size_t getProfit() const;
  /** @} */

  /** @name Manpower Logic
   * Methods for handling the workforce.
   * @{
   */
  void addManpower(size_t count);
  void removeManpower(size_t count);
  void reduceManpower(float ratio);
  void setManpower(size_t count);
  size_t getManpower() const;
  /** @} */

  /** @name Factory Management
   * Methods for managing factory ownership.
   * @{
   */
  
  /** @brief Adds a factory ID to the company's registry. */
  void addFactory(const uuids::uuid &factoryId);

  /** @brief Returns a list of all factory UUIDs owned by this company. */
  std::vector<uuids::uuid> getFactoryIds() const;

  /** * @brief Removes the factory reference from the company list.
   * @note Does NOT delete the factory from the global Gamestate.
   */
  void removeFactoryRef(const uuids::uuid &factoryID);
  /** @} */

  /** @name Gameplay Loops
   * Critical logic for the simulation cycles.
   * @{
   */

  /** @brief Distributes company manpower among owned factories. */
  void fillEmployeesOfFactories(Gamestate &gamestate);

  /** @brief Transfers required raw materials from company storage to factories. */
  void fillResourcesOfFactories(Gamestate &gamestate);

  /** @brief Triggers the production process in all owned factories. */
  void processFactories(Gamestate &gamestate);

  /** @brief Collects finished products from factories into the company inventory. */
  void collectOutputs(Gamestate &gamestate);

  /** * @brief Cleans up ID references to factories that no longer exist in the Gamestate.
   * Uses safe iteration to avoid invalidating references.
   */
  void cleanDeadFactories(Gamestate &gamestate);
  /** @} */

  /** @name Resources & Inventory
   * Methods for managing physical goods.
   * @{
   */
  void addResource(const Resource &resource);

  /** @brief Helper to add multiple resources at once. */
  inline void addResource(const std::vector<Resource> &newResources) {
    for (const auto &res : newResources)
      addResource(res);
  }

  const std::vector<Resource> &getResources() const;
  const std::vector<Product> &getInventory() const;

  /** @brief Calculates total net worth including capital and value of assets. */
  int calculateNetWorth() const;
  /** @} */

private:
  uuids::uuid id;
  std::string name;
  size_t manpower = 0;
  size_t capital = 0;
  size_t debt = 0;
  size_t profit = 0;

  std::vector<uuids::uuid> factories; ///< List of UUIDs representing owned factories.
  std::vector<Resource> resources;    ///< Raw materials storage.
  std::vector<Product> inventory;     ///< Finished goods storage.
};

void to_json(nlohmann::json &j, const Company &c);