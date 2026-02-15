#include "TradeNode.h"
#include "World/Market.h"
#include <unordered_set>
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

void TradeNode::tick(Market& market, Gamestate& gamestate) {
    // 1. Yerel Üretim (Tarla süren köylüler)
    runLocalProduction();

    // 2. Tüketim ve Mutluluk Hesabı
    processConsumption(market);

    // 3. Nüfus Güncellemesi
    updateDemographics();

    // 4. Ticaret (Fazlalıkları Sat)
    manageBudget(market, gamestate);
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
        storage.add(item.id, item.quantity);
    }
}

void TradeNode::processConsumption(Market& market) {
    std::vector<PipelineData> needs;
    
    // 1. İhtiyaç Duyulan Pipeline'ları Yükle
    // Aynı zamanda toplam ihtiyaç listesini çıkar (Alışveriş listesi için)
    std::unordered_map<std::string, float> totalRequirements;

    for(const auto& pid : consumption_pipeline_ids) {
        if(PipelineManager::pipelines.count(pid)) {
            const auto& pipe = PipelineManager::pipelines.at(pid);
            needs.push_back(pipe);

            // Bu pipeline için gereken girdileri topla
            for(const auto& input : pipe.inputs) {
                totalRequirements[input.id] += input.quantity;
            }
        }
    }

    // 2. Eksikleri Belirle ve Marketten Al (Otomatik Tedarik)
    std::vector<ItemStack> shoppingList;
    
    for (const auto& [reqId, reqQty] : totalRequirements) {
        float currentStock = storage.getAmount(reqId);
        if (currentStock < reqQty) {
            // Depoda yeterince yok, eksik kadar al
            float deficit = reqQty - currentStock;
            shoppingList.push_back({reqId, deficit});
        }
    }

    if (!shoppingList.empty()) {
        // Market.h'daki buyItems fonksiyonunu çağırıyoruz
        // Bu işlem paramız yettiği kadarını depoya ekler
        market.buyItems(shoppingList, storage, capital);
    }

    // 3. Tüketimi Gerçekleştir
    // EconomyUtils inputs/outputs işlemini yapar (Yeterli materyal varsa siler, çıktı üretir)
    auto result = EconomyUtils::processProduction(storage, needs, 1.0);

    // 4. Mutluluk Hesaplama
    float totalHappinessProduced = 0.0f;
    
    for(const auto& item : result.producedItems) {
        if (item.id == "core_happiness_043") { // ID'yi kendi projenin ID'sine göre ayarla
            totalHappinessProduced += item.quantity;
        }
        // Public Order vs burada işlenebilir
    }

    // Varsayalım ki her 100 kişi için 1 birim mutluluk gerekiyor
    float requiredHappiness = population * 0.01f; 
    
    if (requiredHappiness > 0) {
        happinessPercentage = std::clamp(totalHappinessProduced / requiredHappiness, 0.0f, 1.0f);
    } else {
        happinessPercentage = 0.5f; 
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

// TradeNode.cpp

// İmza değişikliği: Gamestate referansı lazım! 
// (Header dosyasında da void manageBudget(Market& market, Gamestate& gamestate); yapmayı unutma)
void TradeNode::manageBudget(Market& market, Gamestate& gamestate) {
    
    // 1. Kritik İhtiyaçları Belirle (Yedekle)
    std::unordered_set<std::string> essentialItems;
    for(const auto& pid : consumption_pipeline_ids) {
        if(PipelineManager::pipelines.count(pid)) {
            for(const auto& input : PipelineManager::pipelines.at(pid).inputs) {
                essentialItems.insert(input.id);
            }
        }
    }

    // 2. Envanteri Gez ve Fazlalıkları Bul
    const auto& currentInventory = storage.getAllItems();
    
    for (const auto& item : currentInventory) {
        // Eğer bu ürün tüketim için gerekli değilse SAT
        if (essentialItems.find(item.id) == essentialItems.end()) {
            
            if (item.quantity > 0.001f) {
                // --- FİYAT BELİRLEME STRATEJİSİ (AI V1.0) ---
                double basePrice = 1.0;
                if (ItemManager::items.count(item.id)) {
                    basePrice = ItemManager::items.at(item.id).base_price;
                }
                
                // Strateji: Taban fiyatın %5 üstüne koy. (Kâr etmek istiyor)
                // İstersen market.getPrice(item.id) kullanıp piyasaya göre de verebilirsin.
                double askPrice = basePrice * 1.05; 

                // --- EMİR OLUŞTUR ---
                MarketOrder sellOrder(
                    this->getId(),    // Satıcı: BEN (Node)
                    item.id,          // Ürün
                    OrderType::SELL,  // Tip
                    askPrice,         // Fiyat
                    item.quantity     // Miktar
                );

                // --- EMRİ GÖNDER ---
                // Bu işlem "storage"dan malı anında düşecek (Escrow)
                market.placeOrder(gamestate, sellOrder);
            }
        } 
        // Kritik ürünler şimdilik cepte kalsın.
    }
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