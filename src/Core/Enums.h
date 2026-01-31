/**
 * @file Enums.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Defines technology tiers and item types for the game "Producer".
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once
#include "../../lib/nlohmann/json.hpp"

/** @brief Tiers for mining technology. */
enum class miningTechnologies { BASIC_MINING = 1, ADVANCED_MINING = 2 };

/** @brief Tiers for agriculture technology. */
enum class agricultureTechnologies {
    BASIC_AGRICULTURE = 1,
    ADVANCED_AGRICULTURE = 2
};

/** @brief Tiers for manufacturing technology. */
enum class manufacturingTechnologies {
    BASIC_MANUFACTURING = 1,
    ADVANCED_MANUFACTURING = 2
};

/** @brief Tiers for construction technology. */
enum class constructionTechnologies {
    BASIC_CONSTRUCTION = 1,
    ADVANCED_CONSTRUCTION = 2
};

/**
 * @brief Categorized list of all resources and goods in the game economy.
 */
enum class ItemType {
    NONE,

    // --- Basic Construction Materials ---
    WOOD,       // Logs
    STONE,      // Raw stone
    CLAY,       // Used for brick making
    SAND,       // Used for glass making

    // --- Agriculture and Food ---
    GRAIN,      // Wheat, corn, etc.
    MEAT,       // Livestock products
    FISH,       // From water bodies
    FRUIT,      // Orchard products
    
    // --- Textile Raw Materials ---
    WOOL,       // Sheep wool
    COTTON,     // Plant fiber
    SILK,       // Luxury fiber
    DYE,        // Plant-based pigments
    HIDES,      // Raw leather

    // --- Ores and Minerals (Mining) ---
    IRON_ORE,   // Raw iron
    COAL,       // Industrial fuel
    COPPER_ORE, // Raw copper
    GOLD_ORE,   // Precious metal
    SULPHUR,    // Gunpowder ingredient
    SALTPETER,  // Gunpowder ingredient

    // --- Intermediate Goods ---
    LUMBER,         // Processed wood (Wood -> Lumber)
    BRICK,          // Processed clay (Clay -> Brick)
    GLASS,          // Processed sand (Sand -> Glass)
    STEEL,          // Refined metal (Iron + Coal -> Steel)
    CLOTH,          // Fabric (Wool/Cotton -> Cloth)
    PAPER,          // Processed fiber (Wood -> Paper)
    GUNPOWDER,      // Explosive (Sulphur + Saltpeter -> Gunpowder)

    // --- Consumer Goods ---
    FOOD_RATIONS,   // Processed food (Grain + Meat -> Rations)
    CLOTHES,        // Basic attire (Cloth -> Clothes)
    FURNITURE,      // Home goods (Lumber -> Furniture)
    ALCOHOL,        // Spirits (Grain/Fruit -> Alcohol) - Increases happiness
    TOOLS,          // Equipment (Metal + Wood -> Tools) - Increases production efficiency

    // --- Luxury Goods ---
    FINE_CLOTHES,   // High-end attire (Silk + Dye -> Fine Clothes)
    JEWELRY,        // Adornments (Gold -> Jewelry)
    BOOKS,          // Knowledge (Paper -> Books) - May provide research points

    // --- Military Equipment ---
    SIMPLE_WEAPON,  // Melee weapons (Iron + Wood)
    FIREARM,        // Ranged weapons (Steel + Wood)
    AMMUNITION,     // Consumables (Gunpowder + Iron)
    ARTILLERY,      // Heavy weapons (Steel + Gunpowder)
    ARMOR,          // Protection (Iron/Steel + Leather)
    
    // --- Vehicles and Heavy Industry ---
    WAGON,          // Transportation (Lumber + Iron)
    SHIP_HULL,      // Naval construction
    MACHINERY       // Industrial equipment (Steel + Tools)
};

// JSON Serialization support
NLOHMANN_JSON_SERIALIZE_ENUM(miningTechnologies, {
    {miningTechnologies::BASIC_MINING, "BASIC_MINING"},
    {miningTechnologies::ADVANCED_MINING, "ADVANCED_MINING"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(agricultureTechnologies, {
    {agricultureTechnologies::BASIC_AGRICULTURE, "BASIC_AGRICULTURE"},
    {agricultureTechnologies::ADVANCED_AGRICULTURE, "ADVANCED_AGRICULTURE"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(manufacturingTechnologies, {
    {manufacturingTechnologies::BASIC_MANUFACTURING, "BASIC_MANUFACTURING"},
    {manufacturingTechnologies::ADVANCED_MANUFACTURING, "ADVANCED_MANUFACTURING"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(constructionTechnologies, {
    {constructionTechnologies::BASIC_CONSTRUCTION, "BASIC_CONSTRUCTION"},
    {constructionTechnologies::ADVANCED_CONSTRUCTION, "ADVANCED_CONSTRUCTION"}
})