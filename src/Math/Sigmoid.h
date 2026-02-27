// Sigmoid.h
#pragma once

#include <vector>
#include <cmath>

namespace Math {

    /**
     * @brief Sigmoid activation function.
     * @param x The input value.
     * @return The value mapped to the range (0, 1).
     */
    inline float Sigmoid(float x) {
        // 
        if (x >= 45.0f) return 1.0f;
        if (x <= -45.0f) return 0.0f;
        
        return 1.0f / (1.0f + std::exp(-x));
    }

    /**
     * @brief Applies sigmoid to all elements in a vector in-place.
     * @param inputs The vector of values to be modified.
     * @note This function modifies the input vector in-place, so it does not create a new one.
     */
    inline void SigmoidInPlace(std::vector<float>& inputs) {
        for (float& val : inputs) {
            val = Sigmoid(val);
        }
    }
}