/**
 * @file Constants.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Core economic and demographic constants for the game "Producer".
 * @version 0.1
 * @date 2026-01-29
 * * @copyright Copyright (c) 2026
 */

#pragma once

/**
 * @brief Contains all constant values used within the game engine and economy simulation.
 */
namespace Constants {
    // Demographic Constants
    const int VILLAGE_STARTING_POPULATION = 100;
    const int TOWN_STARTING_POPULATION = 1000;
    const int CITY_STARTING_POPULATION = 10000;
    const int METROPOLIS_STARTING_POPULATION = 100000;

    // Economic Constants
    const int MAX_EMPLOYEES_PER_FACTORY = 1000;
    const int MIN_PRICE = 1;
    const int MAX_PRICE = 10000;
} // namespace Constants