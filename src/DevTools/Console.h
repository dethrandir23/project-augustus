// Console.h
#pragma once

#include "nlohmann/json.hpp"
#include <chrono>
#include <iomanip>
#include <string>
#include <vector>

enum class LogType { INFO, ERROR, WARNING };

std::string logTypeToStr(LogType type);
LogType strToLogType(const std::string &str);

struct Log {
    std::string message;
    LogType logType;
    std::string timestamp;

    Log() {}
    
    Log(const std::string &msg, LogType type) : message(msg), logType(type) {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        timestamp = ss.str();
    }

    Log(const std::string& ts, LogType type, const std::string& msg) 
        : message(msg), logType(type), timestamp(ts) {}

friend std::ostream& operator<<(std::ostream& os, const Log& log) {
  return os << "[" << log.timestamp << "] " << "[" << logTypeToStr(log.logType) << "] " << log.message;
}

};

class Console {
public:
  Console() = delete;

  static std::string help();
  static void logHelp();
  static nlohmann::json parseInput(const std::string &input);

  static void log(const std::string &s);
  static void log(const std::string &s, LogType type);
  static void log(Log log);

  static std::string readLog(size_t index);
  static Log getLog(size_t index);

  static void clearLogs();
  static void removeLog(size_t index);
  static size_t getLogCount();

  static std::vector<Log> getLogs();
  static std::vector<std::string> getLogMessages();

private:
  static std::vector<Log> logs;
};