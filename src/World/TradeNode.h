#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Types.h"
#include "../Economy/EconomyUtils.h"
#include "../Registry/TradeNodeManager.h"
#include "../Registry/PipelineManager.h" 
#include "../Registry/NameManager.h"
#include "../Game/IdUtils.h"
#include "Core/Inventory.h"
#include <string>
#include <vector>
#include <algorithm>

// Market forward declaration (Çünkü henüz Market.h'ı yenilemedik)
class Market;
class Gamestate;

class TradeNode {
public:
    // Constructor: Sadece Template ID ve Market ID ile kurulur
    TradeNode(const std::string& templateId, uuids::uuid marketId);

    // --- Core Gameplay Loop (Her Tur Çağrılır) ---
    void tick(Market& market, Gamestate& gamestate);

    // --- Interaction ---
    /**
     * @brief Şirketler buradan işçi kiralar.
     * @param count İstenen işçi sayısı.
     * @return size_t Gerçekten alınabilen işçi sayısı.
     */
    size_t recruitWorkers(size_t count);

    // --- Getters / Setters ---
    uuids::uuid getId() const { return id; }
    uuids::uuid getMarketId() const { return marketId; }
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    size_t getPopulation() const { return population; }
    void setPopulation(size_t newPop) { population = newPop; }
    inline void addCapital(double amount) { capital += amount; }
    inline double getCapital() const { return capital; }
    double& getCapitalRef() { return capital; }
    inline void removeCapital(double amount) { capital = (capital > amount) ? (capital - amount) : 0; }
    
    // YENİ: Mutluluk Yüzdesi (0.0 = İsyan, 1.0 = Cennet)
    float getHappinessPercentage() const { return happinessPercentage; }
    
    // Depo Erişim (Company ile aynı mantık)
    Inventory& getStorage() { return storage; }


    // Serialization
    friend void to_json(nlohmann::json& j, const TradeNode& n);

private:
    // --- Helper Logic ---
    void runLocalProduction();      // Köylülerin kendi üretimi
    void processConsumption(Market& market); // Tüketim ve İhtiyaç Giderme
    void updateDemographics();      // Nüfus artışı/azalışı
    void manageBudget(Market& market, Gamestate& gamestate); // Fazlalık satışı

    // --- Data ---
    uuids::uuid id;
    uuids::uuid marketId;
    std::string templateId;
    std::string name;

    // Demographics
    size_t population = 0;
    size_t recruitable_pop = 0; // Şirketlerin alabileceği boşta gezenler
    float happinessPercentage = 0.5f; // Başlangıçta nötr (%50)

    // Economy
    double capital = 0.0;
    
    // Storage (Resource + Product + Abstract Items hepsi burada)
    Inventory storage;
    
    // Cache Lists (Template'den yüklenir)
    std::vector<std::string> local_pipeline_ids;
    std::vector<std::string> consumption_pipeline_ids;
};