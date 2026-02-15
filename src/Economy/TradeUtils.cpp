#include "TradeUtils.h"
#include <cmath>

namespace TradeUtils {

    double calculateTotalCost(const std::vector<ItemStack>& items, PriceCalculator priceFunc) {
        double total = 0.0;
        for (const auto& item : items) {
            double unitPrice = priceFunc(item.id);
            total += unitPrice * item.quantity;
        }
        return total;
    }

    bool performTrade(
        Inventory& sourceInv, 
        double& sourceWallet, 
        Inventory& targetInv, 
        double& targetWallet, 
        const std::vector<ItemStack>& items,
        PriceCalculator priceFunc
    ) {
        // 1. ADIM: VALIDATION (Kontrol)
        
        // A) Maliyet Kontrolü
        double totalCost = calculateTotalCost(items, priceFunc);
        if (targetWallet < totalCost) {
            return false; // Alıcının parası yetmiyor
        }

        // B) Stok Kontrolü
        // Alıcı hepsini istiyor ama satıcıda var mı?
        for (const auto& req : items) {
            if (!sourceInv.has(req.id, req.quantity)) {
                return false; // Satıcıda yeterli mal yok
            }
        }

        // 2. ADIM: EXECUTION (İşlem)
        // Buraya geldiysek her şey yolunda demektir.

        // Parayı transfer et
        targetWallet -= totalCost;
        sourceWallet += totalCost;

        // Eşyaları transfer et
        for (const auto& req : items) {
            sourceInv.remove(req.id, req.quantity); // Satıcıdan sil
            targetInv.add(req.id, req.quantity);    // Alıcıya ekle
        }

        return true;
    }

    bool transferItems(Inventory& source, Inventory& target, const std::vector<ItemStack>& items) {
        // Önce stok kontrolü
        for (const auto& req : items) {
            if (!source.has(req.id, req.quantity)) return false;
        }

        // Transfer
        for (const auto& req : items) {
            source.remove(req.id, req.quantity);
            target.add(req.id, req.quantity);
        }
        return true;
    }
} // namespace TradeUtils