/**
 * @file Gamestate.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief The central data repository for the "Producer" game simulation.
 * @details This class holds the authoritative state of the world, including all 
 * active Companies, Factories, Markets, TradeNodes, and their spatial positions.
 * It acts as the "Database" that the GameManager manipulates.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#pragma once
#include "../../lib/nlohmann/json.hpp"
#include "../Economy/Company.h"
#include "../Economy/Factory.h"
#include "../World/Market.h"
#include "../World/TradeNode.h"
#include "Game/IdUtils.h"
#include <iostream>
#include <unordered_map>

/**
 * @class Gamestate
 * @brief Container class for all game entities.
 */
class Gamestate {
public:
  Gamestate() = default;
  ~Gamestate() = default;

  /**
   * @brief Friend function to handle JSON serialization logic.
   */
  friend nlohmann::json serializeGamestate(const Gamestate &gamestate);

  /** @name Entity Retrieval (Safe Lookups)
   * Methods return pointers to live objects or nullptr if not found.
   * @{
   */
  Factory *getFactory(const uuids::uuid &id);
  Company *getCompany(const uuids::uuid &id);
  Market *getMarket(const uuids::uuid &id);
  TradeNode *getTradeNode(const uuids::uuid &id);
  /** @} */

  /** @name Player Management
   * @{
   */
  uuids::uuid getPlayerCompanyId() const;
  void setPlayerCompanyId(const uuids::uuid &id);

  /**
   * @brief Initializes a new company for the player.
   * @param name Name of the company.
   * @return uuids::uuid The ID of the newly created company.
   */
  uuids::uuid createPlayerCompany(const std::string &name);
  /** @} */

  /** @name Serialization
   * @{
   */
  /** @brief Converts the entire gamestate to a JSON string. */
  std::string serialize() const;

  /** @brief Reconstructs gamestate from a JSON string. @return True if successful. */
  bool deserialize(const std::string &serialized);
  /** @} */

  /** @name Spatial Data
   * @{
   */
  Position getPosition(const uuids::uuid &id) const;
  void setPosition(const uuids::uuid &id, const Position &pos);
  /** @} */

  /** @name Data Management (Write Access)
   * Methods to register new entities into the simulation.
   * @{
   */
  void addMarket(const uuids::uuid &id, const Market &market);
  void addTradeNode(const uuids::uuid &id, const TradeNode &node);
  void addFactory(const uuids::uuid &id, const Factory &factory);
  void addCompany(const uuids::uuid &id, const Company &company);

  /** @brief Wipes all data. Used when starting a new game or loading a save. */
  void clear();
  /** @} */

  /** @name Direct Container Access
   * Returns references to the underlying maps for iteration by the GameManager.
   * @{
   */
  std::unordered_map<uuids::uuid, Market> &getMarkets();
  std::unordered_map<uuids::uuid, TradeNode> &getNodes();
  std::unordered_map<uuids::uuid, Company> &getCompanies();
  std::unordered_map<uuids::uuid, Factory> &getFactories();
  std::unordered_map<uuids::uuid, Position> &getPositions();
  /** @} */


private:
  // Primary Data Stores
  std::unordered_map<uuids::uuid, Market> markets;
  std::unordered_map<uuids::uuid, TradeNode> nodes;
  std::unordered_map<uuids::uuid, Company> companies;
  std::unordered_map<uuids::uuid, Factory> factories;

  // Spatial Index
  std::unordered_map<uuids::uuid, Position> entityPositions;

  // Metadata
  uuids::uuid playerCompanyId;
};

/**
 * @brief Global helper to serialize the gamestate.
 */
nlohmann::json serializeGamestate(const Gamestate &gamestate);