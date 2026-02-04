#include "TradeNode.h"
// #include "Market.h" // Market hazır olunca açacağız

TradeNode::TradeNode(const std::string& tId, uuids::uuid mId) 
    : templateId(tId), marketId(mId) 
{
    id = IdUtils::generateUuid();
    
    // Template Yükleme
    if (TradeNodeManager::templates.count(templateId)) {
        const auto& tmpl = TradeNodeManager::templates.at(templateId);
        
        name = NameManager::getRandomName(tmpl.name_pool_id);
        
        // Random Population
        size_t range = (tmpl.max_pop - tmpl.min_pop);
        population = tmpl.min_pop + (range > 0 ? rand() % range : 0);
        
        // %40'ı işe alınabilir olsun
        recruitable_pop = static_cast<size_t>(population * 0.4); 
        
        capital = tmpl.initial_capital;
        local_pipeline_ids = tmpl.local_pipelines;
        consumption_pipeline_ids = tmpl.consumption_pipelines;
    }
}

size_t TradeNode::recruitWorkers(size_t count) {
    size_t available = std::min(count, recruitable_pop);
    recruitable_pop -= available;
    return available;
}

void TradeNode::tick(Market& market) {
    // 1. Yerel Üretim (Tarla süren köylüler)
    runLocalProduction();

    // 2. Tüketim ve Mutluluk Hesabı
    processConsumption(market);

    // 3. Nüfus Güncellemesi
    updateDemographics();

    // 4. Ticaret (Fazlalıkları Sat)
    manageBudget(market);
}

void TradeNode::runLocalProduction() {
    // PipelineManager'dan yerel üretimleri bulup çalıştır
    std::vector<PipelineData> pipes;
    for(const auto& pid : local_pipeline_ids) {
        if(PipelineManager::pipelines.count(pid)) {
            pipes.push_back(PipelineManager::pipelines.at(pid));
        }
    }

    // Üretim verimliliği şimdilik %100 (1.0)
    // EconomyUtils inputs/outputs işlemini yapar
    auto result = EconomyUtils::processProduction(storage, pipes, 1.0);
    
    // Çıktıları depoya ekle
    for(const auto& item : result.producedItems) {
        EconomyUtils::addToInventory(storage, item.id, item.quantity);
    }
}

void TradeNode::processConsumption(Market& market) {
    std::vector<PipelineData> needs;
    for(const auto& pid : consumption_pipeline_ids) {
        if(PipelineManager::pipelines.count(pid)) {
            needs.push_back(PipelineManager::pipelines.at(pid));
        }
    }

    // TODO: Market'ten Eksikleri Satın Al (Market implemente edilince buraya gelecek)
    // calculateDeficits() -> market.buy() -> addToStorage()

    // Tüketimi gerçekleştir (Elimizde yemek varsa yenir, Happiness üretilir)
    // Not: Tüketimde verimlilik olmaz, o yüzden 1.0
    auto result = EconomyUtils::processProduction(storage, needs, 1.0);

    // Üretilen "Happiness" veya "Public Order" itemlarını topla
    // Bunlar envantere girmez, hemen hesaplanır ve silinir.
    float totalHappinessProduced = 0.0f;
    
    for(const auto& item : result.producedItems) {
        if (item.id == "core_happiness_043") {
            totalHappinessProduced += item.quantity;
        }
        // Diğer abstract itemlar (public order vs) burada işlenebilir
    }

    // --- MUTLULUK HESABI ---
    // Varsayalım ki her 100 kişi için 1 birim mutluluk (Happiness Item) gerekiyor.
    // Bu oranlar dengelenebilir.
    float requiredHappiness = population * 0.01f; 
    
    if (requiredHappiness > 0) {
        happinessPercentage = std::clamp(totalHappinessProduced / requiredHappiness, 0.0f, 1.0f);
    } else {
        happinessPercentage = 0.5f; // Nüfus yoksa nötr
    }
}

void TradeNode::updateDemographics() {
    // Mutluluk %50'nin üzerindeyse büyü, altındaysa küçül/göç ver.
    float growthFactor = (happinessPercentage - 0.5f) * 0.1f; // Max %5 büyüme/küçülme
    
    int change = static_cast<int>(population * growthFactor);
    
    if (change > 0) {
        population += change;
        // Yeni gelenlerin bir kısmı iş gücüne katılır
        recruitable_pop += static_cast<size_t>(change * 0.4);
    } else {
        // Nüfus düşerken underflow kontrolü
        size_t loss = static_cast<size_t>(-change);
        population = (population > loss) ? (population - loss) : 0;
        // İş gücü de azalır
        size_t workerLoss = static_cast<size_t>(loss * 0.4);
        recruitable_pop = (recruitable_pop > workerLoss) ? (recruitable_pop - workerLoss) : 0;
    }
}

void TradeNode::manageBudget(Market& market) {
    // TODO: Market implementasyonu sonrası burayı yazacağız.
    // Logic: Abstract olmayan ve Tüketim için gerekli olmayan her şeyi sat.
}

void to_json(nlohmann::json& j, const TradeNode& n) {
    j = nlohmann::json{
        {"id", uuids::to_string(n.id)},
        {"name", n.name},
        {"templateId", n.templateId},
        {"marketId", uuids::to_string(n.marketId)},
        {"population", n.population},
        {"happiness", n.happinessPercentage},
        {"capital", n.capital},
        {"storage", n.storage}
    };
}