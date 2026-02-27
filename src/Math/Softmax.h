// Softmax.h
#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace Math {

    /**
     * @brief Softmax function
     * @param inputs The input values to be normalized
     * @param temperature Controls the randomness of the output (higher = more uniform, lower = more sharp)
     * @return A vector of probabilities that sum to 1.0
     */
    inline std::vector<float> Softmax(const std::vector<float>& inputs, float temperature = 1.0f) {
        if (inputs.empty()) return {};
        
        // Sıcaklık koruması (0'a bölme hatasını önleyelim)
        float T = (temperature <= 0.0f) ? 1.0f : temperature;

        std::vector<float> outputs(inputs.size());
        float max_val = *std::max_element(inputs.begin(), inputs.end());

        float sum = 0.0f;
        for (size_t i = 0; i < inputs.size(); ++i) {
            // Sıcaklığı burada uygula, yeni vector oluşturma!
            outputs[i] = std::exp((inputs[i] - max_val) / T);
            sum += outputs[i];
        }

        for (size_t i = 0; i < outputs.size(); ++i) {
            outputs[i] /= sum;
        }

        return outputs;
    }

    /**
     * @brief Calculates the softmax value for a single element in a vector
     * @param inputs The input values
     * @param index The index of the element to calculate
     * @param temperature Controls the randomness of the output
     * @return The probability of the element at the given index
     */
    inline float SoftmaxSingle(const std::vector<float>& inputs, size_t index, float temperature = 1.0f) {
        if (inputs.empty() || index >= inputs.size()) return 0.0f;
        
        float T = (temperature <= 0.0f) ? 1.0f : temperature;
        float max_val = *std::max_element(inputs.begin(), inputs.end());
        
        float sum = 0.0f;
        for (float val : inputs) {
            sum += std::exp((val - max_val) / T);
        }

        return std::exp((inputs[index] - max_val) / T) / sum;
    }
}