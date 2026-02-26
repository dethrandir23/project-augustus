#include "EconomyUtils.h"
#include <algorithm>
#include "Core/Components/InventoryComponent.h"
#include "Core/ECS/Entity.h"
#include "Core/Inventory.h"
#include "Core/Types.h"
#include "Economy/Components/PerkComponent.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "Registry/FactoryManager.h"
#include "Registry/PipelineManager.h"
#include "Registry/PerkManager.h"
#include "Registry/TechnologyManager.h"
#include "Economy/Components/TechTreeComponent.h"
#include "uuid/uuid.h"

namespace EconomyUtils {

ProductionResult processProduction(
        Inventory &inventory, // Tüketim buradan yapılacak
        const std::vector<PipelineData> &pipelines, 
        double globalEfficiency
    ) {
        ProductionResult result;

        for (const auto& pipe : pipelines) {
            
            // ADIM 1: Girdiler Yeterli mi Kontrolü (Transaction Safety)
            bool canProduce = true;
            
            for (const auto& input : pipe.inputs) {
                // Eskiden: getItemAmount(vec, id)
                // Şimdi: inventory.has(id, amount)
                if (!inventory.has(input.id, input.quantity)) {
                    canProduce = false;
                    break; 
                }
            }

            // ADIM 2: Tüketim ve Üretim
            if (canProduce) {
                // A) Tüket (Inventory sınıfı kendi halleder)
                for (const auto& input : pipe.inputs) {
                    inventory.remove(input.id, input.quantity);
                }

                // B) Üret
                for (const auto& output : pipe.outputs) {
                    float producedQty = output.quantity * globalEfficiency;
                    
                    // Sonuç listesine ekle
                    result.producedItems.push_back({output.id, producedQty});
                }
            }
        }

        return result;
    }

void produce(Entity &f, double globalModifiers) {
    // 1. İhtiyacımız olan BÜTÜN parçaları (componentleri) Entity'den çekiyoruz
    auto* prodComp = f.GetComponent<ProductionComponent>("ProductionComponent");
    auto* workComp = f.GetComponent<WorkforceComponent>("WorkforceComponent");
    auto* inputInv = f.GetComponent<InventoryComponent>("inputStorage");
    auto* outputInv = f.GetComponent<InventoryComponent>("outputStorage");

    // Eğer bu parçalardan biri bile eksikse, bu fabrika çalışamaz! (Güvenlik kilidi)
    if (!prodComp || !workComp || !inputInv || !outputInv) return;

    // 2. FactoryData'yı (Template) bul
    if (FactoryManager::factories.find(prodComp->templateId) == FactoryManager::factories.end()) return;
    const auto& fData = FactoryManager::factories.at(prodComp->templateId);

    // 3. Verimlilik Hesabı (Güncel işçi / Maksimum işçi)
    double workerEfficiency = 0.0;
    if (fData.max_workers > 0) {
        // HATA DÜZELTİLDİ: Artık güncel işçi sayısını Workforce'dan alıyoruz!
        workerEfficiency = static_cast<double>(workComp->currentWorkers) / fData.max_workers; 
    }

    if (workerEfficiency <= 0.001) return;

    double totalEfficiency = workerEfficiency * globalModifiers;

    // 4. Reçeteleri Topla (Artık Template'ten değil, prodComp'un kendi listesinden alıyoruz)
    std::vector<PipelineData> activePipelines;
    for (const auto& pipeId : prodComp->activePipelineIds) { // <-- DİKKAT: prodComp
        if (PipelineManager::pipelines.count(pipeId)) {
            activePipelines.push_back(PipelineManager::pipelines.at(pipeId));
        }
    }

    // 5. Üretimi Gerçekleştir
    // NOT: processProduction inventory'i değiştireceği için referans yollamalıyız.
    // InventoryComponent içinde `Inventory& GetInventoryRef()` gibi bir fonksiyonun olmalı!
    auto result = processProduction(
        inputInv->GetInternalInventory(), // Mutlaka referans olmalı
        activePipelines, 
        totalEfficiency
    );

    // 6. Çıktıları Ekle
    for (const auto& producedItem : result.producedItems) {
        outputInv->Add(producedItem.id, producedItem.quantity);
    }
}

    void addInput(Entity &f, const std::string& itemId, float amount) {
        auto invComp = f.GetComponent<InventoryComponent>("inputStorage");
        if (invComp) {
            invComp->Add(itemId, amount);
        }
    }

    std::vector<ItemStack> collectOutputs(Entity &f) {
        auto invComp = f.GetComponent<InventoryComponent>("outputStorage");
        if (invComp) {
            std::vector<ItemStack> collected = invComp->GetAllItems();
            invComp->Clear();
            return collected;
        }
        return {};
    }

    // Returns missing items for pipelines in an inventory
    std::vector<ItemStack> getMissingItems(const std::string &definitionId, const Inventory &storage) {
        std::vector<ItemStack> missing;
        if (FactoryManager::factories.find(definitionId) == FactoryManager::factories.end())
            return missing;

        const auto& fData = FactoryManager::factories.at(definitionId);

        for (const auto& pipeId : fData.pipeline_ids) {
            if (PipelineManager::pipelines.find(pipeId) == PipelineManager::pipelines.end())
                continue;
            const auto& pipe = PipelineManager::pipelines.at(pipeId);

            for (const auto& input : pipe.inputs) {
                float currentQty = storage.getAmount(input.id);
                float needed = input.quantity * 5; // Örn: 5 turluk stok tutmaya çalışsın (Buffer)
                if (currentQty < needed) {
                    bool found = false;
                    for (auto& m : missing) {
                        if (m.id == input.id) {
                            m.quantity += (needed - currentQty);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        missing.push_back({input.id, needed - currentQty});
                    }
                }
            }
        }
        return missing;
    
    }


void executeProduction(Entity& entity, const std::string& prodKey, const std::string& invKey) {
    auto* prod = entity.GetComponent<ProductionComponent>(prodKey);
    auto* inv = entity.GetComponent<InventoryComponent>(invKey);
    
    if (!prod || !inv) return;

    // Aktif pipelineları çek
    std::vector<PipelineData> activePipelines;
    for (const auto& pipeId : prod->activePipelineIds) {
        if (PipelineManager::pipelines.count(pipeId)) {
            activePipelines.push_back(PipelineManager::pipelines.at(pipeId));
        }
    }

    // Üret (Girdi de çıktı da aynı depo: invKey)
    auto result = processProduction(inv->GetInternalInventory(), activePipelines, prod->efficiency);

    // Çıktıları aynı depoya geri koy
    for (const auto& item : result.producedItems) {
        inv->Add(item.id, item.quantity);
    }
}

float calculateEntityModifier(Entity& entity, const std::string& modifierType, float baseValue) {
    float additive = 0.0f;
    float multiplicative = 1.0f;

    // 1. Perkleri kontrol et
    auto* perks = entity.GetComponent<PerkComponent>("PerkComponent");
    if (perks) {
        for(const auto& perkData : perks->activePerks) {
            const auto& perk = PerkManager::perks.at(perkData.perkId); // ID string'e çevrilmiş varsayıyorum
            for(const auto& effect : perk.effects) {
                if(effect.type == modifierType) {
                    if(effect.mode == "additive") additive += effect.value;
                    else if(effect.mode == "multiplicative") multiplicative *= (1.0f + effect.value);
                }
            }
        }
    }

    // 2. Teknolojileri kontrol et
    auto* techs = entity.GetComponent<TechTreeComponent>("TechTreeComponent");
    if (techs) {
        for(const auto& techId : techs->knownTechnologies) {
            const auto& tech = TechnologyManager::techs.at(techId);
            for(const auto& effect : tech.effects) {
                if(effect.type == modifierType) {
                     if(effect.type == "additive") additive += effect.value;
                     else multiplicative *= (1.0f + effect.value);
                }
            }
        }
    }

    return (baseValue + additive) * multiplicative;
}

} // namespace