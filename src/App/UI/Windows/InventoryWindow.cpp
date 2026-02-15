#include "Game/Gamestate.h"
#include "InventoryWindow.h"
#include "imgui.h"
#include "App/UI/Panels/InventoryPanel.h"
#include "Icons/IconsFontAwesome6.h"

namespace UI::Windows {
    void InventoryWindow(Gamestate &gamestate) {
        if (ImGui::Begin(ICON_FA_WAREHOUSE " Inventory", nullptr, ImGuiWindowFlags_None)) {
            Panels::InventoryPanel(gamestate);
        }
        ImGui::End();
    }
}