#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>

#include "Api/EngineController.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <emscripten/val.h>

static EngineController *webController = nullptr;

static EngineController &controller() {
    if (!webController) webController = new EngineController();
    return *webController;
}

static emscripten::val jsCallback;

void setJsCallback(emscripten::val cb) {
    jsCallback = cb;
}

void pushEvent(const std::string &type, const std::string &data) {
    if (jsCallback != emscripten::val::undefined()) {
        jsCallback(type, data);
    }
}

void initEngine() {
    controller().init();
    pushEvent("engine_ready", "{}");
}

bool loadGameFiles(const std::vector<std::string> &contents,
                   const std::vector<std::string> &names) {
    bool ok = controller().loadGameFiles(contents, names);
    pushEvent("files_loaded", std::string(ok ? "true" : "false"));
    return ok;
}

bool startScenario(const std::string &scenarioId) {
    bool ok = controller().startScenario(scenarioId);
    pushEvent("scenario_loaded", std::string(ok ? "true" : "false"));
    return ok;
}

void setPlayer(const std::string &name, const std::string &templateId, bool isAI) {
    controller().setPlayer(name, templateId, isAI);
    pushEvent("player_created", controller().getPlayerState());
}

void step() {
    controller().step();
    pushEvent("tick_complete", controller().getSerializedState());
}

void stepN(size_t n) {
    controller().stepN(n);
    pushEvent("stepN_complete", controller().getSerializedState());
}

std::string getSerializedState() { return controller().getSerializedState(); }

std::string getPlayerState() { return controller().getPlayerState(); }

std::string getMarketData(const std::string &marketId) {
    return controller().getMarketData(marketId);
}

std::string getFactoryStatus(const std::string &factoryId) {
    return controller().getFactoryStatus(factoryId);
}

bool sendInput(const std::string &inputJson) {
    try {
        nlohmann::json j = nlohmann::json::parse(inputJson);
        return controller().sendInput(j);
    } catch (const std::exception &e) {
        return false;
    }
}

std::string getPendingEvents() { return controller().getPendingEvents(); }

std::vector<std::string> readConsole() { return controller().readConsole(); }

bool saveGame(const std::string &name) { return controller().saveGame(name); }

bool loadGame(const std::string &name) { return controller().loadGame(name); }

std::vector<std::string> listSaves() { return controller().listSaves(); }

void logToConsole(const std::string &msg) { controller().logToConsole(msg); }

EMSCRIPTEN_BINDINGS(game_module) {
    emscripten::function("setJsCallback", &setJsCallback);
    emscripten::function("initEngine", &initEngine);
    emscripten::function("loadGameFiles", &loadGameFiles);
    emscripten::function("startScenario", &startScenario);
    emscripten::function("setPlayer", &setPlayer);

    emscripten::function("step", &step);
    emscripten::function("stepN", &stepN);

    emscripten::function("getSerializedState", &getSerializedState);
    emscripten::function("getPlayerState", &getPlayerState);
    emscripten::function("getMarketData", &getMarketData);
    emscripten::function("getFactoryStatus", &getFactoryStatus);
    emscripten::function("getPendingEvents", &getPendingEvents);

    emscripten::function("sendInput", &sendInput);

    emscripten::function("readConsole", &readConsole);
    emscripten::function("logToConsole", &logToConsole);

    emscripten::function("saveGame", &saveGame);
    emscripten::function("loadGame", &loadGame);
    emscripten::function("listSaves", &listSaves);

    emscripten::register_vector<std::string>("StringList");

    emscripten::enum_<LogType>("LogType")
        .value("INFO", LogType::INFO)
        .value("ERROR", LogType::ERROR)
        .value("WARNING", LogType::WARNING);

    emscripten::value_object<Log>("Log")
        .field("message", &Log::message)
        .field("logType", &Log::logType)
        .field("timestamp", &Log::timestamp);

    emscripten::class_<Console>("Console")
        .class_function("help", &Console::help)
        .class_function("parseInput", &Console::parseInput)
        .class_function(
            "log",
            emscripten::select_overload<void(const std::string &)>(&Console::log))
        .class_function(
            "logWithType",
            emscripten::select_overload<void(const std::string &, LogType)>(
                &Console::log))
        .class_function("logObject",
                        emscripten::select_overload<void(Log)>(&Console::log))
        .class_function("readLog", &Console::readLog)
        .class_function("getLog", &Console::getLog)
        .class_function("clearLogs", &Console::clearLogs)
        .class_function("removeLog", &Console::removeLog)
        .class_function("getLogCount", &Console::getLogCount)
        .class_function("getLogs", &Console::getLogs)
        .class_function("getLogMessages", &Console::getLogMessages);
}
#endif
