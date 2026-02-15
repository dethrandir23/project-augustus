#include "../Api/NativeApi.h"
#include "../DevTools/Console.h"
#include "Economy/Company.h"
#include "Game/GameManager.h"
#include "Icons/IconsFontAwesome6.h"
#include "ModLoader.h"
#include "imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <filesystem>
#include <stdio.h>
#include "Windows/CompanyInfoWindow.h"
#include "Windows/GameControlsWindow.h"
#include "Windows/InventoryWindow.h"
#include "Windows/ConsoleWindow.h"
#include "Windows/InventoryWindow.h"
#include "Windows/MarketWindow.h"
#include "Panels/ManagerPanel.h"
#include "Panels/CompanyInfoPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/GameControls.h"
#include "Panels/InventoryPanel.h"
#include "Panels/MarketPanel.h"
#include "UIManager.h"
#include "DevTools/ConsoleUtils.h"

namespace fs = std::filesystem;

constexpr const size_t WINDOW_WIDTH = 1280;
constexpr const size_t WINDOW_HEIGHT = 720;
constexpr const char *WINDOW_TITLE = "Producer Engine";

void printConsole() {
  // Console loglarını basan yardımcı fonksiyon (değişmedi)
  for (const Log &l : Console::getLogs()) {
    // logTypeToStr fonksiyonunun tanimli oldugunu varsayiyorum
    std::cout << "[" << logTypeToStr(l.logType) << "] " << l.message << " ["
              << l.timestamp << "]" << std::endl;
  }
}

int main(int argc, char **argv) {
  // --- 1. ARGUMENT PARSING & MOD LOADING (Aynı kalıyor) ---
  if (argc != 2) {
    printf("Usage: %s <load_order.json>\n", argv[0]);
    return 1;
  }

  auto path = fs::path(argv[1]);
  if (!fs::exists(path)) {
    printf("File not found: %s\n", argv[1]);
    return 1;
  }

  printf("File found: %s\n", argv[1]);
  printf("Mods are caching...\n");

  if (!ModLoader::loadMods(path)) {
    printf("Mods failed to cached.\n");
    printConsole();
    return 1;
  }

  printf("%zu mods successfully cached.\n", ModLoader::getModCount());

  // --- 2. ENGINE INIT ---
  GameApi::initEngine();

  if (!ModLoader::insertDataIntoEngine(GameApi::loadGameFiles)) {
    printf("Mods failed to load.\n");
    printConsole();
    return 1;
  };

  printf("%zu mods successfully loaded.\n", ModLoader::getModCount());
  printf("Starting producer...\n");

  if (GameApi::startScenario("debug_scenario")) {
    printf("Scenario loaded.\n");
  } else {
    printf("Failed to load scenario.\n");
    return 1;
  }

  GameApi::SetPlayer("APPLE INC.", "core_startup_company_001");
  printf("Player set.\n");
  
  // GameApi::subscribeToEvents();
  GLFWwindow *window = UIManager::PrepareWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
      printf("Failed to create window!\n");
      return 1;
  }

  UIManager::addWindow(ICON_FA_GEARS " UI Manager", UI::Panels::UIManagerPanel);
  UIManager::addWindow(ICON_FA_BUILDING " Company", UI::Panels::CompanyInfoPanel);
  auto consoleId = UIManager::addWindow(ICON_FA_TERMINAL " Console", UI::Panels::ConsolePanel);
  UIManager::addWindow(ICON_FA_GAMEPAD " Game", UI::Panels::GameControls);
  UIManager::addWindow(ICON_FA_BOX " Inventory", UI::Panels::InventoryPanel);
  UIManager::addWindow(ICON_FA_STORE " Market", UI::Panels::MarketPanel);

  UIManager::getWindows()[consoleId].flags = ImGuiWindowFlags_MenuBar;

  // Initial Player Data Setup
  auto playerCompany = GameApi::globalGamestate.getPlayerCompany();
  if (playerCompany.has_value()) {
      playerCompany.value().setCapital(1000000.0);
  }

  // --- 4. GAME LOOP ---
  while (!glfwWindowShouldClose(window)) {
      // Çizim Başlangıcı (ImGui NewFrame vb.)
      UIManager::StartDraw();

      // Logic Update
      // Not: UIManager ImGui context'ini initialize etti, ImGui::GetIO() güvenli.
      float dt = ImGui::GetIO().DeltaTime; 
      GameManager::update(GameApi::globalGamestate, dt);

      // UI Update & Render
      // Burasi sihirli kisim: Ekledigin tum pencereler burada ciziliyor.
      UIManager::drawWindows(GameApi::globalGamestate);

      // Çizim Bitişi (Render & SwapBuffers)
      UIManager::EndDraw(window);
  }

  UIManager::Cleanup(window);

  return 0;
}

/*
int main(int, char **) {

  GLFWwindow *window = PrepareWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window)
    return 1;
  bool game = true;
  while (!glfwWindowShouldClose(window) && game) {
    StartDraw();

// Ekranı kaplayan bir pencere açalım
// ImGuiIO &io = ImGui::GetIO();
// ImGui::SetNextWindowSize(io.DisplaySize);
// ImGui::SetNextWindowPos(ImVec2(0, 0));
// ImGui::Begin("Ana Pencere", nullptr, ImGuiWindowFlags_NoDecoration |
ImGuiWindowFlags_NoResize);
// ImGui::End();

    EndDraw(window);
  }

  Cleanup(window);
  return 0;
}

*/