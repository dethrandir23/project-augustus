// GameManager.h
#pragma once
#include "Gamestate.h"

class GameManager {
public:
    GameManager() = delete;

    static void tick(Gamestate &gamestate);

    static void update(Gamestate &gamestate, float deltaTime);

private:
    // --- Sistemler (Systems) ---
    static void processPerks(Gamestate& gamestate);
    static void processDemographics(Gamestate& gamestate);
    static void processCompanyLogistics(Gamestate& gamestate);
    static void processFactories(Gamestate& gamestate);
    static void processTradeNodes(Gamestate& gamestate);
    static void processAI(Gamestate& gamestate);
    static void expireOrders(Gamestate& gamestate);
};