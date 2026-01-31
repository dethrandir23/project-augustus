/**
 * @file Templates.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Predefined factory templates (blueprints) for the game "Producer".
 * @details Defines standardized factory types like Sawmills, Steel Mills, etc., 
 * populated with their respective production pipelines.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <initializer_list>
#include <string>
#include <vector>
#include "Pipeline.h"

/**
 * @struct FactoryTemplate
 * @brief A blueprint for creating a Factory with pre-assigned pipelines.
 */
struct FactoryTemplate {
    std::vector<Pipeline> pipelines;

    FactoryTemplate() = default;
    
    /**
     * @brief Constructor to initialize a template with a list of pipelines.
     * @param p Initializer list of Pipeline objects.
     */
    FactoryTemplate(std::initializer_list<Pipeline> p)
        : pipelines(p) {}

    /**
    * @brief Assigns the contents of another FactoryTemplate object to this one.
    * @param rhs The FactoryTemplate object to assign from.
    */
    void operator=(const FactoryTemplate &rhs) {
        this->pipelines = rhs.pipelines;
        return;
    }
};

/**
 * @namespace DefaultFactoryTemplates
 * @brief Static collection of all standard factory types available in the game.
 */
namespace DefaultFactoryTemplates {

    // ==========================================
    // 1. BASIC CONSTRUCTION & MATERIAL PROCESSING
    // ==========================================

    /**
     * @brief Sawmill
     * @details Converts raw logs into usable lumber for construction and furniture.
     */
    const FactoryTemplate SAWMILL = {
        {
            Pipeline{
                { {ItemType::WOOD, 10.0f} },
                { {ItemType::LUMBER, 0.0, 8.0f}, {ItemType::WOOD, 0.0, 1.0f} }, // Produces some waste/firewood
                1.0
            }
        }
    };

    /**
     * @brief Brickworks
     * @details Processes Clay or Stone into Bricks for construction.
     */
    const FactoryTemplate BRICKWORKS = {
        {
            // Line 1: Clay -> Brick (Standard)
            Pipeline{
                { {ItemType::CLAY, 10.0f}, {ItemType::COAL, 2.0f} },
                { {ItemType::BRICK, 0.0, 10.0f} },
                0.95
            },
            // Line 2: Stone -> Cut Stone (Treated as Brick substitute)
            Pipeline{
                { {ItemType::STONE, 10.0f} },
                { {ItemType::BRICK, 0.0, 8.0f} }, // Harder to process
                0.8
            }
        }
    };

    /**
     * @brief Glassworks
     * @details Melts Sand into Glass.
     */
    const FactoryTemplate GLASSWORKS = {
        {
            Pipeline{
                { {ItemType::SAND, 10.0f}, {ItemType::COAL, 3.0f} },
                { {ItemType::GLASS, 0.0, 5.0f} },
                0.9
            }
        }
    };

    // ==========================================
    // 2. HEAVY INDUSTRY & METALLURGY
    // ==========================================

    /**
     * @brief Steel Mill
     * @details The backbone of industry. Smelts Iron Ore and Coal into Steel.
     */
    const FactoryTemplate STEEL_MILL = {
        {
            Pipeline{
                { {ItemType::IRON_ORE, 5.0f}, {ItemType::COAL, 5.0f} },
                { {ItemType::STEEL, 0.0, 4.0f} },
                1.0
            }
        }
    };

    /**
     * @brief Machine Shop
     * @details Advanced factory that turns Steel into complex Machinery.
     */
    const FactoryTemplate MACHINE_FACTORY = {
        {
            Pipeline{
                { {ItemType::STEEL, 10.0f}, {ItemType::TOOLS, 2.0f} },
                { {ItemType::MACHINERY, 0.0, 1.0f} },
                0.8
            }
        }
    };

    /**
     * @brief Tool Factory
     * @details Manufactures basic Tools required for other production lines.
     */
    const FactoryTemplate TOOL_FACTORY = {
        {
            Pipeline{
                { {ItemType::STEEL, 2.0f}, {ItemType::WOOD, 1.0f} },
                { {ItemType::TOOLS, 0.0, 5.0f} },
                1.1
            }
        }
    };

    // ==========================================
    // 3. TEXTILES & CLOTHING
    // ==========================================

    /**
     * @brief Textile Mill
     * @details Processes raw fibers (Cotton or Wool) into Cloth.
     */
    const FactoryTemplate TEXTILE_MILL = {
        {
            // Line 1: Cotton Processing
            Pipeline{
                { {ItemType::COTTON, 10.0f} },
                { {ItemType::CLOTH, 0.0, 8.0f} },
                1.0
            },
            // Line 2: Wool Processing
            Pipeline{
                { {ItemType::WOOL, 10.0f} },
                { {ItemType::CLOTH, 0.0, 6.0f} },
                1.0
            }
        }
    };

    /**
     * @brief Garment Factory (Clothing)
     * @details Sews Cloth into finished Clothes. Capable of Luxury production.
     */
    const FactoryTemplate CLOTHING_FACTORY = {
        {
            // Standard Clothes
            Pipeline{
                { {ItemType::CLOTH, 5.0f} },
                { {ItemType::CLOTHES, 0.0, 5.0f} },
                1.0
            },
            // Luxury Clothes (Requires Silk & Dye)
            Pipeline{
                { {ItemType::SILK, 5.0f}, {ItemType::DYE, 1.0f} },
                { {ItemType::FINE_CLOTHES, 0.0, 2.0f} },
                0.9
            }
        }
    };

    // ==========================================
    // 4. FOOD & CONSUMABLES
    // ==========================================

    /**
     * @brief Food Processing Plant
     * @details Industrial scale food production from Grain or Meat.
     */
    const FactoryTemplate FOOD_PLANT = {
        {
            // Grain Processing
            Pipeline{
                { {ItemType::GRAIN, 10.0f} },
                { {ItemType::FOOD_RATIONS, 0.0, 8.0f} },
                1.0
            },
            // Meat Processing (Cannery)
            Pipeline{
                { {ItemType::MEAT, 10.0f}, {ItemType::IRON_ORE, 0.5f} }, // Small iron usage for cans
                { {ItemType::FOOD_RATIONS, 0.0, 10.0f} },
                1.0
            }
        }
    };

    /**
     * @brief Furniture Factory
     * @details Carpentry workshop producing consumer goods.
     */
    const FactoryTemplate FURNITURE_FACTORY = {
        {
            Pipeline{
                { {ItemType::LUMBER, 5.0f}, {ItemType::CLOTH, 1.0f} },
                { {ItemType::FURNITURE, 0.0, 3.0f} },
                1.0
            }
        }
    };

    /**
     * @brief Paper Mill
     * @details Processes Wood into Paper.
     */
    const FactoryTemplate PAPER_MILL = {
        {
            Pipeline{
                { {ItemType::WOOD, 5.0f} },
                { {ItemType::PAPER, 0.0, 10.0f} },
                0.9
            }
        }
    };

    // ==========================================
    // 5. MILITARY INDUSTRY
    // ==========================================

    /**
     * @brief Arms Factory
     * @details Produces personal firearms and ammunition.
     */
    const FactoryTemplate ARMS_FACTORY = {
        {
            // Firearms Line
            Pipeline{
                { {ItemType::STEEL, 3.0f}, {ItemType::WOOD, 2.0f} },
                { {ItemType::FIREARM, 0.0, 5.0f} },
                1.0
            },
            // Ammunition Line
            Pipeline{
                { {ItemType::GUNPOWDER, 2.0f}, {ItemType::IRON_ORE, 2.0f} },
                { {ItemType::AMMUNITION, 0.0, 20.0f} },
                1.0
            }
        }
    };

    /**
     * @brief Explosives Factory (Chemical Plant)
     * @details Mixes dangerous chemicals to create Gunpowder.
     */
    const FactoryTemplate EXPLOSIVES_FACTORY = {
        {
            Pipeline{
                { {ItemType::SULPHUR, 5.0f}, {ItemType::SALTPETER, 5.0f} },
                { {ItemType::GUNPOWDER, 0.0, 10.0f} },
                0.8
            }
        }
    };
    
    /**
     * @brief Shipyard
     * @details Massive facility for constructing ship hulls.
     */
    const FactoryTemplate SHIPYARD = {
        {
             Pipeline{
                { {ItemType::LUMBER, 50.0f}, {ItemType::CLOTH, 20.0f}, {ItemType::IRON_ORE, 5.0f} },
                { {ItemType::SHIP_HULL, 0.0, 1.0f} },
                0.7
            }
        }
    };
}