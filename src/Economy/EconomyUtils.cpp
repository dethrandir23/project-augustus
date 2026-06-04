#include "EconomyUtils.h"
#include <algorithm>
#include "Core/GameConstants.h"
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

static const std::string NONE_ITEM = "core_none_000";

ProductionResult processProduction(
        Inventory &inventory,
        const std::vector<PipelineData> &pipelines,
        double globalEfficiency,
        double availableLabor
    ) {
        ProductionResult result;
        if (pipelines.empty()) return result;

        // ------------------------------------------------------------------
        // ADIM 1: Her item için toplam talebi hesapla
        // ------------------------------------------------------------------
        std::unordered_map<std::string, double> totalDemand;
        for (const auto& pipe : pipelines) {
            for (const auto& input : pipe.inputs) {
                if (input.id == NONE_ITEM) continue;
                totalDemand[input.id] += static_cast<double>(input.quantity);
            }
        }

        // ------------------------------------------------------------------
        // ADIM 2: Her pipeline için kaynak bottleneck'ini hesapla
        //         "laborOnly" = hiç gerçek input'u olmayan (extraction, agriculture)
        // ------------------------------------------------------------------
        struct PipeScale {
            double resourceScale;
            double laborCost;
            bool laborOnly; // sadece işgücü ile sınırlı (no real inputs)
        };
        std::vector<PipeScale> scales(pipelines.size());

        for (size_t i = 0; i < pipelines.size(); ++i) {
            const auto& pipe = pipelines[i];
            double minScale = std::numeric_limits<double>::infinity();
            bool hasRealInput = false;

            for (const auto& input : pipe.inputs) {
                if (input.id == NONE_ITEM) continue;
                hasRealInput = true;
                if (totalDemand[input.id] <= 0.0) {
                    minScale = 0.0;
                    break;
                }
                double share = static_cast<double>(input.quantity) / totalDemand[input.id];
                double alloc = inventory.getAmount(input.id) * share;
                double inputScale = alloc / static_cast<double>(input.quantity);
                minScale = std::min(minScale, inputScale);
            }

            scales[i].laborOnly = !hasRealInput;
            scales[i].resourceScale = hasRealInput ? minScale : std::numeric_limits<double>::infinity();
            scales[i].laborCost = static_cast<double>(pipe.laborCost);
        }

        // ------------------------------------------------------------------
        // ADIM 3: Labor cap
        //         Önce resource-constrained pipeline'ların işgücünü hesapla,
        //         kalan işgücünü laborOnly pipeline'lara dağıt
        // ------------------------------------------------------------------
        double resourceLaborDemand = 0.0;
        double totalLaborOnlyCost = 0.0;
        for (size_t i = 0; i < pipelines.size(); ++i) {
            if (scales[i].laborOnly) {
                totalLaborOnlyCost += scales[i].laborCost;
            } else {
                resourceLaborDemand += scales[i].laborCost * scales[i].resourceScale;
            }
        }

        double laborRatio = 1.0;
        double laborOnlyScale = 0.0;
        if (availableLabor >= 0.0) {
            if (resourceLaborDemand > availableLabor && resourceLaborDemand > 0.0) {
                // Resource-constrained pipelines zaten tüm işgücünü yiyor
                laborRatio = availableLabor / resourceLaborDemand;
                laborOnlyScale = 0.0;
            } else {
                double remainingLabor = availableLabor - resourceLaborDemand;
                if (totalLaborOnlyCost > 0.0 && remainingLabor > 0.0) {
                    laborOnlyScale = remainingLabor / totalLaborOnlyCost;
                } else {
                    laborOnlyScale = 0.0;
                }
            }
        }

        // ------------------------------------------------------------------
        // ADIM 4: Üretimi gerçekleştir
        // ------------------------------------------------------------------
        for (size_t i = 0; i < pipelines.size(); ++i) {
            const auto& pipe = pipelines[i];
            double finalScale;
            if (scales[i].laborOnly) {
                finalScale = laborOnlyScale * globalEfficiency;
            } else {
                finalScale = scales[i].resourceScale * laborRatio * globalEfficiency;
            }
            if (finalScale <= 0.0) continue;

            // Tüket
            for (const auto& input : pipe.inputs) {
                if (input.id == NONE_ITEM) continue;
                double amount = static_cast<double>(input.quantity) * finalScale;
                inventory.remove(input.id, static_cast<float>(amount));
            }

            // Üret
            for (const auto& output : pipe.outputs) {
                double producedQty = static_cast<double>(output.quantity) * finalScale;
                result.producedItems.push_back({output.id, static_cast<float>(producedQty)});
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

    if (workerEfficiency <= GameConstants::MIN_WORKER_EFFICIENCY) return;

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
    double factoryLabor = static_cast<double>(workComp->currentWorkers) * GameConstants::LABOR_POINTS_PER_WORKER;
    auto result = processProduction(
        inputInv->GetInternalInventory(), // Mutlaka referans olmalı
        activePipelines, 
        totalEfficiency,
        factoryLabor
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
                float needed = input.quantity * GameConstants::INPUT_BUFFER_MULTIPLIER;
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


void executeProduction(Entity& entity, const std::string& prodKey, const std::string& invKey, double availableLabor) {
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
    auto result = processProduction(inv->GetInternalInventory(), activePipelines, prod->efficiency, availableLabor);

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