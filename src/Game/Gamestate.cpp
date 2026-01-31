/**
 * @file Gamestate.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of the Gamestate data management logic.
 * @version 0.1
 * @date 2026-01-29
 * @copyright Copyright (c) 2026
 */

#include "Gamestate.h"
#include "../../lib/nlohmann/json.hpp"

// ==========================================
// Entity Retrieval
// ==========================================

Factory *Gamestate::getFactory(const uuids::uuid &id) {
  auto it = factories.find(id);
  if (it != factories.end()) {
    return &it->second;
  }
  return nullptr;
}

Company *Gamestate::getCompany(const uuids::uuid &id) {
  auto it = companies.find(id);
  if (it != companies.end()) {
    return &it->second;
  }
  return nullptr;
}

Market *Gamestate::getMarket(const uuids::uuid &id) {
  auto it = markets.find(id);
  if (it != markets.end()) {
    return &it->second;
  }
  return nullptr;
}

TradeNode *Gamestate::getTradeNode(const uuids::uuid &id) {
  auto it = nodes.find(id);
  if (it != nodes.end()) {
    return &it->second;
  }
  return nullptr;
}

// ==========================================
// Player Management
// ==========================================

uuids::uuid Gamestate::getPlayerCompanyId() const { return playerCompanyId; }
void Gamestate::setPlayerCompanyId(const uuids::uuid &id) {
  playerCompanyId = id;
}

/**
 * @brief Creates the player's initial company setup.
 * @details Generates a UUID, sets default starting capital (10,000), 
 * and registers it in the companies map.
 */
uuids::uuid Gamestate::createPlayerCompany(const std::string &name) {
  uuids::uuid newId = IdUtils::generateUuid();
  Company newCompany(newId, name);
  newCompany.setCapital(10000); // Starting Capital
  companies.emplace(newId, newCompany);
  playerCompanyId = newId;
  return newId;
}

// ==========================================
// Serialization
// ==========================================

std::string Gamestate::serialize() const {
  nlohmann::json j = serializeGamestate(*this);
  return j.dump();
}

bool Gamestate::deserialize(const std::string &serialized) {
  // @todo Implement deserialization logic (JSON -> Gamestate Objects)
  return false;
}

// ==========================================
// Spatial Data
// ==========================================

Position Gamestate::getPosition(const uuids::uuid &id) const {
  auto it = entityPositions.find(id);
  if (it != entityPositions.end()) {
    return it->second;
  }
  return {-1, -1}; // Invalid/Error position
}

void Gamestate::setPosition(const uuids::uuid &id, const Position &pos) {
  entityPositions[id] = pos;
}

// ==========================================
// Data Management (Add/Clear)
// ==========================================

void Gamestate::addMarket(const uuids::uuid &id, const Market &market) {
  markets.emplace(id, market);
}

void Gamestate::addTradeNode(const uuids::uuid &id, const TradeNode &node) {
  nodes.emplace(id, node);
}

void Gamestate::addFactory(const uuids::uuid &id, const Factory &factory) {
  factories.emplace(id, factory);
}

void Gamestate::addCompany(const uuids::uuid &id, const Company &company) {
  companies.emplace(id, company);
}

void Gamestate::clear() {
  markets.clear();
  nodes.clear();
  companies.clear();
  factories.clear();
  entityPositions.clear();
}

// ==========================================
// Direct Accessors
// ==========================================

std::unordered_map<uuids::uuid, Market> &Gamestate::getMarkets() {
  return markets;
}
std::unordered_map<uuids::uuid, TradeNode> &Gamestate::getNodes() {
  return nodes;
}
std::unordered_map<uuids::uuid, Company> &Gamestate::getCompanies() {
  return companies;
}
std::unordered_map<uuids::uuid, Factory> &Gamestate::getFactories() {
  return factories;
}
std::unordered_map<uuids::uuid, Position> &Gamestate::getPositions() {
  return entityPositions;
}

// ==========================================
// Global Serialization Helper
// ==========================================

/**
 * @brief Serializes the complete game state into a JSON object.
 * @details Iterates through all entity maps and converts them using their respective to_json functions.
 */
nlohmann::json serializeGamestate(const Gamestate &gamestate) {
  nlohmann::json j;

  j["playerCompanyId"] = uuids::to_string(gamestate.getPlayerCompanyId());

  // Companies
  nlohmann::json companiesJson;
  for (const auto &[id, company] : gamestate.companies) {
    companiesJson[uuids::to_string(id)] = company;
  }
  j["companies"] = companiesJson;

  // Factories
  nlohmann::json factoriesJson;
  for (const auto &[id, factory] : gamestate.factories) {
    factoriesJson[uuids::to_string(id)] = factory;
  }
  j["factories"] = factoriesJson;

  // Markets
  nlohmann::json marketsJson;
  for (const auto &[id, market] : gamestate.markets) {
    marketsJson[uuids::to_string(id)] = market;
  }
  j["markets"] = marketsJson;

  // TradeNodes
  nlohmann::json nodesJson;
  for (const auto &[id, node] : gamestate.nodes) {
    nodesJson[uuids::to_string(id)] = node;
  }
  j["tradeNodes"] = nodesJson;

  // Positions
  nlohmann::json positionsJson;
  for (const auto &[id, pos] : gamestate.entityPositions) {
    positionsJson[uuids::to_string(id)] = pos;
  }
  j["positions"] = positionsJson;

  return j;
}