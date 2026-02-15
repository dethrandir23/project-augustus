#include "CompanyInfoWindow.h"
#include "App/UI/Panels/CompanyInfoPanel.h"
#include "Game/Gamestate.h"
#include "imgui.h"

namespace UI::Windows {
void CompanyInfoWindow(Gamestate &gamestate) {
  ImGui::Begin("Company Info");
  UI::Panels::CompanyInfoPanel(gamestate);
  ImGui::End();
}
} // namespace UI::Windows