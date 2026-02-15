#include "EconomyUtils.h"
#include <algorithm> // std::find_if için

namespace EconomyUtils {

ProductionResult processProduction(
        Inventory &inventory, // Tüketim buradan yapılacak
        const std::vector<PipelineData> &pipelines, 
        double globalEfficiency
    ) {
        ProductionResult result;

        for (const auto& pipe : pipelines) {
            
            // ADIM 1: Girdiler Yeterli mi Kontrolü (Transaction Safety)
            bool canProduce = true;
            
            for (const auto& input : pipe.inputs) {
                // Eskiden: getItemAmount(vec, id)
                // Şimdi: inventory.has(id, amount)
                if (!inventory.has(input.id, input.quantity)) {
                    canProduce = false;
                    break; 
                }
            }

            // ADIM 2: Tüketim ve Üretim
            if (canProduce) {
                // A) Tüket (Inventory sınıfı kendi halleder)
                for (const auto& input : pipe.inputs) {
                    inventory.remove(input.id, input.quantity);
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