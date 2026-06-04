#ifdef EMSCRIPTEN
#include <emscripten.h>
#include "Api/WebApi.cpp"
int main() { return 0; }
#else
#include "webview.h"
#include "Api/EngineController.h"
#include "ModLoader.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

std::string readFile(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

std::string inlineFrontend(const std::string &distPath) {
    std::string html = readFile(distPath + "/index.html");
    if (html.empty()) return "";

    // Bridge script: proxies WASM GameModule calls to window.api_* native bindings
    const std::string bridge = R"(<script>
function _s(v){return v==null?'[]':typeof v==='string'?v:JSON.stringify(v)}
var _cb=null,_module={
    initEngine:function(){
        window.api_initEngine();
        if(_cb)_cb('engine_ready','{}');
    },
    step:async function(){
        await window.api_step();
        if(_cb)_cb('tick_complete',_s(await window.api_getSerializedState()));
    },
    stepN:async function(n){
        if(window.api_stepN)await window.api_stepN(n);else for(var i=0;i<n;i++)await window.api_step();
        if(_cb)_cb('stepN_complete',_s(await window.api_getSerializedState()));
    },
    sendInput:async function(json){
        try{var o=JSON.parse(json);return await window.api_sendInput(JSON.stringify({type:o.type,payload:o.payload||{}}))}
        catch(e){return!1}
    },
    getSerializedState:async function(){return _s(await window.api_getSerializedState())},
    getDeltaState:async function(){return _s(await window.api_getDeltaState())},
    getPlayerState:async function(){return _s(await window.api_getPlayerState())},
    getMarketData:async function(id){return _s(await window.api_getMarketData(String(id)))},
    getFactoryStatus:async function(id){return _s(await window.api_getFactoryStatus(String(id)))},
    getEntityOrders:async function(id){return _s(await window.api_getEntityOrders(String(id)))},
    getPendingEvents:async function(){return _s(await window.api_getPendingEvents())},
    readConsole:async function(){var r=await window.api_readConsole();try{return typeof r==='string'?JSON.parse(r):r||[]}catch(e){}return[]},
    saveGame:async function(n){return await window.api_saveGame(String(n))},
    loadGame:async function(n){return await window.api_loadGame(String(n))},
    listSaves:async function(){var r=await window.api_listSaves();try{return typeof r==='string'?JSON.parse(r):r||[]}catch(e){}return[]},
    loadGameFiles:function(){return true},
    getFactoryTemplates:async function(){return _s(await window.api_getFactoryTemplates())},
    getCompanyNetWorth:async function(id){return _s(await window.api_getCompanyNetWorth(String(id)))},
    getMarketStats:async function(id){return _s(await window.api_getMarketStats(String(id)))},
    getNodeStats:async function(id){return _s(await window.api_getNodeStats(String(id)))},
    getFactoryStats:async function(id){return _s(await window.api_getFactoryStats(String(id)))},
    getEconomySummary:async function(){return _s(await window.api_getEconomySummary())},
    getEconomyReport:async function(){return _s(await window.api_getEconomyReport())},
    startScenario:async function(id){return await window.api_startScenario(String(id))},
    setPlayer:async function(n,t,a){await window.api_setPlayer(String(n),String(t),!!a)},
    setJsCallback:function(cb){_cb=cb}
};
window.GameModule=function(){return Promise.resolve(_module)};
window.api_initEngine=function(){};
</script>)";

    // 1. Inline CSS: <link ... href="assets/xxx.css"> → <style>content</style>
    size_t pos = 0;
    while ((pos = html.find("<link", pos)) != std::string::npos) {
        size_t hrefPos = html.find("href=\"", pos);
        if (hrefPos == std::string::npos || hrefPos > html.find('>', pos)) break;
        hrefPos += 6;
        size_t endQuote = html.find('"', hrefPos);
        std::string href = html.substr(hrefPos, endQuote - hrefPos);
        if (href.find(".css") == std::string::npos) { pos = hrefPos; continue; }
        size_t closeTag = html.find('>', endQuote);
        if (closeTag == std::string::npos) break;
        std::string css = readFile(distPath + "/" + href);
        if (css.empty()) break;
        std::string oldBlock = html.substr(pos, closeTag + 1 - pos);
        html.replace(pos, oldBlock.size(), "<style>\n" + css + "\n</style>");
        pos += 1;
    }

    // 2. Collect all external JS, remove script tags, inline at end of <body>
    std::string allScripts = bridge + "\n";
    pos = 0;
    while ((pos = html.find("<script", pos)) != std::string::npos) {
        size_t closeTag = html.find('>', pos);
        if (closeTag == std::string::npos) break;
        size_t srcPos = html.find("src=\"", pos);
        if (srcPos != std::string::npos && srcPos < closeTag) {
            srcPos += 5;
            size_t endQuote = html.find('"', srcPos);
            std::string src = html.substr(srcPos, endQuote - srcPos);
            size_t scriptEnd = html.find("</script>", closeTag);
            if (scriptEnd == std::string::npos) break;
            std::string js = readFile(distPath + "/" + src);
            if (!js.empty()) {
                allScripts += "<script>\n" + js + "\n</script>\n";
            }
            html.erase(pos, scriptEnd + 9 - pos);
        } else {
            pos = closeTag + 1;
        }
    }

    // 3. Inject all scripts before </body>
    size_t bodyEnd = html.find("</body>");
    if (bodyEnd != std::string::npos) {
        html.insert(bodyEnd, allScripts);
    } else {
        html += allScripts;
    }

    return html;
}

int main(int argc, char **argv) {
    std::cout << "[INFO] TradeFall Engine starting...\n";

    augustus_engine::EngineController::instance().init();
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <load_order.json>\n";
        return 1;
    }
    if (!ModLoader::loadMods(argv[1])) {
        std::cerr << "[FATAL] Failed to load mods from: " << argv[1] << "\n";
        return 1;
    }
    ModLoader::insertDataIntoEngine(
        [](const auto &c, const auto &n) { return augustus_engine::EngineController::instance().loadGameFiles(c, n); });
    webview::webview w(true, nullptr);
    w.set_title("TradeFall - Project Augustus");
    w.set_size(1280, 720, WEBVIEW_HINT_NONE);

    auto &engine = augustus_engine::EngineController::instance();
    w.bind("api_step", [&engine](std::string) -> std::string {
        engine.step();
        return "true";
    });

    w.bind("api_sendInput", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string payload = args[0].get<std::string>();
            bool success = engine.sendInput(payload);
            return success ? "true" : "false";
        } catch (...) {
            return "false";
        }
    });

    w.bind("api_readConsole", [&engine](std::string) -> std::string {
        auto logs = engine.readConsole();
        nlohmann::json j = logs;
        return j.dump();
    });

    w.bind("api_getSerializedState", [&engine](std::string) -> std::string {
        return engine.getSerializedState();
    });

    w.bind("api_getPendingEvents", [&engine](std::string) -> std::string {
        return engine.getPendingEvents();
    });

    w.bind("api_getPlayerState", [&engine](std::string) -> std::string {
        return engine.getPlayerState();
    });

    w.bind("api_saveGame", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string name = args[0].get<std::string>();
            return engine.saveGame(name) ? "true" : "false";
        } catch (...) {
            return "false";
        }
    });

    w.bind("api_loadGame", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string name = args[0].get<std::string>();
            return engine.loadGame(name) ? "true" : "false";
        } catch (...) {
            return "false";
        }
    });

    w.bind("api_getDeltaState", [&engine](std::string) -> std::string {
        return engine.getDeltaState();
    });

    w.bind("api_initEngine", [](std::string) -> std::string {
        return "true";
    });

    w.bind("api_startScenario", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string id = args[0].get<std::string>();
            return engine.startScenario(id) ? "true" : "false";
        } catch (...) { return "false"; }
    });

    w.bind("api_setPlayer", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string name = args[0].get<std::string>();
            std::string templateId = args[1].get<std::string>();
            bool isAI = args[2].get<bool>();
            engine.setPlayer(name, templateId, isAI);
            return "true";
        } catch (...) { return "false"; }
    });

    w.bind("api_stepN", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            size_t n = args[0].get<size_t>();
            for (size_t i = 0; i < n; i++) engine.step();
            return "true";
        } catch (...) { return "false"; }
    });

    w.bind("api_getMarketData", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string id = args[0].get<std::string>();
            return engine.getMarketData(id);
        } catch (...) { return "[]"; }
    });

    w.bind("api_getEntityOrders", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string id = args[0].get<std::string>();
            return engine.getEntityOrders(id);
        } catch (...) { return "[]"; }
    });

    w.bind("api_getFactoryStatus", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string id = args[0].get<std::string>();
            return engine.getFactoryStatus(id);
        } catch (...) { return "{}"; }
    });

    w.bind("api_getFactoryTemplates", [&engine](std::string) -> std::string {
        return engine.getFactoryTemplates();
    });

    w.bind("api_listSaves", [&engine](std::string) -> std::string {
        auto saves = engine.listSaves();
        nlohmann::json j = saves;
        return j.dump();
    });

    w.bind("api_getCompanyNetWorth", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string id = args[0].get<std::string>();
            return engine.getCompanyNetWorth(id);
        } catch (...) { return "{}"; }
    });

    w.bind("api_getEconomyReport", [&engine](std::string) -> std::string {
        return engine.getEconomyReport();
    });

    w.bind("api_getEconomySummary", [&engine](std::string) -> std::string {
        return engine.getEconomySummary();
    });

    w.bind("api_getMarketStats", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            return engine.getMarketStats(args[0].get<std::string>());
        } catch (...) { return "{}"; }
    });

    w.bind("api_getNodeStats", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            return engine.getNodeStats(args[0].get<std::string>());
        } catch (...) { return "{}"; }
    });

    w.bind("api_getFactoryStats", [&engine](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            return engine.getFactoryStats(args[0].get<std::string>());
        } catch (...) { return "{}"; }
    });

    std::string htmlContent = inlineFrontend("/home/efe/Documents/projects/project-augustus/src/frontend/dist");
    if (htmlContent.empty()) {
        std::cerr << "[ERROR] Frontend files not found at src/frontend/dist/\n";
        return 1;
    }
    w.set_html(htmlContent);

    std::cout << "[INFO] UI started. DevTools open.\n";
    w.run();

    return 0;
}
#endif
