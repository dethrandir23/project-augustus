#include "GameControlsWindow.h"
#include "App/UI/Panels/GameControls.h"
#include "Game/Gamestate.h"
#include "imgui.h"

namespace UI::Windows {
void GameControlsWindow(Gamestate &gamestate) {
  if (ImGui::Begin("Game Controls")) {
    Panels::GameControls(gamestate);
  }
  ImGui::End();
}
} // namespace UI::Windows