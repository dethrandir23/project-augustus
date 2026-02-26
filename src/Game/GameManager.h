// GameManager.h
#pragma once
#include "Gamestate.h"

class GameManager {
public:
    GameManager() = delete; // Statik class

    /**
     * @brief Simülasyonu 1 tur ilerletir. (Eski adıyla stepGamestate)
     */
    static void tick(Gamestate &gamestate);

    static void update(Gamestate &gamestate, float deltaTime);

private:
    // --- Sistemler (Systems) ---
    static void processPerks(Gamestate& gamestate);
    static void processDemographics(Gamestate& gamestate);
    static void processCompanyLogistics(Gamestate& gamestate);
    static void processFactories(Gamestate& gamestate); // YENİ: Fabrikaları çalıştırır
    static void processTradeNodes(Gamestate& gamestate);
};