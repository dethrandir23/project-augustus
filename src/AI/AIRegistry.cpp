#include "AIRegistry.h"
#include "Core/ECS/Entity.h"

std::unordered_map<std::string, std::vector<AIRegistry::AILogicFunc>> AIRegistry::logicRegistry;

void AIRegistry::registerLogic(const std::string& entityType, AILogicFunc logic) {
    logicRegistry[entityType].push_back(logic);
}

void AIRegistry::executeLogic(Entity& entity, Gamestate& gamestate) {
    auto it = logicRegistry.find(entity.GetType());
    if (it != logicRegistry.end()) {
        for (const auto& logicFunc : it->second) {
            logicFunc(entity, gamestate);
        }
    }
}

void AIRegistry::init() {
    // Deprecated: use AIManager instead
    // Brain registration is now handled by AIManager::init()
}
