#pragma once

#include "Core/Inventory.h"
#include "Core/Types.h"
#include <vector>
#include <functional>
#include <string>

namespace TradeUtils {

    // Fiyatı sorgulamak için kullanacağımız fonksiyon tipi.
    // Örn: [](const std::string& id) { return market.getPrice(id); }
    using PriceCalculator = std::function<double(const std::string& itemId)>;

    /**
     * @brief İki taraf arasında güvenli ticaret işlemi gerçekleştirir.
     * * @param sourceInv     Satıcının deposu (Eşya çıkar)
     * @param sourceWallet  Satıcının cüzdanı (Para girer)
     * @param targetInv     Alıcının deposu (Eşya girer)
     * @param targetWallet  Alıcının cüzdanı (Para çıkar)
     * @param items         Transfer edilecek eşyalar ve miktarları
     * @param priceFunc     Her bir eşyanın birim fiyatını döndüren fonksiyon
     * @return true         İşlem başarılıysa (Yeterli stok ve para varsa)
     * @return false        Yetersiz bakiye veya yetersiz stok varsa (İşlem iptal edilir)
     */
    bool performTrade(
        Inventory& sourceInv, 
        double& sourceWallet, 
        Inventory& targetInv, 
        double& targetWallet, 
        const std::vector<ItemStack>& items,
        PriceCalculator priceFunc
    );

    /**
     * @brief Sadece maliyet hesaplar, işlem yapmaz. (UI'da "Toplam Tutar" göstermek için ideal)
     */
    double calculateTotalCost(const std::vector<ItemStack>& items, PriceCalculator priceFunc);

    /**
     * @brief Basit eşya transferi (Para dönmez, sadece mal değişimi - Looting vs için)
     */
    bool transferItems(Inventory& source, Inventory& target, const std::vector<ItemStack>& items);

}