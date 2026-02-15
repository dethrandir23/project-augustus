// MarketDropdown.cpp
#include "Widgets/MarketDropdown.h"
#include "Game/Gamestate.h"
#include "World/Market.h"
#include "Icons/IconsFontAwesome6.h"
#include <imgui.h>

namespace UI::Widgets {
    std::optional<uuids::uuid> MarketDropdown(Gamestate &gamestate) {
        static std::optional<uuids::uuid> selectedID = std::nullopt;
        
        auto& markets = gamestate.getMarkets();

        if (selectedID.has_value() && !markets.count(selectedID.value())) {
            selectedID = std::nullopt;
        }

        std::string previewValue = "Select Market...";
        if (selectedID.has_value()) {
            previewValue = markets.at(selectedID.value()).getName();
        }

        ImGui::PushItemWidth(200);
        if (ImGui::BeginCombo("##market_selector", previewValue.c_str())) {
            
            for (const auto& [id, market] : markets) {
                const bool is_selected = (selectedID == id);
                
                std::string label = std::string(ICON_FA_SHOP) + " " + market.getName();

                if (ImGui::Selectable(label.c_str(), is_selected)) {
                    selectedID = id;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();

        return selectedID;
    }
}