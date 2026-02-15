#include "InputBar.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "Icons/IconsFontAwesome6.h"

namespace UI::Widgets {
bool InputBar(const char *label, std::string &buffer) {
    bool pressed = false;
    
    // Buton genişliğini hesapla (Metin genişliği + padding)
    float buttonWidth = 80.0f; 
    
    // InputText genişliği = Pencere genişliği - Buton genişliği - Aradaki boşluk
    float inputWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;

    ImGui::PushItemWidth(inputWidth);
    
    // EnterReturnsTrue: Enter'a basınca true döner
    if (ImGui::InputText("##console_input", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
        pressed = true;
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();

    // Buton (İkonlu gönder butonu)
    if (ImGui::Button(ICON_FA_PAPER_PLANE " Send", ImVec2(buttonWidth, 0))) {
        pressed = true;
    }

    // Input alanına otomatik odaklanma (Opsiyonel: Her enter'dan sonra tekrar odaklar)
    if (pressed) {
        ImGui::SetKeyboardFocusHere(-1); // Önceki elemana (InputText) odaklan
    }

    return pressed;
}
} // namespace UI::Widgets