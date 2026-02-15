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
            node.tick(*m, gamestate);
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

    gamestate.getCompanies()[gamestate.getPlayerCompanyId()].addManpower(100);

    gamestate.getEventHandler().tickEvents(gamestate);
}

void GameManager::update(Gamestate &gamestate, float deltaTime) {
    if (gamestate.paused) return;

    gamestate.accumulator += deltaTime;
    float secondsPerTurn = 1.0f / static_cast<float>(gamestate.gameSpeed);

    while (gamestate.accumulator >= secondsPerTurn) {
        stepGamestate(gamestate);
        gamestate.accumulator -= secondsPerTurn;
    }
}