#pragma once
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace AIPicker {

    struct Candidate { 
        std::string actionId; // Örn: "factory_iron", "pay_loan" vs.
        double prob; 
    };

    inline double calculateProbability(double score, double temperature) {
        // Sıfıra bölünme hatasını önlemek için ufak bir güvenlik
        if (temperature < 0.001) temperature = 0.001; 
        return std::exp(score / temperature);
    }

    inline std::string pickWeighted(const std::vector<Candidate>& candidates, int k) {
        if (candidates.empty()) return "";
        
        double totalProb = 0;
        for (int i = 0; i < k; ++i) {
            totalProb += candidates[i].prob;
        }

        static std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<> dis(0, totalProb);
        double target = dis(gen);

        double currentSum = 0;
        for (int i = 0; i < k; ++i) {
            currentSum += candidates[i].prob;
            if (target <= currentSum) {
                return candidates[i].actionId;
            }
        }
        return candidates[0].actionId;
    }

    inline std::string selectWithNoise(std::vector<Candidate>& candidates, int k, double temperature) {
        if (candidates.empty()) return "";

        // 1. Ham skorları Softmax/Temperature ile olasılığa çevir
        for (auto& c : candidates) {
            c.prob = calculateProbability(c.prob, temperature); // Önceden c.prob içinde ham skor tutuluyordu varsayıyoruz
        }

        // 2. Adayları olasılığa göre sırala
        std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b){
            return a.prob > b.prob;
        });

        // 3. İlk K tanesini al
        int actualK = std::min((int)candidates.size(), k);

        // 4. Ağırlıklı rastgele seçim yap
        return pickWeighted(candidates, actualK);
    }
}