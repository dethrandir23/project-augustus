#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <unordered_map>
#include <functional>

class Gamestate;

class InputHandler {
public:
    using ActionCallback = std::function<bool(Gamestate&, const nlohmann::json&)>;

    static void init();

    static void registerAction(const std::string& actionName, ActionCallback callback);

    static bool handleInput(Gamestate& gamestate, const nlohmann::json& input);

private:
    static std::unordered_map<std::string, ActionCallback> actionRegistry;
};