// CompanyInfoPanel.cpp
#include "Game/Gamestate.h"
#include "imgui.h"

#include "Icons/IconsFontAwesome6.h"

extern ImFont *fontNormal;
extern ImFont *fontBig;

namespace UI::Panels {
void CompanyInfoPanel(Gamestate &gamestate) {
  auto &companies = gamestate.getCompanies();
  auto playerId = gamestate.getPlayerCompanyId();

  auto it = companies.find(playerId);
  if (it == companies.end()) {
    ImGui::TextColored(ImVec4(1, 0, 0, 1),
                       "Player company missing!");
    return;
  }

  Company &playerCompany = it->second;

  if (fontBig)
    ImGui::PushFont(fontBig);
  ImGui::Text("%s %s", ICON_FA_INDUSTRY, playerCompany.getName().c_str());
  if (fontBig)
    ImGui::PopFont();

  ImGui::Separator();
  ImGui::Spacing();

  if (ImGui::BeginTable("company_stats", 2)) {

    ImGui::TableNextColumn();
    ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.2f, 1.0f),
                       "%s Capital:", ICON_FA_COINS);
    ImGui::TableNextColumn();
    ImGui::Text("$ %.2f", playerCompany.getCapital());

    ImGui::TableNextColumn();
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f),
                       "%s Manpower:", ICON_FA_USERS);
    ImGui::TableNextColumn();
    ImGui::Text("%zu workers", playerCompany.getManpower());

    ImGui::TableNextColumn();
    ImGui::Text("%s Debt:", ICON_FA_HAND_HOLDING_DOLLAR);
    ImGui::TableNextColumn();

    float debt = playerCompany.getDebt();
    if (debt > 0) {
      ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "-$ %.2f", debt);
    } else {
      ImGui::TextDisabled("No Debt");
    }

    ImGui::EndTable();
  }
}
} // namespace UI::Panels