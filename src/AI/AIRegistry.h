#pragma once
#include <string>
#include <unordered_map>
#include <functional>

class Gamestate;
class Entity;

class AIRegistry {
public:

    using AILogicFunc = std::function<void(Entity&, Gamestate&)>;

    static void init();

    static void registerLogic(const std::string& entityType, AILogicFunc logic);

    static void executeLogic(Entity& entity, Gamestate& gamestate);

private:
    static std::unordered_map<std::string, std::vector<AILogicFunc>> logicRegistry;
};