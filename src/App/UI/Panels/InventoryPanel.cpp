#include "InventoryPanel.h"
#include "Core/Inventory.h"
#include "Registry/ItemManager.h"
#include "Economy/Company.h"
#include "imgui.h"
#include "Icons/IconsFontAwesome6.h"

namespace UI::Panels {
    void InventoryPanel(Gamestate &gamestate) {
        // Güvenlik: Oyuncu şirketi var mı?
        Company* player = gamestate.getCompany(gamestate.getPlayerCompanyId());
        if (!player) {
            ImGui::TextColored(ImVec4(1,0,0,1), "No active player company.");
            return;
        }

        const auto& inventory = player->getStorage();
        auto items = inventory.getAllItems();

        if (items.empty()) {
            ImGui::TextDisabled("Inventory is empty.");
            return;
        }

        // --- TABLO TASARIMI ---
        // 2 Sütun: İsim, Miktar
        // Flags: Resizable (boyutlanabilir), Borders (çizgili), RowBg (satır renklendirme)
        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg;

        if (ImGui::BeginTable("InventoryTable", 2, flags)) {
            
            // Başlıklar
            ImGui::TableSetupColumn("Item Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            for (const auto& item : items) {
                ImGui::TableNextRow();
                
                // Item ID'den Item Tanımını çekelim (İsim ve belki ikon için)
                std::string displayName = item.id; // Fallback
                
                // Eğer ItemManager'da varsa gerçek adını al
                if (ItemManager::items.count(item.id)) {
                    displayName = ItemManager::items.at(item.id).name;
                }

                // --- Sütun 1: İsim ---
                ImGui::TableNextColumn();
                // Simge + İsim (Kutu ikonu koydum, istersen item tipine göre değiştirebilirsin)
                ImGui::Text("%s %s", ICON_FA_BOX_OPEN, displayName.c_str());
                
                // Hover yapınca ID'yi göster (Debug için süper)
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("ID: %s", item.id.c_str());
                }

                // --- Sütun 2: Miktar ---
                ImGui::TableNextColumn();
                // Miktarı sağa yasla ve biraz renk ver
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.1f", item.quantity);
            }

            ImGui::EndTable();
        }
    }
}