/**
 * @file Factory.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Representation of a production facility in the "Producer" game.
 * @details Factories are owned by Companies, employ workers, consume resources,
 * and produce goods via assigned Pipelines.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once
#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Constants.h"
#include "../Core/Enums.h"
#include "../Core/Types.h"
#include "Pipeline.h"
#include <algorithm>
#include <string>
#include <vector>

/**
 * @class Factory
 * @brief An entity capable of transforming raw resources into products.
 */
class Factory {
public:
  /**
   * @brief Basic constructor for unowned or neutral factories.
   * @param name Display name of the factory.
   * @param id Unique identifier.
   */
  Factory(const std::string &name, uuids::uuid id)
      : name(name), id(id),
        manufacturingTech(manufacturingTechnologies::BASIC_MANUFACTURING) {}

  /**
   * @brief Constructor for player/AI owned factories.
   * @param name Display name.
   * @param id Unique identifier.
   * @param ownerId UUID of the owning Company.
   */
  Factory(const std::string &name, uuids::uuid id, uuids::uuid ownerId)
      : name(name), id(id), ownerCompanyId(ownerId),
        manufacturingTech(manufacturingTechnologies::BASIC_MANUFACTURING) {}

  friend void to_json(nlohmann::json &, const Factory &);

  /** @name Properties
   * Basic getters and setters for factory attributes.
   * @{
   */
  uuids::uuid getOwnerId() const;
  void setOwnerId(uuids::uuid newOwnerId);

  manufacturingTechnologies getManufacturingTechnology() const;
  void setManufacturingTechnology(manufacturingTechnologies tech);

  void setName(const std::string &newName);
  std::string getName() const;

  void setId(const uuids::uuid &newId);
  uuids::uuid getId() const;
  /** @} */

  /** @name Production Configuration
   * Methods to setup what the factory produces.
   * @{
   */
  void addPipeline(const Pipeline &pipeline);
  /** @} */

  /** @name Workforce Management
   * Methods to handle employees.
   * @{
   */

  /**
   * @brief Attempts to add workers to the factory.
   * @param count Desired number of workers to add.
   * @return size_t The actual number of workers added (capped by
   * MAX_EMPLOYEES).
   */
  size_t addEmployees(size_t count);

  void removeEmployees(size_t count);
  void setEmployeeCount(size_t count);
  size_t getEmployeeCount() const;
  /** @} */

  /** @name Production Cycle
   * Methods executed during the game tick.
   * @{
   */

  /** @brief Consolidates multiple stacks of the same resource type in input
   * storage. */
  void mergeInputs();

  /** * @brief Executes the production logic.
   * @details Consumes inputs and generates outputs based on efficiency and
   * workforce.
   */
  void processPipelines();

  /** @brief Consolidates multiple stacks of the same product type in output
   * storage. */
  void mergeOutputs();
  /** @} */

  /** @name Inventory Management
   * Methods to handle resource flow in and out.
   * @{
   */
  std::vector<Product> getOutputs() const;
  std::vector<Resource> getInputs() const;

  /**
   * @brief Extracts finished goods from the factory.
   * @details This method usually clears the local output buffer, transferring
   * ownership to the caller.
   * @return std::vector<Product> The collected items.
   */
  std::vector<Product> collectOutputs();

  /**
   * @brief Calculates what resources are needed for the next production cycle.
   * @return std::vector<Resource> List of required inputs.
   */
  std::vector<Resource> listRequiredResources() const;

  /** @brief Variadic template for adding multiple resources (Definition
   * required). */
  template <typename... Resources> void addToResources(Resources... resources) {
    (inputs.push_back(resources), ...);
  }

  /**
   * @brief Adds a specific single resource to inputs.
   */
  void addInputResource(const Resource &resource);

  /**
   * @brief Batch adds resources to the factory's input storage.
   * @param newResources Vector of resources to add.
   */
  inline void addToResources(std::vector<Resource> newResources) {
    for (const auto &res : newResources) {
      bool found = false;
      for (auto &storedRes : inputs) {
        if (storedRes.type == res.type) {
          storedRes.quantity += res.quantity;
          found = true;
          break;
        }
      }
      if (!found) {
        inputs.push_back(res);
      }
    }
  }

  /** @} */

private:
  std::string name;
  uuids::uuid id;
  uuids::uuid ownerCompanyId;

  size_t employeeCount = 0;
  manufacturingTechnologies manufacturingTech;

  std::vector<Pipeline>
      pipelines; ///< Production recipes assigned to this factory.
  std::vector<Resource> inputs; ///< Raw materials waiting to be processed.
  std::vector<Product> outputs; ///< Finished goods waiting to be collected.
};

void to_json(nlohmann::json &j, const Factory &f);