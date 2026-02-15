#include "GameControls.h"
#include "Game/GameManager.h"
#include "Game/Gamestate.h"
#include "Game/InputHandler.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Icons/IconsFontAwesome6.h" // İkonları unutma!

using json = nlohmann::json;

namespace {
const json stepGameJson = {{"type", "STEP_GAME"}, {"payload", {{"times", 1}}}};
} // namespace

namespace UI::Panels {
void GameControls(Gamestate &gamestate) {
  // --- 1. DURUM BİLGİSİ (Üst Kısım) ---
  // Tarih için Takvim İkonu, Tur için Kum Saati
  ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s Date: %02d.%02d.%d", 
      ICON_FA_CALENDAR_DAYS, 
      gamestate.getCurrentDay(),
      gamestate.getCurrentMonth(), 
      gamestate.getCurrentYear());

  ImGui::SameLine(0, 20); // Araya 20px boşluk koy

  ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s Turn: %d", 
      ICON_FA_HOURGLASS_HALF, 
      gamestate.getCurrentTurn());

  ImGui::Separator();
  ImGui::Spacing();

  // --- 2. KONTROLLER (Alt Kısım) ---
  
  // A. Next Turn (Step) Butonu
  // Sadece ikon kullanarak clean bir buton yapalım
  if (ImGui::Button(ICON_FA_FORWARD_STEP " Step")) { 
    GameManager::stepGamestate(gamestate);
  }
  
  // Eğer üzerine gelinirse ne işe yaradığını yazalım (Tooltip)
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Advance 1 Turn");

  ImGui::SameLine();

  // B. Play / Pause (Renk Değiştiren Buton)
  if (gamestate.paused) {
      // Oyun Duruyor -> Oynat Butonu (YEŞİL)
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.0f, 1.0f)); 
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
      
      if (ImGui::Button(ICON_FA_PLAY " Play")) {
          gamestate.togglePause();
      }
      
      ImGui::PopStyleColor(2); // Renkleri geri al
  } else {
      // Oyun Akıyor -> Durdur Butonu (TURUNCU/SARI)
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.0f, 1.0f)); 
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
      
      if (ImGui::Button(ICON_FA_PAUSE " Pause")) {
          gamestate.togglePause();
      }
      
      ImGui::PopStyleColor(2);
  }

  ImGui::SameLine();
  
  // C. Hız Ayarı (Slider)
  // InputInt yerine Slider daha "Gamey" hissettirir.
  ImGui::PushItemWidth(100); // Slider çok geniş olmasın
  ImGui::Text("%s", ICON_FA_GAUGE_HIGH); // Hız göstergesi ikonu
  ImGui::SameLine();
  if (ImGui::SliderInt("##speed", &gamestate.gameSpeed, 1, 60, "%d FPS")) {
       // clamp zaten slider içinde otomatik oluyor ama yine de güvenli olsun
       gamestate.gameSpeed = std::clamp(gamestate.gameSpeed, 1, 60);
  }
  ImGui::PopItemWidth();
  
  // Hover yapınca açıklama
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Game Speed (Ticks per Second)");
}
} // namespace UI::Panels