/**
 * @file Pipeline.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Defines production recipes and data structures for the manufacturing process.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <string>
#include <vector>
#include "../Core/Types.h"
#include "../../lib/nlohmann/json.hpp"

/**
 * @struct Pipeline
 * @brief Represents a single production recipe.
 * @details Defines what goes in (Resources), what comes out (Products), 
 * and how efficient the conversion is.
 */
struct Pipeline {
  std::vector<Resource> inputResources; ///< Required raw materials.
  std::vector<Product> outputProducts;  ///< Generated finished goods.
  double efficiency;                    ///< Base multiplier for output quantity (1.0 = Standard).
};

/**
 * @namespace DefaultPipelines
 * @brief Contains static data for all hardcoded game recipes.
 */
namespace DefaultPipelines {

    /**
     * @brief Retrieves the master list of all production recipes in the game.
     * @return std::vector<Pipeline> A list of all available pipelines.
     */
    std::vector<Pipeline> getAllRecipes();

}

// ==========================================
// JSON Serialization
// ==========================================

inline void to_json(nlohmann::json &j, const Pipeline &p) {
  j = nlohmann::json{
      {"inputs", p.inputResources},
      {"outputs", p.outputProducts},
      {"efficiency", p.efficiency}
  };
}

inline void from_json(const nlohmann::json &j, Pipeline &p) {
  j.at("inputs").get_to(p.inputResources);
  j.at("outputs").get_to(p.outputProducts);
  j.at("efficiency").get_to(p.efficiency);
}

/**
 * @brief Helper to serialize the entire recipe book to JSON.
 * @return nlohmann::json Array of all pipelines.
 */
inline nlohmann::json serializeAllPipelines() {
  return DefaultPipelines::getAllRecipes();
}