// src/AI/Logic/CompanyLogic.h
#pragma once
#include "Core/ECS/Entity.h"
#include "Game/Gamestate.h"
#include "AI/Evaluators/EconomyEvaluator.h"
#include "AI/AIUtils/AIPicker.h" // Senin attığın kodun olduğu util
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Game/EntityFactory.h" // createFactory için
#include "DevTools/Console.h"
#include <vector>
#include <iostream>

namespace CompanyLogic {

    // 1. MANTIK: Borç Yönetimi (Acil Durum)
    inline void manageDebt(Entity& company, Gamestate& gamestate) {
        auto* wallet = company.GetComponent<WalletComponent>("WalletComponent");
        if (!wallet) return;

        // Eğer borcum var ve ödeyecek kadar param varsa kapat kurtul.
        if (wallet->debt > 0 && wallet->balance >= wallet->debt) {
            wallet->balance -= wallet->debt;
            wallet->debt = 0;
            Console::log("[AI] " + company.GetName() + " tum borclarini odedi.", LogType::INFO);
        }
    }

    // 2. MANTIK: Büyüme ve Yatırım (Fırsat Avcısı)
    inline void buildFactories(Entity& company, Gamestate& gamestate) {
        auto* wallet = company.GetComponent<WalletComponent>("WalletComponent");
        if (!wallet) return;

        // A. Yatırım yapacak havamda mıyım? (Param var mı?)
        float investDesire = EconomyEvaluator::scoreInvestmentDesire(company);
        if (investDesire <= 0.0f) return; // Fakirim, yatırım yok.

        // B. Masadaki seçenekleri değerlendir (Skorlama)
        std::vector<AIPicker::Candidate> candidates;
        
        for (const auto& [templateId, fData] : FactoryManager::factories) {
            float profitScore = EconomyEvaluator::scoreFactoryProfitability(templateId, gamestate);
            
            // Sadece kârlı (Score > 0) olan fabrikaları listeye al
            if (profitScore > 0.5f) {
                // Burada istersen AIBrainComponent'teki dynamicDesires'ı profitScore'a ekleyebilirsin
                candidates.push_back({templateId, profitScore});
            }
        }

        // C. Seçim Yap! (Senin TOML Config'i ve Softmax'ı kullanıyoruz)
        // Not: İleride ConfigManager'dan çekeceksin: ConfigManager::getFloat("temperature.balanced");
        double temperature = 0.7; // Orta risk
        int topK = 3; // En karlı ilk 3 seçenek arasından seç

        std::string chosenFactoryId = AIPicker::selectWithNoise(candidates, topK, temperature);

        // D. Eyleme Geç (İnşa Et)
        if (!chosenFactoryId.empty()) {
            
            // TODO: Şimdilik sabit maliyet 5000 diyoruz, ileride fData.build_cost eklersin
            double buildCost = 5000.0; 
            
            if (wallet->balance >= buildCost) {
                wallet->balance -= buildCost; // Parayı kes
                
                // Fabrikayı yarat
                const auto& fData = FactoryManager::factories.at(chosenFactoryId);
                Entity* newFactory = EntityBuilder::createFactory(fData, company.GetId(), ""); // İsim oto üretilir
                
                // Dünyaya ve şirkete ekle
                gamestate.addEntity(newFactory);
                
                auto* assets = company.GetComponent<AssetOwnerComponent>("AssetOwnerComponent");
                if (assets) {
                    assets->addAsset(newFactory->GetId());
                }

                Console::log("[AI] " + company.GetName() + " yeni bir " + fData.name + " insa etti! Beklenen Kar: " + std::to_string(candidates[0].prob), LogType::INFO);
            }
        }
    }
}