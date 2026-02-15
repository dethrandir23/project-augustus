// ConsoleWindow.cpp
#include "Game/Gamestate.h"
#include "ConsoleWindow.h"
#include "imgui.h"
#include "App/UI/Panels/ConsolePanel.h"

namespace UI::Windows {
    void ConsoleWindow(Gamestate &gamestate) {
        if (ImGui::Begin("Console")) { 
            UI::Panels::ConsolePanel(gamestate);
        }
        ImGui::End();
    }
}