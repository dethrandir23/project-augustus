// MarketWindow.cpp
#include "Windows/MarketWindow.h"
#include "Icons/IconsFontAwesome6.h"
#include "Panels/MarketPanel.h"
#include <imgui.h>

namespace UI::Windows {
void MarketWindow(Gamestate &gamestate) {
  if (ImGui::Begin(ICON_FA_SHOP " Global Markets", nullptr)) {
    UI::Panels::MarketPanel(gamestate);
  }
  ImGui::End();
}
} // namespace UI::Windows