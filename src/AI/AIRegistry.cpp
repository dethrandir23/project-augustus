#include "AIRegistry.h"
#include "AI/Logic/CompanyLogic.h"
#include "AI/Logic/TradeNodeLogic.h"
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
    registerLogic("company", [](Entity& e, Gamestate& gs) {
        CompanyLogic::manageDebt(e, gs);
        CompanyLogic::buildFactories(e, gs);
        CompanyLogic::sellSurplus(e, gs);
        CompanyLogic::handleEvents(e, gs);
    });
    registerLogic("trade_node", [](Entity& e, Gamestate& gs) {
        TradeNodeLogic::buyConsumptionNeeds(e, gs);
        TradeNodeLogic::sellSurplus(e, gs);
        TradeNodeLogic::handleEvents(e, gs);
    });
}