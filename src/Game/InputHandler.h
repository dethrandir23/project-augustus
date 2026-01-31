#pragma once
#include "../../lib/nlohmann/json.hpp"
#include "../Economy/Company.h"
#include "../Economy/Factory.h"

class Gamestate;

enum class InputType {
  // --- SİMÜLASYON KONTROL ---
  STEP_GAME,  // Bir tur ilerlet (Manual step)
  RESET_GAME, // Oyunu sıfırla (Yeni oyun başlat)

  // --- İNŞAAT VE YÖNETİM ---
  BUILD_FACTORY,  // Yeni fabrika kur (Şablondan)
  SCRAP_FACTORY,  // Fabrikayı yık (Geriye biraz kaynak dönebilir)
  RENAME_FACTORY, // Fabrika ismini değiştir

  // --- İŞ GÜCÜ (WORKFORCE) ---
  CHANGE_WORKFORCE, // İşçi al veya çıkar (+10 veya -5 gibi delta)

  // --- TİCARET (MANUAL TRADING) ---
  MARKET_BUY_ITEM,
  MARKET_SELL_ITEM,

  // --- DEBUG / HİLELER (Geliştirme süreci için şart) ---
  ADD_MONEY, // Para ekle
  ADD_ITEM,  // Depoya havadan hammadde indir

  UNKNOWN
};

InputType stringToInputType(const std::string &str) {
  static const std::unordered_map<std::string, InputType> inputTypeMapping = {
      {"STEP_GAME", InputType::STEP_GAME},
      {"RESET_GAME", InputType::RESET_GAME},
      {"BUILD_FACTORY", InputType::BUILD_FACTORY},
      {"SCRAP_FACTORY", InputType::SCRAP_FACTORY},
      {"RENAME_FACTORY", InputType::RENAME_FACTORY},
      {"CHANGE_WORKFORCE", InputType::CHANGE_WORKFORCE},
      {"MARKET_BUY_ITEM", InputType::MARKET_BUY_ITEM},
      {"MARKET_SELL_ITEM", InputType::MARKET_SELL_ITEM},
      {"ADD_MONEY", InputType::ADD_MONEY},
      {"ADD_ITEM", InputType::ADD_ITEM},
      {"UNKNOWN", InputType::UNKNOWN}};

  auto it = inputTypeMapping.find(str);
  if (it != inputTypeMapping.end()) {
    return it->second;
  }
  return InputType::UNKNOWN;
}

static class InputHandler {
public:
  InputHandler() = delete;

  static bool handleInput(Gamestate &gamestate, const nlohmann::json &input);

  static bool handleStepGame(Gamestate &gamestate, const nlohmann::json &input);
  static bool handleResetGame(Gamestate &gamestate,
                              const nlohmann::json &input);

  static bool handleBuildFactory(Gamestate &gamestate,
                                 const nlohmann::json &input);
  static bool handleRenameFactory(Gamestate &gamestate,
                                  const nlohmann::json &input);
  static bool handleScrapFactory(Gamestate &gamestate,
                                 const nlohmann::json &input);

  static bool handleChangeWorkforce(Gamestate &gamestate,
                                    const nlohmann::json &input);

  static bool handleMarketBuyItem(Gamestate &gamestate,
                                  const nlohmann::json &input);
  static bool handleMarketSellItem(Gamestate &gamestate,
                                   const nlohmann::json &input);

  static bool handleAddMoney(Gamestate &gamestate, const nlohmann::json &input);
  static bool handleAddItem(Gamestate &gamestate, const nlohmann::json &input);

private:
};