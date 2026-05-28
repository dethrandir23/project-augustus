#include "Api/EngineController.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

std::string readFile(const fs::path &path) {
    std::ifstream f(path);
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

int main() {
    augustus_engine::EngineController::instance().init();

    // Load all game definition files from the mod directory
    fs::path dataRoot = "/home/efe/Documents/projects/project-augustus/mock/core/data";
    std::vector<std::string> names = {
        "items.json", "pipelines.json", "factories.json",
        "economy_settings.json", "trade_nodes.json", "companies.json",
        "markets.json", "map.json", "scenarios.json", "events.json"
    };
    std::vector<std::string> contents;
    for (auto &name : names) {
        std::string path = (dataRoot / "common" / name).string();
        if (!fs::exists(path))
            path = (dataRoot / "economy" / name).string();
        if (!fs::exists(path))
            path = (dataRoot / "scenarios" / name).string();
        if (!fs::exists(path))
            path = (dataRoot / "events" / name).string();
        contents.push_back(readFile(path));
    }
    if (!augustus_engine::EngineController::instance().loadGameFiles(contents, names)) {
        std::cerr << "Failed to load game files\n";
        return 1;
    }

    // Start scenario
    if (!augustus_engine::EngineController::instance().startScenario("debug_scenario")) {
        std::cerr << "Failed to start scenario\n";
        return 1;
    }

    // Create player company
    augustus_engine::EngineController::instance().setPlayer("My Trading Co.", "core_startup_company_001", false);

    // Run 10 ticks
    for (int i = 0; i < 10; i++) {
        augustus_engine::EngineController::instance().step();
        std::cout << "Tick " << (i + 1) << " completed\n";

        // Read logs
        for (auto &log : augustus_engine::EngineController::instance().readConsole())
            std::cout << "  [LOG] " << log << "\n";
    }

    // Show final player state
    std::cout << "\n--- Player State ---\n";
    std::cout << augustus_engine::EngineController::instance().getPlayerState() << "\n";

    // Save game
    augustus_engine::EngineController::instance().saveGame("demo_save");

    return 0;
}
