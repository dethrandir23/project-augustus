#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <unordered_map>

class Gamestate;

enum class InputType {
    STEP_GAME,
    RESET_GAME,
    BUILD_FACTORY,
    SCRAP_FACTORY,
    RENAME_FACTORY,
    CHANGE_WORKFORCE,
    MARKET_BUY_ITEM,
    MARKET_SELL_ITEM,
    ADD_MONEY,
    ADD_ITEM,
    UNKNOWN
};

class InputHandler {
public:
    InputHandler() = delete;

    // Ana giriş noktası
    static bool handleInput(Gamestate &gamestate, const nlohmann::json &input);

private:
    // İşlemciler
    static bool handleStepGame(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleBuildFactory(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleScrapFactory(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleRenameFactory(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleChangeWorkforce(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleMarketBuyItem(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleMarketSellItem(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleAddMoney(Gamestate &gamestate, const nlohmann::json &payload);
    static bool handleAddItem(Gamestate &gamestate, const nlohmann::json &payload);

    // Yardımcı
    static InputType stringToInputType(const std::string &str);
};