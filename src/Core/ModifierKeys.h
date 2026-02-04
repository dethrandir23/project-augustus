#pragma once

/**
 * @file ModifierKeys.h
 * @brief Oyun motorunun tanıdığı tüm modifier (etki) anahtarları burada tanımlanır.
 * Modcular JSON'a buradaki string değerlerini yazmalıdır.
 */

namespace ModifierKeys {

    // --- EKONOMİ (Economy) ---
    namespace Economy {
        // Borç faiz oranı çarpanı (Default: 1.0)
        constexpr const char* DEBT_INTEREST_RATE = "debt_interest_rate";
        
        // Vergi oranı çarpanı (Default: 1.0) - İleride eklersin
        constexpr const char* TAX_RATE = "tax_rate";
        
        // Kredi çekme limiti çarpanı
        constexpr const char* LOAN_LIMIT = "loan_limit";
    }

    // --- ÜRETİM (Production) ---
    namespace Production {
        // Genel üretim verimliliği (Default: 1.0)
        // DİKKAT: "effiency" değil "efficiency" (İngilizce doğrusu) ;)
        constexpr const char* EFFICIENCY = "production_efficiency";
        
        // Üretim maliyeti (Girdi harcama oranı)
        constexpr const char* INPUT_CONSUMPTION = "input_consumption";
    }

    // --- İŞ GÜCÜ (Manpower) ---
    namespace Manpower {
        // İşçi alma maliyeti
        constexpr const char* HIRE_COST = "hire_personal_cost";
        
        // İşçi maaşları çarpanı
        constexpr const char* SALARY_MULTIPLIER = "salary_multiplier";
        
        // Şirkete her tur gelen doğal iş gücü artışı
        constexpr const char* GROWTH_RATE = "manpower_growth";
    }

    // --- TİCARET (Trade) ---
    namespace Trade {
        // Satış yaparken kazanılan bonus gelir
        constexpr const char* SALES_PRICE_BONUS = "sales_price_bonus";
    }
}