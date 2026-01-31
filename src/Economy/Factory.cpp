/**
 * @file Factory.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of the Factory class logic.
 * @details Handles the complex logic of production scaling, resource consumption, 
 * and efficiency calculations based on workforce and technology.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "Factory.h"
#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Constants.h"
#include "../Core/Enums.h"
#include "../Core/Types.h"
#include "Pipeline.h"
#include <string>
#include <vector>

// ==========================================
// Getters & Setters
// ==========================================

uuids::uuid Factory::getOwnerId() const { return ownerCompanyId; }
void Factory::setOwnerId(uuids::uuid newOwnerId) {
  ownerCompanyId = newOwnerId;
}

manufacturingTechnologies Factory::getManufacturingTechnology() const {
  return manufacturingTech;
}

void Factory::setManufacturingTechnology(manufacturingTechnologies tech) {
  manufacturingTech = tech;
}

void Factory::setName(const std::string &newName) { name = newName; }
std::string Factory::getName() const { return name; }

void Factory::setId(const uuids::uuid &newId) { id = newId; }
uuids::uuid Factory::getId() const { return id; }

// ==========================================
// Production Configuration
// ==========================================

void Factory::addPipeline(const Pipeline &pipeline) {
  pipelines.push_back(pipeline);
}

void Factory::addInputResource(const Resource &resource) {
  inputs.push_back(resource);
}

// ==========================================
// Workforce Logic
// ==========================================

/**
 * @brief Adds employees up to the maximum factory capacity.
 * @return size_t The actual number of employees added.
 */
size_t Factory::addEmployees(size_t count) {
  size_t spaceAvailable = Constants::MAX_EMPLOYEES_PER_FACTORY - employeeCount;
  
  // Use the smaller of the two values (Requested vs Available)
  size_t toAdd = (count < spaceAvailable) ? count : spaceAvailable;

  employeeCount += toAdd;
  return toAdd; 
}

void Factory::removeEmployees(size_t count) {
  if (employeeCount >= count) { 
    employeeCount -= count;
  } else {
    employeeCount = 0; // Prevent underflow
  }
}

void Factory::setEmployeeCount(size_t count) {
  if (count <= Constants::MAX_EMPLOYEES_PER_FACTORY && count >= 0) {
    employeeCount = count;
  }
}

size_t Factory::getEmployeeCount() const { return employeeCount; }

// ==========================================
// Production Cycle Logic
// ==========================================

void Factory::mergeInputs() {
  std::vector<Resource> mergedInputs;
  for (const auto &input : inputs) {
    bool found = false;
    for (auto &merged : mergedInputs) {
      if (merged.type == input.type) {
        merged.quantity += input.quantity;
        found = true;
        break;
      }
    }
    if (!found) {
      mergedInputs.push_back(input);
    }
  }
  inputs = mergedInputs;
}

/**
 * @brief Executes the main production logic.
 * @details Algorithm:
 * 1. **Worker Ratio:** Calculates theoretical max production based on employee count (0.0 to 1.0).
 * 2. **Resource Limiter:** For each pipeline, identifies the "limiting reagent" (scarcest input resource).
 * 3. **Bottleneck Determination:** The final production ratio is the minimum of Worker Ratio and Resource Ratio.
 * 4. **Execution:** Consumes inputs and generates outputs scaled by the final ratio.
 */
void Factory::processPipelines() {
  mergeInputs();

  // 1. Calculate Production Capacity based on Workforce
  float workerRatio =
      static_cast<float>(employeeCount) / Constants::MAX_EMPLOYEES_PER_FACTORY;

  for (const auto &pipeline : pipelines) {

    // 2. Calculate Production Capacity based on Raw Materials
    float resourceLimitRatio = 1.0f; // Start at 100% assumption

    for (const auto &requiredRes : pipeline.inputResources) {
      float foundQuantity = 0.0f;
      
      // Check inventory for this specific requirement
      for (const auto &storedRes : inputs) {
        if (storedRes.type == requiredRes.type) {
          foundQuantity = storedRes.quantity;
          break;
        }
      }

      // Calculate availability ratio for this specific resource
      // If we have less than required, this becomes the new limiter.
      float currentResourceRatio = (requiredRes.quantity > 0)
                                       ? (foundQuantity / requiredRes.quantity)
                                       : 0;

      if (currentResourceRatio < resourceLimitRatio) {
        resourceLimitRatio = currentResourceRatio;
      }
    }

    // 3. Determine the Bottleneck
    // Production is limited by whichever is lower: Workforce or Resources.
    float finalProductionRatio = std::min(workerRatio, resourceLimitRatio);

    // 4. Execute Production (if ratio is significant enough to matter)
    if (finalProductionRatio > 0.001f) {

      // A) Consume Inputs scaled by the final ratio
      for (const auto &requiredRes : pipeline.inputResources) {
        float amountToConsume = requiredRes.quantity * finalProductionRatio;
        for (auto &storedRes : inputs) {
          if (storedRes.type == requiredRes.type) {
            storedRes.quantity -= amountToConsume;
            break;
          }
        }
      }

      // B) Generate Outputs
      // Formula: BaseQty * PipelineEfficiency * TechLevel * BottleneckRatio
      float productionFactor = pipeline.efficiency *
                               static_cast<float>(manufacturingTech) *
                               finalProductionRatio;

      for (const auto &outputProd : pipeline.outputProducts) {
        float producedQty = outputProd.quantity * productionFactor;
        outputs.push_back({outputProd.type, outputProd.price, producedQty});
      }
    }
  }

  // Cleanup: Remove depleted resources (close to zero)
  inputs.erase(
      std::remove_if(inputs.begin(), inputs.end(),
                     [](const Resource &r) { return r.quantity <= 0.001f; }),
      inputs.end());

  mergeOutputs();
}

void Factory::mergeOutputs() {
  std::vector<Product> mergedOutputs;
  for (const auto &output : outputs) {
    bool found = false;
    for (auto &merged : mergedOutputs) {
      if (merged.type == output.type) {
        merged.quantity += output.quantity;
        
        // Update price using Weighted Average Price formula
        merged.price = ((merged.price * merged.quantity) +
                        (output.price * output.quantity)) /
                       (merged.quantity + output.quantity);
        found = true;
        break;
      }
    }
    if (!found) {
      mergedOutputs.push_back(output);
    }
  }
  outputs = mergedOutputs;
}

// ==========================================
// Inventory Helpers
// ==========================================

std::vector<Product> Factory::getOutputs() const { return outputs; }
std::vector<Resource> Factory::getInputs() const { return inputs; }

std::vector<Product> Factory::collectOutputs() {
  std::vector<Product> collectedOutputs = outputs;
  outputs.clear(); // Clear local buffer after collection
  return collectedOutputs;
}

std::vector<Resource> Factory::listRequiredResources() const {
  std::vector<Resource> requiredResources;

  for (const auto &pipeline : pipelines) {
    for (const auto &inputRes : pipeline.inputResources) {

      bool found = false;
      for (auto &res : requiredResources) {
        if (res.type == inputRes.type) {
          res.quantity += inputRes.quantity;
          found = true;
          break;
        }
      }
      if (!found) {
        requiredResources.push_back(inputRes);
      }
    }
  }
  return requiredResources;
}

// ==========================================
// JSON Serialization
// ==========================================

void to_json(nlohmann::json &j, const Factory &f) {
  j = nlohmann::json{
      {"id", uuids::to_string(f.getId())},
      {"name", f.getName()},
      {"ownerCompanyId", uuids::to_string(f.getOwnerId())},
      {"employeeCount", f.getEmployeeCount()},
      {"manufacturingTech", static_cast<int>(f.getManufacturingTechnology())}};

  // Serialize Pipelines
  j["pipelines"] = f.pipelines; // Requires to_json for Pipeline

  // Serialize Storage
  j["inputs"] = f.getInputs();
  j["outputs"] = f.getOutputs();
}