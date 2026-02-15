#pragma once
#include <Game/Gamestate.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <optional>
#include <GLFW/glfw3.h>
#include "Game/IdUtils.h" // uuid include'un nerede ise orayi ver
#include "imgui.h"

using WindowFunction = std::function<void(Gamestate&)>;

struct WindowComponent {
    std::string title;
    WindowFunction function;
    uuids::uuid id;

    bool isOpen = true;
    bool forceReset = false;
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;

    std::optional<float> defaultWidth;
    std::optional<float> defaultHeight;
    bool hasDefaultPos = false;
    float defaultPosX = 0;
    float defaultPosY = 0;

    WindowComponent() {}
};

class UIManager {
public:
    UIManager() = delete;

    // Main.cpp'de cagirdigin isimle ayni yaptim:
    static void drawWindows(Gamestate &gamestate);

    static uuids::uuid addWindow(const std::string& title, const WindowFunction &windowFunction);
    
    // ManagerPanel'in erisebilmesi icin bu sart:
    static std::unordered_map<uuids::uuid, WindowComponent>& getWindows();

    static GLFWwindow *PrepareWindow(int width, int height, const char *title);
    static void StartDraw();
    static void EndDraw(GLFWwindow *window);
    static void Cleanup(GLFWwindow *window);

    static void setMainWindowSize(int width, int height);
    static int getMainWindowWidth();
    static int getMainWindowHeight();

private:
    static void drawMainMenuBar(Gamestate& gamestate);

    static std::unordered_map<uuids::uuid, WindowComponent> windows;
    static int mainWindowWidth;
    static int mainWindowHeight;
    static std::string mainWindowTitle;
};