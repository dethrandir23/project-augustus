#include "App/UI/Panels/ConsolePanel.h"
#include "App/UI/Widgets/InputBar.h"
#include "DevTools/Console.h"
#include "DevTools/ConsoleUtils.h"
#include "Game/InputHandler.h"
#include "imgui.h"
#include "nlohmann/json.hpp"
#include "Icons/IconsFontAwesome6.h"
#include <vector>
#include <string>

using json = nlohmann::json;

// Renk/İkon yardımcıları
ImVec4 GetLogColor(LogType type) {
    switch (type) {
        case LogType::INFO:    return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
        case LogType::WARNING: return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
        case LogType::ERROR:   return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        default:               return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

const char* GetLogIcon(LogType type) {
    switch (type) {
        case LogType::INFO:    return ICON_FA_CIRCLE_INFO;
        case LogType::WARNING: return ICON_FA_TRIANGLE_EXCLAMATION;
        case LogType::ERROR:   return ICON_FA_BUG;
        default:               return ICON_FA_COMMENT;
    }
}

namespace UI::Panels {

// UI durumunu tutmak için static değişkenler (Veya panel sınıfındaysa member yapabilirsin)
static bool showTimestamps = true;
static bool showIcons = true;
static bool autoScroll = true;
static char searchBuffer[128] = "";
static bool filterInfo = true;
static bool filterWarning = true;
static bool filterError = true;

void ConsolePanel(Gamestate &gamestate) {
    float footerHeight = ImGui::GetFrameHeightWithSpacing() + 5.0f;

    // --- MENU BAR ---
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu(ICON_FA_FILE_EXPORT " Export")) {
                if (ImGui::MenuItem("As Text (.txt)")) ConsoleUtils::saveConsoleToFile(std::string("logs.txt"), ExportFormat::TXT);
                if (ImGui::MenuItem("As JSON (.json)")) ConsoleUtils::saveConsoleToFile(std::string("logs.json"), ExportFormat::JSON);
                if (ImGui::MenuItem("As CSV (.csv)")) ConsoleUtils::saveConsoleToFile(std::string("logs.csv"), ExportFormat::CSV);
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(ICON_FA_FILE_IMPORT " Import Default")) {
                 ConsoleUtils::loadConsoleFromFile(std::string("logs.json"));
            }
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_TRASH " Clear")) Console::clearLogs();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Timestamps", nullptr, &showTimestamps);
            ImGui::MenuItem("Show Icons", nullptr, &showIcons);
            ImGui::MenuItem("Auto-scroll", nullptr, &autoScroll);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Filter")) {
            ImGui::MenuItem("Info", nullptr, &filterInfo);
            ImGui::MenuItem("Warning", nullptr, &filterWarning);
            ImGui::MenuItem("Error", nullptr, &filterError);
            ImGui::EndMenu();
        }

        // Sağ tarafta log sayısını gösterelim
        float posX = ImGui::GetCursorPosX();
        float width = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(posX + width - 100);
        ImGui::TextDisabled("Logs: %zu", Console::getLogCount());

        ImGui::EndMenuBar();
    }

    // --- SEARCH BAR ---
    ImGui::PushItemWidth(-1);
    if (ImGui::InputTextWithHint("##Search", ICON_FA_MAGNIFYING_GLASS " Search messages...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
        // Arama yapıldıkça liste güncellenir
    }
    ImGui::PopItemWidth();

    // --- SCROLLING REGION ---
if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        
        const auto& allLogs = Console::getLogs();
        
        // 1. ADIM: Önce görünür olması gereken logların indekslerini topla
        static std::vector<int> visibleIndices;
        visibleIndices.clear();
        
        for (int i = 0; i < (int)allLogs.size(); i++) {
            const Log& l = allLogs[i];

            // Filtreleme Kontrolleri
            if (l.logType == LogType::INFO && !filterInfo) continue;
            if (l.logType == LogType::WARNING && !filterWarning) continue;
            if (l.logType == LogType::ERROR && !filterError) continue;

            // Arama Kontrolü
            if (strlen(searchBuffer) > 0 && 
                l.message.find(searchBuffer) == std::string::npos) continue;

            visibleIndices.push_back(i);
        }

        // 2. ADIM: Clipper'a sadece GÖRÜNÜR olanların sayısını ver
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(visibleIndices.size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                // visibleIndices üzerinden asıl loga erişiyoruz
                const Log& l = allLogs[visibleIndices[i]];

                ImGui::PushStyleColor(ImGuiCol_Text, GetLogColor(l.logType));
                
                std::string line;
                if (showTimestamps) line += "[" + l.timestamp + "] ";
                if (showIcons)      line += std::string(GetLogIcon(l.logType)) + " ";
                line += l.message;

                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered()) {
                    if (ImGui::IsMouseClicked(0)) ImGui::SetClipboardText(l.message.c_str());
                }
            }
        }

        // ... Otomatik kaydırma mantığı aynı ...
        ImGui::EndChild();
    }

    ImGui::Separator();

    // INPUT BAR
    static std::string input = "";
    if (Widgets::InputBar("##ConsoleInput", input)) {
        if (!input.empty()) {
            Console::log("> " + input, LogType::INFO);
            json output = Console::parseInput(input);
            if (!output.is_null()) {
                InputHandler::handleInput(gamestate, output);
            }
            input = ""; 
            ImGui::SetKeyboardFocusHere(-1); 
        }
    }
}
}