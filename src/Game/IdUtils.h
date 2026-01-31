/**
 * @file IdUtils.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Utility class for generating Unique Identifiers (UUIDs).
 * @details Wraps the 'stduuid' library to provide a centralized, thread-safe, 
 * and efficient way to create random IDs for game entities.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <algorithm>
#include <string>
#include "../../lib/uuid/uuid.h"
#include <random>

/**
 * @class IdUtils
 * @brief Static helper for ID generation.
 */
class IdUtils {
public:
    /**
     * @brief Generates a new random UUID (Version 4).
     * @details Uses a static Mersenne Twister engine to ensure the random number 
     * generator is initialized only once per program execution (efficient).
     * @return uuids::uuid A cryptographically strong unique identifier.
     */
    static inline uuids::uuid generateUuid() {
        static std::random_device rd;
        static std::mt19937 engine(rd()); 
        
        static uuids::uuid_random_generator gen_uuid(engine);
        
        return gen_uuid();
    }

};