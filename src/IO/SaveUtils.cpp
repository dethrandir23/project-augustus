// SaveUtils.cpp
#include "DevTools/Console.h"
#include "Game/Gamestate.h"
#include "FileUtils.h"
#include "DevTools/ConfigUtils.h"
#include <algorithm>

namespace fs = std::filesystem;
using json = nlohmann::json;


// mock for now, for release generate from arguments instead.
constexpr const char *configPath = "Config.toml";
constexpr const char *error_no_path = "no_save_path";

/**
 * @namespace SaveUtils
 * @brief Utility functions for managing game save states, including serialization and compression.
 */
namespace SaveUtils {

    bool saveGame(const Gamestate &gamestate, const std::string &saveName) {
        if (saveName.empty()) return false;
        
auto config = ConfigUtils::loadConfig(configPath);
    
    // Debug için: Config yüklendi mi?
    if (config.empty()) {
        Console::log("FATAL: Config file NOT found at: " + std::string(configPath), LogType::ERROR);
        return false; 
    }

    std::string savePath = ConfigUtils::get<std::string>(config, "storage.save_path", "no_save_path");
        
    if (savePath == "no_save_path") {
        Console::log("FATAL: 'save_path' not found in config!", LogType::ERROR);
        return false;
    }

        try {
            if (!fs::exists(savePath)) {
                Console::log("Creating save path: " + savePath, LogType::INFO);
                fs::create_directories(savePath);
            }
            json j = serializeGamestate(gamestate);
            std::vector<uint8_t> data = FileUtils::compress(json::to_msgpack(j));
            
            fs::path fpath = fs::path(savePath) / saveName;
            FileUtils::writeFile(fpath.string() + ".save", data);
        } catch (const std::exception &e) {
            Console::log("Save error: " + std::string(e.what()), LogType::ERROR);
            return false;
        }
        return true;
        
    }

    bool loadGame(Gamestate &gamestate, const std::string &saveName) {
        if (saveName.empty()) return false;

        std::string savePath = ConfigUtils::get<std::string>(ConfigUtils::loadConfig(configPath),
                                                        "storage.save_path",
                                              "no_save_path");
        
        if (savePath == error_no_path) return false;

        try {
            fs::path fpath = fs::path(savePath) / (saveName + ".save");
            if (!FileUtils::exists(fpath.string())) return false;

            std::vector<uint8_t> compressedData = FileUtils::readFile(fpath.string());
            
            // FileUtils::decompress expects vector<int8_t>, cast needed based on FileUtils.h
            std::vector<uint8_t> decompressedData = FileUtils::decompress(compressedData);

            json j = json::from_msgpack(decompressedData);
            
            gamestate.clear();
            gamestate.loadFromSave(j);

            return true;
        } catch (const std::exception &e) {
            Console::log("Load error: " + std::string(e.what()), LogType::ERROR);
            return false;
        }
    }

    Gamestate loadGame(const std::string &saveName) {
        Gamestate gs;
        loadGame(gs, saveName);
        return gs;
    }

    std::vector<std::string> listSaves() {
        std::string savePath = ConfigUtils::get<std::string>(ConfigUtils::loadConfig(configPath),
                                                        "storage.save_path",
                                              "no_save_path");
        
        if (savePath == error_no_path) return {};

        auto paths = FileUtils::listFiles(savePath);

        std::for_each(paths.begin(), paths.end(), [](std::string &p){
            fs::path path = p;
            p = path.filename().string();
        });

        return paths;
    }
}