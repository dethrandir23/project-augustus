#include "Api/EngineController.h"
#include "DevTools/Console.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

static std::string readFile(const fs::path& path) {
    std::ifstream f(path);
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

static json loadAllDataFiles(const fs::path& dataRoot) {
    json report;
    report["loaded_files"] = json::array();

    std::vector<std::string> contents;
    std::vector<std::string> names;

    for (auto& entry : fs::recursive_directory_iterator(dataRoot)) {
        if (!entry.is_regular_file()) continue;
        auto path = entry.path();
        if (path.extension() != ".json") continue;

        auto content = readFile(path);
        // Verify it has a registry_type field before attempting to load
        try {
            auto parsed = json::parse(content);
            if (!parsed.contains("registry_type")) continue;
        } catch (...) {
            continue; // Skip unparseable files
        }

        auto rel = fs::relative(path, dataRoot.parent_path()).string();
        contents.push_back(content);
        names.push_back(rel);
        report["loaded_files"].push_back(rel);
    }

    Console::log("Loading " + std::to_string(contents.size()) + " data files...");
    bool ok = augustus_engine::EngineController::instance().loadGameFiles(contents, names);
    report["data_load_success"] = ok;
    return report;
}

static void captureSnapshot(json& history, int tick) {
    json snap;
    snap["tick"] = tick;

    try {
        auto state = json::parse(augustus_engine::EngineController::instance().getSerializedState());
        auto player = json::parse(augustus_engine::EngineController::instance().getPlayerState());

        snap["turn"] = state.value("turn", 0);
        snap["entities"] = state["entities"].size();

        const auto& components = player.value("components", json::object());
        const auto& walletComp = components.value("WalletComponent", json::object());
        snap["balance"] = walletComp.value("balance", 0.0);
        snap["debt"] = walletComp.value("debt", 0.0);

        const auto& assetsComp = components.value("AssetOwnerComponent", json::object());
        const auto& ownedAssets = assetsComp.value("ownedAssets", json::array());
        int ownedFactories = 0;
        for (const auto& a : ownedAssets) {
            if (a.is_string()) ownedFactories++;
        }
        snap["factories_owned"] = ownedFactories;

        int aiFactories = 0;
        for (const auto& e : state["entities"]) {
            if (e.value("type", "") == "factory") aiFactories++;
        }
        snap["factories_total"] = aiFactories;

        history.push_back(snap);

        bool isKey = (tick <= 5) || (tick % 50 == 0);
        if (isKey) {
            std::cout << "  [Tick " << tick << "] "
                      << "Turn: " << snap["turn"].get<int>()
                      << " | Balance: " << snap["balance"].get<double>()
                      << " | Debt: " << snap["debt"].get<double>()
                      << " | My Factories: " << snap["factories_owned"].get<int>()
                      << " | Total Factories: " << snap["factories_total"].get<int>()
                      << std::endl;
        }
    } catch (const std::exception& e) {
        Console::log("Snapshot error at tick " + std::to_string(tick) + ": " + e.what(), LogType::ERROR);
    }
}

int runHeadless(int totalTicks, int numAICompanies, const std::string& dataRoot) {
    json results;
    results["config"] = {{"total_ticks", totalTicks},
                          {"num_ai_companies", numAICompanies},
                          {"data_root", dataRoot}};
    results["phases"] = json::object();
    results["tick_history"] = json::array();

    // Phase 1: Engine Init
    std::cout << "[Phase 1] Initializing engine..." << std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    augustus_engine::EngineController::instance().init();
    auto t2 = std::chrono::high_resolution_clock::now();
    results["phases"]["init_time_ms"] = std::chrono::duration<double, std::milli>(t2 - t1).count();
    results["phases"]["init_success"] = true;

    // Phase 2: Data Loading
    std::cout << "[Phase 2] Loading data from: " << dataRoot << std::endl;
    t1 = std::chrono::high_resolution_clock::now();
    json loadReport = loadAllDataFiles(dataRoot);
    t2 = std::chrono::high_resolution_clock::now();
    loadReport["time_ms"] = std::chrono::duration<double, std::milli>(t2 - t1).count();
    results["phases"]["data_loading"] = loadReport;

    if (!loadReport["data_load_success"].get<bool>()) {
        std::cerr << "FATAL: Data loading failed!" << std::endl;
        results["fatal"] = "Data loading failed";
        return 1;
    }

    // Phase 3: Scenario
    std::cout << "[Phase 3] Starting scenario..." << std::endl;
    t1 = std::chrono::high_resolution_clock::now();
    bool scenarioOk = augustus_engine::EngineController::instance().startScenario("debug_scenario");
    t2 = std::chrono::high_resolution_clock::now();
    results["phases"]["scenario_time_ms"] = std::chrono::duration<double, std::milli>(t2 - t1).count();
    results["phases"]["scenario_success"] = scenarioOk;

    if (!scenarioOk) {
        std::cerr << "FATAL: Scenario not found!" << std::endl;
        results["fatal"] = "Scenario not found";
        return 1;
    }

    // Phase 4: Companies
    std::cout << "[Phase 4] Creating companies (1 player + " << numAICompanies << " AI)..." << std::endl;

    // Ensure Config.toml exists for save/load
    {
        fs::path cfgPath = fs::current_path() / "Config.toml";
        if (!fs::exists(cfgPath)) {
            std::cout << "  (Creating temporary Config.toml for save/load test)" << std::endl;
            std::ofstream cf(cfgPath);
            std::string saveDir = (fs::current_path() / "test_saves").string();
            cf << "[storage]\nsave_path = \"" << saveDir << "\"\n";
            cf.close();
        }
    }

    t1 = std::chrono::high_resolution_clock::now();
    augustus_engine::EngineController::instance().setPlayer("Player Corp", "core_startup_company_001", false);
    int aiCreated = 0;
    for (int i = 0; i < numAICompanies; i++) {
        try {
            augustus_engine::EngineController::instance().setPlayer(
                "AI Company " + std::to_string(i + 1),
                "core_startup_company_001", true);
            aiCreated++;
        } catch (const std::exception& e) {
            Console::log("AI company creation failed: " + std::string(e.what()), LogType::ERROR);
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    results["phases"]["company_time_ms"] = std::chrono::duration<double, std::milli>(t2 - t1).count();
    results["phases"]["total_companies"] = 1 + aiCreated;
    results["phases"]["ai_companies"] = aiCreated;

    // Phase 5: Game Loop
    std::cout << "[Phase 5] Running " << totalTicks << " ticks..." << std::endl;
    t1 = std::chrono::high_resolution_clock::now();
    int okTicks = 0, failTicks = 0;
    for (int tick = 1; tick <= totalTicks; tick++) {
        try {
            augustus_engine::EngineController::instance().step();
            okTicks++;
            captureSnapshot(results["tick_history"], tick);
        } catch (const std::exception& e) {
            Console::log("Tick " + std::to_string(tick) + " error: " + e.what(), LogType::ERROR);
            failTicks++;
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    double simTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
    results["phases"]["sim_time_ms"] = simTime;
    results["phases"]["ticks_ok"] = okTicks;
    results["phases"]["ticks_failed"] = failTicks;
    results["phases"]["avg_tick_ms"] = okTicks > 0 ? simTime / okTicks : 0;

    // Phase 6: Final Analysis
    std::cout << "[Phase 6] Analyzing results..." << std::endl;
    json analysis;
    try {
        auto state = json::parse(augustus_engine::EngineController::instance().getSerializedState());
        auto player = json::parse(augustus_engine::EngineController::instance().getPlayerState());
        auto events = json::parse(augustus_engine::EngineController::instance().getPendingEvents());

        analysis["final_turn"] = state.value("turn", 0);
        analysis["final_date"] = state.value("date", "");

        const auto& components2 = player.value("components", json::object());
        const auto& walletComp2 = components2.value("WalletComponent", json::object());
        analysis["player_balance"] = walletComp2.value("balance", 0.0);
        analysis["player_debt"] = walletComp2.value("debt", 0.0);

        const auto& assetsComp2 = components2.value("AssetOwnerComponent", json::object());
        const auto& ownedAssets2 = assetsComp2.value("ownedAssets", json::array());
        analysis["player_factories"] = ownedAssets2.size();

        int aiFactories = 0;
        for (const auto& e : state["entities"]) {
            if (e.value("type", "") == "factory") aiFactories++;
        }
        analysis["ai_factories_built"] = aiFactories - analysis["player_factories"].get<int>();
        analysis["total_companies"] = 0;
        for (const auto& e : state["entities"]) {
            if (e.value("type", "") == "company") analysis["total_companies"] = analysis["total_companies"].get<int>() + 1;
        }
        analysis["pending_events"] = events.size();

        auto logs = augustus_engine::EngineController::instance().readConsole();
        analysis["console_lines"] = logs.size();

        results["analysis"] = analysis;
    } catch (const std::exception& e) {
        Console::log("Analysis error: " + std::string(e.what()), LogType::ERROR);
        results["analysis_error"] = e.what();
    }

    // Phase 7: Save/Load Test
    std::cout << "[Phase 7] Save/Load test..." << std::endl;
    t1 = std::chrono::high_resolution_clock::now();
    bool saveOk = augustus_engine::EngineController::instance().saveGame("headless_test_save");
    t2 = std::chrono::high_resolution_clock::now();
    results["phases"]["save_ms"] = std::chrono::duration<double, std::milli>(t2 - t1).count();
    results["phases"]["save_ok"] = saveOk;

    auto saves = augustus_engine::EngineController::instance().listSaves();
    results["phases"]["saves_list"] = saves;

    t1 = std::chrono::high_resolution_clock::now();
    bool loadOk = augustus_engine::EngineController::instance().loadGame("headless_test_save");
    t2 = std::chrono::high_resolution_clock::now();
    results["phases"]["load_ms"] = std::chrono::duration<double, std::milli>(t2 - t1).count();
    results["phases"]["load_ok"] = loadOk;

    if (loadOk) {
        auto loadedState = json::parse(augustus_engine::EngineController::instance().getSerializedState());
        results["phases"]["loaded_turn"] = loadedState.value("turn", -1);
        results["phases"]["load_verified"] = (loadedState.value("turn", -1) == analysis.value("final_turn", -1));
    }

    // Phase 8: Market query test
    try {
        auto md = json::parse(augustus_engine::EngineController::instance().getMarketData("all"));
        results["phases"]["market_query_ok"] = md.is_array();
        results["phases"]["market_count"] = md.size();
    } catch (...) {
        results["phases"]["market_query_ok"] = false;
    }

    // Print Summary
    std::cout << "\n=== RESULTS ===" << std::endl;
    std::cout << "  Ticks:       " << okTicks << "/" << totalTicks << std::endl;
    std::cout << "  Failures:    " << failTicks << std::endl;
    std::cout << "  Save/Load:   " << (saveOk && loadOk ? "OK" : "FAIL") << std::endl;
    std::cout << "  Final turn:  " << analysis.value("final_turn", -1) << std::endl;
    std::cout << "  Balance:     " << analysis.value("player_balance", 0.0) << std::endl;
    std::cout << "  Factories:   " << analysis.value("player_factories", 0) << " player, "
              << analysis.value("ai_factories_built", 0) << " AI" << std::endl;
    std::cout << "  Events:      " << analysis.value("pending_events", 0) << " pending" << std::endl;
    std::cout << "  Avg tick:    " << results["phases"]["avg_tick_ms"].get<double>() << " ms" << std::endl;

    // Analysis / Recommendations
    std::cout << "\n=== ANALYSIS ===" << std::endl;
    if (failTicks > 0)
        std::cout << "  [WARN] " << failTicks << " ticks threw exceptions" << std::endl;

    if (analysis.value("player_balance", 0.0) <= 0 && analysis.value("player_factories", 0) == 0)
        std::cout << "  [NEEDS CHANGE] Player went bankrupt with no factories. "
                  << "Starting capital or economy balance needs adjustment." << std::endl;

    if (analysis.value("ai_factories_built", 0) == 0)
        std::cout << "  [NEEDS CHANGE] AI built 0 factories. "
                  << "investDesire may never go positive or all factory scores < threshold." << std::endl;

    if (analysis.value("ai_factories_built", 0) > analysis.value("player_factories", 0) * 5)
        std::cout << "  [NOTE] AI built vastly more factories than player. "
                  << "Multiple AI entities act per tick vs player's single action." << std::endl;

    if (!saveOk || !loadOk)
        std::cout << "  [NEEDS CHANGE] Save/Load cycle failed." << std::endl;
    else if (!results["phases"]["load_verified"].get<bool>())
        std::cout << "  [WARN] Save/Load state mismatch (turn differs)." << std::endl;
    else
        std::cout << "  [OK] Save/Load cycle verified." << std::endl;

    // Write report
    fs::path reportPath = fs::current_path() / "headless_test_report.json";
    std::ofstream rf(reportPath);
    rf << results.dump(2);
    rf.close();
    std::cout << "\n  Report: " << reportPath << std::endl;

    return failTicks > 0 ? 1 : 0;
}

int main(int argc, char** argv) {
    std::cout << "\n=== Project Augustus - Headless Engine Test ===\n" << std::endl;

    int totalTicks = 200;
    int numAICompanies = 3;
    std::string dataRoot = "data/core/data";

    if (argc > 1) totalTicks = std::stoi(argv[1]);
    if (argc > 2) numAICompanies = std::stoi(argv[2]);
    if (argc > 3) dataRoot = argv[3];

    // Resolve relative data root
    fs::path root(dataRoot);
    if (!root.is_absolute()) {
        root = fs::current_path() / dataRoot;
    }

    if (!fs::exists(root)) {
        std::cerr << "Data root not found: " << root << std::endl;
        std::cerr << "Provide the path to data/core/data/ as argv[3] or run from project root." << std::endl;
        return 1;
    }

    return runHeadless(totalTicks, numAICompanies, root.string());
}
