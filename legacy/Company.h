#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Types.h"
#include "../Registry/EconomyManager.h"
#include "../World/TradeNode.h"
#include "Core/ECS/Entity.h"
#include "Core/Inventory.h"
#include "Registry/CompanyManager.h"
#include <string>
#include <vector>

// Forward Declaration
class Gamestate;
struct CompanyTemplate;

class Company {
public:
  Company() = default;
  // Explicit ID ve Name constructorları
  Company(const uuids::uuid &id, const std::string &name)
      : id(id), name(name) {}

  Company(const uuids::uuid &id, const CompanyTemplate &tmpl);

  // --- Core Properties ---
  void setName(const std::string &name) { this->name = name; }
  std::string getName() const { return name; }

  void setId(const uuids::uuid &newId) { id = newId; }
  uuids::uuid getId() const { return id; }

  void setCapital(double cap) {
    capital = cap;
  } // double daha iyi (küsüratlı paralar için)
  double getCapital() const { return capital; }
  double& getCapitalRef() { return capital; }

  void setDebt(double d) { debt = d; }
  double getDebt() const { return debt; }

  // Profit değişkenini kaldırdık dedin, süper!

  // --- Manpower ---
  void setManpower(size_t count) { manpower = count; }
  size_t getManpower() const { return manpower; }
  void addManpower(size_t count) { manpower += count; }
  void removeManpower(size_t count) {
    manpower = (manpower > count) ? (manpower - count) : 0;
  }
  // Company.h
  void recruitWorkersFromNode(const uuids::uuid &nodeId, size_t count,
                              Gamestate &gamestate);

  // --- Factory Management ---
  void addFactory(const uuids::uuid &fid) { factories.push_back(fid); }
  const std::vector<uuids::uuid> &getFactoryIds() const { return factories; }
  void removeFactoryRef(const uuids::uuid &fid);

  // --- Storage (Unified) ---
  // Artık hammadde/ürün ayrımı yok. Hepsi storage içinde.
  void addItem(const std::string &itemId, float amount);
  float getItemAmount(const std::string &itemId) const;
Inventory& getInventory() { return inventory; }
    const Inventory& getInventory() const { return inventory; }

  // --- Gameplay Logic ---
  // Bu fonksiyonlar Gamestate üzerinden fabrikalara erişecek
  void manageFactories(
      Gamestate &gamestate); // Hepsini tek çatı altında toplayabiliriz

  // --- Technologies ---
  void addTechnology(const std::string &techId);
  const std::vector<std::string> &getKnownTechnologies() const;

  std::vector<std::string> getUnlockedTechnologies() const;

  void addPerk(const std::string &perkId);

  // Her tur çağrılacak fonksiyon
  void tickPerks();

  void removePerk(const std::string &perkID);

  // Getter (Sadece ID listesi döndürmek istersek UI için)
  std::vector<std::string> getActivePerkIds() const;

  float calculateModifier(const std::string &modifierType, float baseValue);

  Inventory &getStorage() { return inventory; }

  bool buyFromMarket(Market& market, const std::vector<ItemStack>& shoppingList);


  // --- Serialization ---
  friend void to_json(nlohmann::json &j, const Company &c);

  std::vector<std::string> knownTechnologies;

private:
  uuids::uuid id;
  std::string name;

  double capital = 0.0;
  double debt = 0.0;
  size_t manpower = 0;

  std::vector<ActivePerk> activePerks;

  std::vector<uuids::uuid> factories;
  Inventory inventory;
};