#pragma once
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace AIPicker {

    struct Candidate { 
        std::string actionId;
        double prob; 
    };

    inline double calculateProbability(double score, double temperature) {
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

        for (auto& c : candidates) {
            c.prob = calculateProbability(c.prob, temperature);
        }

        std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b){
            return a.prob > b.prob;
        });

        int actualK = std::min((int)candidates.size(), k);

        return pickWeighted(candidates, actualK);
    }
}