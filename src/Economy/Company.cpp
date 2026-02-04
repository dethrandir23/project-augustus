#include "Company.h"
#include "../Economy/EconomyUtils.h"
#include "../Game/Gamestate.h"
#include "../Registry/PerkManager.h"
#include "../Registry/TechnologyManager.h"
#include "../Core/ModifierKeys.h"

// --- Storage Helpers ---
void Company::addItem(const std::string &itemId, float amount) {
  EconomyUtils::addToInventory(storage, itemId, amount);
}

float Company::getItemAmount(const std::string &itemId) const {
  return EconomyUtils::getItemAmount(storage, itemId);
}

void Company::removeFactoryRef(const uuids::uuid &fid) {
  factories.erase(std::remove(factories.begin(), factories.end(), fid),
                  factories.end());
}

  void Company::addTechnology(const std::string &techId) {
    knownTechnologies.push_back(techId);
  }

  const std::vector<std::string> &Company::getKnownTechnologies() const {
    return knownTechnologies;
  }

void Company::recruitWorkersFromNode(const uuids::uuid& nodeId, size_t count, Gamestate& gamestate) {
    // 1. Şehri Gamestate'den güvenle al
    TradeNode* node = gamestate.getTradeNode(nodeId);
    
    // Eğer şehir yoksa (yıkılmışsa vs.) işlem iptal
    if (!node) return;

    // 2. İşçileri çek
    size_t recruited = node->recruitWorkers(count);
    
    // 3. Şirkete ekle
    this->addManpower(recruited);
    
    double baseCost = EconomyManager::data.recruit_base;
    double costPerWorker = calculateModifier(ModifierKeys::Manpower::HIRE_COST, baseCost);
    double totalCost = recruited * costPerWorker;

    // 5. Para transferi
    this->capital -= totalCost;
    node->addCapital(totalCost);
}

  // 1. Perk Ekleme
void Company::addPerk(const std::string& perkId) {
    // Önce PerkManager'dan default süresini öğrenelim
    int duration = -1;
    if (PerkManager::perks.count(perkId)) {
        duration = PerkManager::perks.at(perkId).default_duration;
    }

    // Listeye ekle
    activePerks.push_back({perkId, duration});
}

void Company::tickPerks() {
    activePerks.erase(std::remove_if(activePerks.begin(), activePerks.end(),
        [](ActivePerk &p) {
            if (p.remainingDuration == -1) return false;

            p.remainingDuration--;

            return p.remainingDuration <= 0;
        }),
        activePerks.end());
}

void Company::removePerk(const std::string &perkID) {
    activePerks.erase(std::remove_if(activePerks.begin(), activePerks.end(),
        [perkID](ActivePerk &p) {
            return p.perkId == perkID;
        }),
        activePerks.end());
}

// --- Main Loop ---
void Company::manageFactories(Gamestate &gamestate) {

  // 1. Ölü fabrikaları temizle (Lambda ile)
  factories.erase(std::remove_if(factories.begin(), factories.end(),
                                 [&](const uuids::uuid &fid) {
                                   return gamestate.getFactory(fid) == nullptr;
                                 }),
                  factories.end());

  // 2. Her fabrika için işlem yap
  for (const auto &fid : factories) {
    Factory *factory = gamestate.getFactory(fid);
    if (!factory)
      continue;

    // A) İŞÇİ GÖNDER (Basit mantık: Herkese eşit dağıt)
    // (İleride burası Priority sistemine göre değişebilir)
    if (manpower > 0) {
      size_t added = factory->addEmployees(10); // Örnek: Her tick 10 işçi yolla
      removeManpower(added);
    }

    // B) HAMMADDE GÖNDER
    // Fabrikanın neye ihtiyacı var? (Factory artık listRequiredResources
    // döndürmüyor ama processPipelines içinde bakıyor. Burada manuel transfer
    // yapmamız lazım)
    // **NOT:** Modern sistemde fabrika direkt Company deposundan çekse daha
    // kolay olmaz mı? Ama şimdilik senin "transfer" mantığını koruyalım:

    auto missingInputs = factory->getMissingInputs();

    for (const auto &req : missingInputs) {
      // Şirketin deposunda bu maldan ne kadar var?
      float availableInCompany = getItemAmount(req.id);

      if (availableInCompany > 0) {
        // İhtiyaç kadar mı gönderelim, elde ne varsa onu mu?
        float amountToSend = std::min(availableInCompany, req.quantity);

        // Transfer işlemi:
        // 1. Şirketten düş (EconomyUtils kullan)
        EconomyUtils::removeFromInventory(storage, req.id, amountToSend);

        // 2. Fabrikaya ekle
        factory->addInput(req.id, amountToSend);
      }
    }

    factory->produce(1.0);

    auto outputs = factory->collectOutputs();
    for (const auto &item : outputs) {
      addItem(item.id, item.quantity);
    }
  }
}

float Company::calculateModifier(const std::string& modifierType, float baseValue) {
    float additive = 0.0f;
    float multiplicative = 1.0f;

    for(const auto& perkId : activePerks) {
        const auto& perk = PerkManager::perks.at(perkId.to_string());
        
        for(const auto& effect : perk.effects) {
            if(effect.type == modifierType) {
                if(effect.mode == "additive") additive += effect.value;
                else if(effect.mode == "multiplicative") multiplicative *= (1.0f + effect.value);
            }
        }
    }

    for(const auto& techId : knownTechnologies) {
        const auto& tech = TechnologyManager::techs.at(techId);
        for(const auto& effect : tech.effects) {
            if(effect.type == modifierType) {
                 if(effect.type == "additive") additive += effect.value;
                 else multiplicative *= (1.0f + effect.value);
            }
        }
    }

    // Sonuç: (Baz Değer + Eklemeler) * Çarpanlar
    return (baseValue + additive) * multiplicative;
}

void to_json(nlohmann::json &j, const Company &c) {
  j = nlohmann::json{{"id", uuids::to_string(c.id)},
                     {"name", c.name},
                     {"manpower", c.manpower},
                     {"capital", c.capital},
                     {"debt", c.debt},
                     {"techs", c.knownTechnologies},
                     {"perks", c.activePerks},
                     {"storage", c.storage},
                     // Factory ID'leri stringe çevir
                     {"factories", nlohmann::json::array()}};
  for (const auto &fid : c.factories) {
    j["factories"].push_back(uuids::to_string(fid));
  }
}