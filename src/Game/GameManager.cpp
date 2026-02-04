#include "GameManager.h"

void GameManager::stepGamestate(Gamestate &gamestate) {
    
    // 0. Tarihi İlerlet
    gamestate.advanceDate();

    // 1. Şirketler (Üretim & Lojistik)
    // Şirketler fabrikalarına hammadde taşır, üretim yaptırır, ürünleri toplar.
    for (auto& [id, company] : gamestate.getCompanies()) {
        company.manageFactories(gamestate);
    }

    // 2. Şehirler (Tüketim & Nüfus)
    // Şehirler marketlerine bakıp ihtiyaçlarını gidermeye çalışır.
    for (auto& [id, node] : gamestate.getNodes()) {
        Market* m = gamestate.getMarket(node.getMarketId());
        if (m) {
            node.tick(*m);
        }
    }

    // 3. Market Dinamikleri (Fiyatlar)
    // Arz/Talep oluştu, şimdi fiyatları güncelleme zamanı.
    for (auto& [id, market] : gamestate.getMarkets()) {
        market.calculateMarketPrices();
    }

    // 4. Finansal Bakım (Maaşlar, Vergiler)
    // Şimdilik placeholder, ileride Company::updateFinancials() eklenebilir.
    for (auto& [id, company] : gamestate.getCompanies()) {
        // company.payWages();
        // company.payDebtInterest();
    }
}