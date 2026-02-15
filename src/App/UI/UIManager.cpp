#include "UIManager.h"
#include <stdexcept>
#include <filesystem>
#include <iostream>

#include "imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Icons/IconsFontAwesome6.h"
#include "Game/IdUtils.h" 

namespace fs = std::filesystem;

std::unordered_map<uuids::uuid, WindowComponent> UIManager::windows;
int UIManager::mainWindowWidth = 1280;
int UIManager::mainWindowHeight = 720;
std::string UIManager::mainWindowTitle = "Game";

ImFont *fontNormal = nullptr;
ImFont *fontBig = nullptr;

// ManagerPanel icin gerekli fonksiyon
std::unordered_map<uuids::uuid, WindowComponent>& UIManager::getWindows() {
    return windows;
}

void UIManager::drawWindows(Gamestate &gamestate) {
    drawMainMenuBar(gamestate); // Artık görünecek!

    for (auto &pair : windows) {
        auto &win = pair.second;
        if (!win.isOpen) continue;

        if (win.forceReset) {
            ImGui::SetNextWindowPos(ImVec2(win.defaultPosX, win.defaultPosY), ImGuiCond_Always);
            win.forceReset = false;
        } else if (win.hasDefaultPos) {
            ImGui::SetNextWindowPos(ImVec2(win.defaultPosX, win.defaultPosY), ImGuiCond_FirstUseEver);
        }

        // --- PENCEREYİ BAYRAKLARLA BAŞLAT ---
        if (ImGui::Begin(win.title.c_str(), &win.isOpen, win.flags)) {
            win.function(gamestate);
        }
        ImGui::End();
    }
}

void UIManager::drawMainMenuBar(Gamestate& gamestate) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Game")) {
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            for (auto &pair : windows) {
                if (ImGui::MenuItem(pair.second.title.c_str(), nullptr, pair.second.isOpen)) {
                    pair.second.isOpen = !pair.second.isOpen;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

uuids::uuid UIManager::addWindow(const std::string& title, const WindowFunction &windowFunction) {
    WindowComponent comp;
    comp.id = IdUtils::generateUuid();
    comp.title = title;
    comp.function = windowFunction;
    comp.isOpen = true;
    windows[comp.id] = comp;
    return comp.id;
}

GLFWwindow *UIManager::PrepareWindow(int width, int height, const char *title) {
    if (!glfwInit()) return NULL;

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) return NULL;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // DOCKING YOK!

    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Font Yükleme
    float fontSize = 18.0f;
    if (fs::exists("fonts/Roboto-Medium.ttf")) {
        fontNormal = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", fontSize);
    } else {
        fontNormal = io.Fonts->AddFontDefault();
    }

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = fontSize;
    
    if (fs::exists("fonts/Font Awesome 7 Free-Solid-900.otf")) {
        io.Fonts->AddFontFromFileTTF("fonts/Font Awesome 7 Free-Solid-900.otf", fontSize, &icons_config, icons_ranges);
    }
    
    io.Fonts->Build();
    
    mainWindowWidth = width;
    mainWindowHeight = height;

    return window;
}

void UIManager::StartDraw() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::EndDraw(GLFWwindow *window) {
    ImGui::Render();
    
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // VIEWPORTS KODU SILINDI
    
    glfwSwapBuffers(window);
}

void UIManager::Cleanup(GLFWwindow *window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void UIManager::setMainWindowSize(int width, int height) {
    mainWindowWidth = width;
    mainWindowHeight = height;
}

int UIManager::getMainWindowWidth() { return mainWindowWidth; }
int UIManager::getMainWindowHeight() { return mainWindowHeight; }