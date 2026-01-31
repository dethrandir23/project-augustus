/**
 * @file Pipeline.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of default game recipes.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "Pipeline.h"

std::vector<Pipeline> DefaultPipelines::getAllRecipes() {
        return {
            // ==========================================
            // 1. BASIC MATERIALS & CONSTRUCTION
            // ==========================================
            
            // Lumber: Logs -> Processed Lumber
            Pipeline{
                { {ItemType::WOOD, 10.0f} },                // Input
                { {ItemType::LUMBER, 0.0, 8.0f} },          // Output (Price 0.0 is placeholder)
                1.0                                         // Efficiency
            },

            // Brick: Clay + Coal (for firing) -> Brick
            Pipeline{
                { {ItemType::CLAY, 10.0f}, 
                  {ItemType::COAL, 2.0f} },
                { {ItemType::BRICK, 0.0, 10.0f} }, 
                0.95 
            },

            // Glass: Sand + Coal -> Glass
            Pipeline{
                { {ItemType::SAND, 10.0f},
                  {ItemType::COAL, 3.0f} },
                { {ItemType::GLASS, 0.0, 5.0f} },
                0.90
            },

            // ==========================================
            // 2. METALLURGY
            // ==========================================

            // Steel Production: Iron Ore + Coal -> Steel
            Pipeline{
                { {ItemType::IRON_ORE, 5.0f},
                  {ItemType::COAL, 5.0f} },
                { {ItemType::STEEL, 0.0, 4.0f} }, 
                1.0 
            },

            // Copper Processing: Skipped for now, assuming direct usage in tools/electronics if added later.

            // ==========================================
            // 3. MANUFACTURING & CRAFTING
            // ==========================================

            // Tools: Steel + Wood (Handles) -> Tools
            Pipeline{
                { {ItemType::STEEL, 2.0f},
                  {ItemType::WOOD, 2.0f} },
                { {ItemType::TOOLS, 0.0, 5.0f} },
                1.1 // High quality tools produce more tools (logic)
            },

            // Furniture: Lumber -> Furniture
            Pipeline{
                { {ItemType::LUMBER, 5.0f} }, 
                { {ItemType::FURNITURE, 0.0, 2.0f} },
                1.0
            },

            // Paper: Wood -> Paper
            Pipeline{
                { {ItemType::WOOD, 5.0f} },
                { {ItemType::PAPER, 0.0, 10.0f} },
                0.8
            },

            // Books: Paper + Hides (Binding) -> Books
            Pipeline{
                { {ItemType::PAPER, 5.0f},
                  {ItemType::HIDES, 1.0f} }, // Could add Dye/Ink later
                { {ItemType::BOOKS, 0.0, 2.0f} },
                1.0
            },

            // ==========================================
            // 4. TEXTILES
            // ==========================================

            // Cloth (Cotton):
            Pipeline{
                { {ItemType::COTTON, 10.0f} },
                { {ItemType::CLOTH, 0.0, 8.0f} },
                1.0
            },

            // Cloth (Wool):
            Pipeline{
                { {ItemType::WOOL, 10.0f} },
                { {ItemType::CLOTH, 0.0, 6.0f} }, // Wool might be less efficient/thicker
                1.0
            },

            // Clothes (Standard): Cloth -> Clothes
            Pipeline{
                { {ItemType::CLOTH, 5.0f} },
                { {ItemType::CLOTHES, 0.0, 5.0f} },
                1.0
            },

            // Fine Clothes: Silk + Dye -> Luxury Clothes
            Pipeline{
                { {ItemType::SILK, 5.0f},
                  {ItemType::DYE, 2.0f} },
                { {ItemType::FINE_CLOTHES, 0.0, 2.0f} },
                0.9 // Delicate production
            },

            // ==========================================
            // 5. FOOD & CONSUMABLES
            // ==========================================

            // Rations: Grain + Meat -> Packaged Food
            Pipeline{
                { {ItemType::GRAIN, 5.0f},
                  {ItemType::MEAT, 5.0f} },
                { {ItemType::FOOD_RATIONS, 0.0, 10.0f} },
                1.0
            },

            // Alcohol (Grain): Beer/Vodka
            Pipeline{
                { {ItemType::GRAIN, 10.0f} },
                { {ItemType::ALCOHOL, 0.0, 4.0f} },
                0.9
            },

            // Alcohol (Fruit): Wine
            Pipeline{
                { {ItemType::FRUIT, 10.0f} },
                { {ItemType::ALCOHOL, 0.0, 5.0f} },
                0.95
            },

            // ==========================================
            // 6. MILITARY & HEAVY INDUSTRY
            // ==========================================

            // Gunpowder: Sulphur + Saltpeter
            Pipeline{
                { {ItemType::SULPHUR, 5.0f},
                  {ItemType::SALTPETER, 5.0f} },
                { {ItemType::GUNPOWDER, 0.0, 10.0f} },
                0.8
            },

            // Firearm: Steel + Wood -> Rifle
            Pipeline{
                { {ItemType::STEEL, 3.0f},
                  {ItemType::WOOD, 2.0f} }, 
                { {ItemType::FIREARM, 0.0, 5.0f} },
                1.0
            },

            // Artillery: High Steel + Gunpowder (Testing/Proving)
            Pipeline{
                { {ItemType::STEEL, 15.0f},
                  {ItemType::GUNPOWDER, 5.0f} }, 
                { {ItemType::ARTILLERY, 0.0, 1.0f} }, // Low volume, high value
                0.9
            },

            // Ammunition: Gunpowder + Iron (Bullets/Shells)
            Pipeline{
                { {ItemType::GUNPOWDER, 2.0f},
                  {ItemType::IRON_ORE, 2.0f} },
                { {ItemType::AMMUNITION, 0.0, 20.0f} }, // Mass production
                1.0
            },

            // Ship Hull: Lumber + Cloth (Sails) + Iron (Nails/Fitting)
            Pipeline{
                { {ItemType::LUMBER, 50.0f},
                  {ItemType::CLOTH, 20.0f},
                  {ItemType::IRON_ORE, 5.0f} },
                { {ItemType::SHIP_HULL, 0.0, 1.0f} },
                0.7 // Time consuming
            },
            
            // Machinery: Steel + Tools (Tools needed to build machines)
            Pipeline{
                { {ItemType::STEEL, 10.0f},
                  {ItemType::TOOLS, 5.0f} },
                { {ItemType::MACHINERY, 0.0, 1.0f} },
                0.8
            }
        };
}