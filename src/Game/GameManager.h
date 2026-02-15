// GameManager.h
#pragma once
#include "Gamestate.h"

class GameManager {
public:
    GameManager() = delete;

    /**
     * @brief Simülasyonu 1 tur ilerletir.
     * Döngü: Date -> Companies -> Nodes -> Markets -> Financials
     */
    static void stepGamestate(Gamestate &gamestate);

    static void update(Gamestate &gamestate, float deltaTime);
};