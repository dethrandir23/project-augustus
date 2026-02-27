// Relu.h
#pragma once

#include <vector>
#include <algorithm>

namespace Math {

    /**
     * @brief Rectified Linear Unit (ReLU) activation function.
     * @param x The input value.
     * @return The maximum of 0 and x.
     
     */
    inline float ReLU(float x) {
        return std::max(0.0f, x);
    }

    /**
     * @brief Leaky Rectified Linear Unit (LeakyReLU) activation function.
     * @param x The input value.
     * @param alpha The slope for negative values (default is 0.01).
     * @return x if x > 0, otherwise x * alpha.
     */
    inline float LeakyReLU(float x, float alpha = 0.01f) {
        return (x > 0.0f) ? x : x * alpha;
    }
    
    /**
     * @brief Applies ReLU to all elements in a vector in-place.
     * @param inputs The vector of values to be modified.
     * @note This function modifies the input vector in-place, so it does not create a new one.
     */
    inline void ReLUInPlace(std::vector<float>& inputs) {
        for (float& val : inputs) {
            val = ReLU(val);
        }
    }
}