// MarketPanel.cpp
#include "Panels/MarketPanel.h"
#include "Widgets/MarketDropdown.h" // Dropdown'u çağıracağız
#include "Game/Gamestate.h"
#include "World/Market.h"
#include "Icons/IconsFontAwesome6.h"
#include <imgui.h>

namespace UI::Panels {
    void MarketPanel(Gamestate &gamestate) {
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s Market:", ICON_FA_EARTH_AMERICAS);
        ImGui::SameLine();
        
        auto selected = Widgets::MarketDropdown(gamestate);

        ImGui::Separator();
        ImGui::Spacing();

        if (!selected.has_value()) {
            ImGui::TextDisabled("Please select a market to view prices.");
            return;
        }

        Market &market = *gamestate.getMarket(selected.value());
        
        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable;

        if (ImGui::BeginTable("MarketPrices", 4, flags)) {
            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Demand", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Supply", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            for (const auto &[itemId, price] : market.getPrices()) {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%s %s", ICON_FA_BOX, itemId.c_str());

                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.2f $", price);

                // Demand (Buy Pressure) - Mavi
                ImGui::TableNextColumn();
                float buyP = market.getBuyPressure().count(itemId) ? market.getBuyPressure().at(itemId) : 0.0f;
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "%.0f", buyP);

                // Supply (Sell Pressure) - Yeşil
                ImGui::TableNextColumn();
                float sellP = market.getSellPressure().count(itemId) ? market.getSellPressure().at(itemId) : 0.0f;
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.0f", sellP);
            }

            ImGui::EndTable();
        }
    }
}