#include "Factory.h"
#include "../Economy/EconomyUtils.h"
#include "../Registry/PipelineManager.h"
#include "../Game/IdUtils.h"
#include <algorithm>

// --- Constructor ---
Factory::Factory(const std::string& defId, uuids::uuid ownerId)
    : definitionId(defId), ownerId(ownerId), employeeCount(0) 
{
    this->id = IdUtils::generateUuid(); // Her fabrikanın unique ID'si olsun
    
    // Default ismini Manager'dan çekelim
    if (FactoryManager::factories.count(defId)) {
        this->customName = FactoryManager::factories.at(defId).name;
    } else {
        this->customName = "Unknown Factory";
    }
}

// --- Workforce ---
size_t Factory::addEmployees(size_t count) {
    // Kapasiteyi Manager'dan öğreniyoruz! Hardcode yok!
    int maxWorkers = 100; // Default
    if (FactoryManager::factories.count(definitionId)) {
        maxWorkers = FactoryManager::factories.at(definitionId).max_workers;
    }

    size_t space = (maxWorkers > employeeCount) ? (maxWorkers - employeeCount) : 0;
    size_t toAdd = std::min(count, space);
    
    employeeCount += toAdd;
    return toAdd;
}

void Factory::setEmployeeCount(size_t count) {
    // Basit setter, istenirse burada da max check yapılabilir
    employeeCount = count;
}

size_t Factory::getEmployeeCount() const {
    return employeeCount;
}

// --- Production Logic (The Brain) ---
void Factory::produce(double globalModifiers) {
    // 1. Fabrika Verisine Eriş
    if (FactoryManager::factories.find(definitionId) == FactoryManager::factories.end()) {
        return; // Tanımsız fabrika, işlem yapma
    }
    const auto& factoryData = FactoryManager::factories.at(definitionId);

    // 2. Verimlilik Hesapla (İşçi Oranı)
    double workerEfficiency = 0.0;
    if (factoryData.max_workers > 0) {
        workerEfficiency = static_cast<double>(employeeCount) / factoryData.max_workers;
    }

    // Eğer hiç işçi yoksa üretim durur
    if (workerEfficiency <= 0.001) return;

    // TODO: İleride buraya "Teknoloji Bonusu" da eklenecek.
    double totalEfficiency = workerEfficiency * globalModifiers;

    // 3. Reçeteleri Topla
    // Fabrikanın yapabildiği tüm işleri (pipelines) listele
    std::vector<PipelineData> activePipelines;
    for (const auto& pipeId : factoryData.pipeline_ids) {
        if (PipelineManager::pipelines.count(pipeId)) {
            activePipelines.push_back(PipelineManager::pipelines.at(pipeId));
        }
    }

    // 4. EconomyUtils ile Üretimi Gerçekleştir
    // Bu fonksiyon inputStorage'dan düşecek, sonucu result'a koyacak.
    auto result = EconomyUtils::processProduction(
        inputStorage, 
        activePipelines, 
        totalEfficiency
    );

    // 5. Çıktıları Depoya Ekle
    for (const auto& producedItem : result.producedItems) {
        outputStorage.add(producedItem.id, producedItem.quantity);
    }
}

// --- Inventory ---
void Factory::addInput(const std::string& itemId, float amount) {
    inputStorage.add(itemId, amount);
}

std::vector<ItemStack> Factory::collectOutputs() {
    std::vector<ItemStack> collected = outputStorage.getAllItems();
    outputStorage.clear();
    return collected;
}

std::vector<ItemStack> Factory::getMissingInputs() const {
    std::vector<ItemStack> missing;
    
    // FactoryManager'dan veriyi çek
    if (FactoryManager::factories.find(definitionId) == FactoryManager::factories.end()) 
        return missing;
        
    const auto& fData = FactoryManager::factories.at(definitionId);

    // Her pipeline için gereksinimleri topla
    // (Basitleştirilmiş: Tüm pipeline'lar aynı anda çalışıyor varsayıyoruz)
    for (const auto& pipeId : fData.pipeline_ids) {
        if (!PipelineManager::pipelines.count(pipeId)) continue;
        const auto& pipe = PipelineManager::pipelines.at(pipeId);

        for (const auto& input : pipe.inputs) {
            // Şu an depoda ne kadar var?
            float currentQty = inputStorage.getAmount(input.id);
            
            // Gereken miktar (Örn: Pipeline input'u kadar)
            // Not: Burada 'input.quantity' birim başınadır, bunu işçi sayısına göre scale edebilirsin
            // ama şimdilik "1 birim üretim için gereken" diyelim.
            float needed = input.quantity * 5; // Örn: 5 turluk stok tutmaya çalışsın (Buffer)
            
            if (currentQty < needed) {
                // Listede zaten varsa üzerine ekle, yoksa yeni oluştur
                bool found = false;
                for(auto& m : missing) {
                    if(m.id == input.id) { m.quantity += (needed - currentQty); found = true; break; }
                }
                if(!found) missing.push_back({input.id, needed - currentQty});
            }
        }
    }
    return missing;
}

// --- Serialization ---
void to_json(nlohmann::json& j, const Factory& f) {
    j = nlohmann::json{
        {"id", uuids::to_string(f.id)},
        {"ownerId", uuids::to_string(f.ownerId)},
        {"definitionId", f.definitionId},
        {"name", f.customName},
        {"employeeCount", f.employeeCount},
        {"inputs", f.inputStorage},   // ItemStack to_json otomatiktir
        {"outputs", f.outputStorage}
    };
}