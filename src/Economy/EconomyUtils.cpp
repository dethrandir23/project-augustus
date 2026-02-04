#include "EconomyUtils.h"
#include <algorithm> // std::find_if için

namespace EconomyUtils {

    // --- HELPER (Private) ---
    // Envanterdeki bir item'ı bulmak için yardımcı fonksiyon
    ItemStack* findStack(std::vector<ItemStack>& inv, const std::string& id) {
        for (auto& stack : inv) {
            if (stack.id == id) return &stack;
        }
        return nullptr;
    }

    // --- IMPLEMENTATION ---

    void addToInventory(std::vector<ItemStack> &inventory, const std::string& itemId, float qty) {
        ItemStack* stack = findStack(inventory, itemId);
        if (stack) {
            stack->quantity += qty;
        } else {
            inventory.push_back({itemId, qty});
        }
    }

    bool removeFromInventory(std::vector<ItemStack> &inventory, const std::string& itemId, float qty) {
        ItemStack* stack = findStack(inventory, itemId);
        if (stack && stack->quantity >= qty) {
            stack->quantity -= qty;
            
            // Eğer miktar 0'a düşerse vector'den silmeli miyiz?
            // Şimdilik 0 olarak kalsın, performans için sürekli resize yapmayalım.
            // İstersen buraya cleanup logic eklersin.
            return true;
        }
        return false;
    }

    float getItemAmount(const std::vector<ItemStack> &inventory, const std::string& itemId) {
        for (const auto& stack : inventory) {
            if (stack.id == itemId) return stack.quantity;
        }
        return 0.0f;
    }

    ProductionResult processProduction(
        std::vector<ItemStack> &inventory, 
        const std::vector<PipelineData> &pipelines, 
        double globalEfficiency
    ) {
        ProductionResult result;

        for (const auto& pipe : pipelines) {
            
            // ADIM 1: Girdiler Yeterli mi Kontrolü
            bool canProduce = true;
            
            // PipelineData içindeki "inputs" artık vector<ItemStack> (PipelineManager'da öyle yapmıştık)
            for (const auto& input : pipe.inputs) {
                float currentAmount = getItemAmount(inventory, input.id);
                if (currentAmount < input.quantity) {
                    canProduce = false;
                    break; 
                }
            }

            // ADIM 2: Tüketim ve Üretim
            if (canProduce) {
                // A) Tüket
                for (const auto& input : pipe.inputs) {
                    removeFromInventory(inventory, input.id, input.quantity);
                }

                // B) Üret
                for (const auto& output : pipe.outputs) {
                    float producedQty = output.quantity * globalEfficiency;
                    
                    // Sonuç listesine ekle
                    result.producedItems.push_back({output.id, producedQty});
                }
            }
        }

        return result;
    }

} // namespace