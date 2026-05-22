#pragma once
#include <memory>

class Gamestate;
class AIBrain;

class AIManager {
public:
    using BrainFactory = std::unique_ptr<AIBrain>(*)();

    static void init();
    static void processAll(Gamestate& gamestate);
    static void registerBrainType(const std::string& entityType, BrainFactory factory);
    static std::unique_ptr<AIBrain> createBrain(const std::string& entityType);
};
