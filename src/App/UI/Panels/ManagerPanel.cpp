#include "Game/Gamestate.h"
#include "UIManager.h" // UIManager'a erişmemiz lazım
#include "imgui.h"
#include "App/UI/UIManager.h"
#include "Icons/IconsFontAwesome6.h" // İkonlar için

namespace UI::Panels {

void UIManagerPanel(Gamestate &gamestate) {
    auto &windows = UIManager::getWindows();

    // Search Bar
    static char searchBuffer[128] = "";
    ImGui::InputText(ICON_FA_MAGNIFYING_GLASS " Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Clear")) { searchBuffer[0] = '\0'; }

    ImGui::Separator();

    if (ImGui::BeginTable("WinManagerTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Window Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Visible", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (auto &pair : windows) {
            auto &win = pair.second;
            
            // Arama filtresi
            if (searchBuffer[0] != '\0' && win.title.find(searchBuffer) == std::string::npos) continue;

            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", win.title.c_str());

            ImGui::TableNextColumn();
            ImGui::PushID(win.title.c_str());
            ImGui::Checkbox("##v", &win.isOpen);
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::PushID((win.title + "_btn").c_str());
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT)) {
                win.defaultPosX = 100; 
                win.defaultPosY = 100;
                win.forceReset = true; // UIManager bunu yakalayacak
                win.isOpen = true;
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    
    // Alt Butonlar
    if (ImGui::Button("Close All")) {
        for (auto &pair : windows) {
            if (pair.second.title.find("UI Manager") == std::string::npos)
                pair.second.isOpen = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Open All")) {
        for (auto &pair : windows) pair.second.isOpen = true;
    }
}

} // namespace UI::Panels