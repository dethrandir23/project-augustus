#pragma once

#include <chrono>
#include <string>
#include <vector>

enum class LogType { INFO, ERROR, WARNING };

struct Log {
  std::string message;
  LogType logType;
  std::string timestamp;

  Log() {}

  Log(const std::string &msg, LogType type) : message(msg), logType(type) {
    auto now = std::chrono::system_clock::now();
    timestamp = std::chrono::system_clock::to_time_t(now);
  }
};

class Console {
public:
  Console() = delete;

  static std::string help();
  static void logHelp();
  static std::string parseInput(const std::string &input);

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