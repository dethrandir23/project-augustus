// ConsoleUtils.h
/**
* @file ConsoleUtils.h
* @brief Provides various utility functions for working with the console.
* 
* This header file contains various utility functions for working with the console.
* 
* @author Bekir Efe Öztürk (@dethrandir23)
* @date 10.02.2026
* @license MIT
* @version 1.0.0
* @copyright Copyright (c) 2026 Bekir Efe Öztürk
*/
#pragma once
#include "DevTools/Console.h"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

// Forward declaration'lara artık gerek kalmayabilir ama tip güvenliği için dursun
enum class LogType;
struct Log;

enum class ExportFormat {
    TXT,
    JSON,
    CSV
};

class ConsoleUtils {
public:
    ConsoleUtils() = delete;

    // Console referansı parametrelerini sildik
    static void printConsole();
    static void printConsole(bool timestamp, bool type);

static nlohmann::json logToJson(const Log& log) {
    return nlohmann::json{
        {"timestamp", log.timestamp},
        {"type", logTypeToStr(log.logType)},
        {"message", log.message}
    };
}

static bool saveConsoleToFile(const std::string& filename, ExportFormat format);

static void saveConsoleToFile(const fs::path &filename, ExportFormat format);

static fs::path saveConsoleToTempFile(ExportFormat format);

    static bool loadConsoleFromFile(const std::string& filename);
    static bool loadConsoleFromFile(const fs::path &filename);

    static std::vector<Log> selectLogsByType(LogType type);
    static std::vector<Log> selectLogsByTimestamp(const std::chrono::system_clock::time_point &firstTime, const std::chrono::system_clock::time_point &lastTime);
    static std::vector<Log> selectLogsByMessage(const std::string& message);
};