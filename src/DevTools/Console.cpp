#include "Console.h"
#include "../../lib/nlohmann/json.hpp"
#include <ctime>
#include <exception>
#include <stdexcept>

// FOR FUTURE: SEPERATE GAME AND DEBUG CONSOLE, RETURN DEBUG MESSAGES IF ONLY
// DEBUG MODE ENABLED OR ADD AN PARAMETER TO GET ONLY NOT DEBUG OR WITH DEBUG OR
// ONLY DEBUG LOGS

/*
        EXAMPLE INPUTS (CHEAT CODES)

-   add_money xxx
-   add_item item_id xxx
-   step_game xxx

*/

std::vector<Log> Console::logs;

std::string logTypeToStr(LogType type) {
  switch (type) {
  case LogType::INFO:
    return "INFO";
  case LogType::ERROR:
    return "ERROR";
  case LogType::WARNING:
    return "WARNING";
  default:
    return "UNKNOWN";
  }
}

std::string Console::help() {
  // raw string
  std::string help = R"(
    Available commands are:
    add_money <amount>
    add_item <item_id> <amount>
    step_game <times>
    help
  )";
  return help;
}

void Console::logHelp() { Console::log(help(), LogType::INFO); }

std::string Console::parseInput(const std::string &input) {
  try {
    if (input.empty())
      return "";

    std::vector<std::string> tokens;
    std::string token;
    std::stringstream tokenStream(input);
    while (std::getline(tokenStream, token, ' ')) {
      tokens.push_back(token);
    }

    if (tokens.empty())
      return "";

    std::string command = tokens[0];
    std::vector<std::string> args;
    for (size_t i = 1; i < tokens.size(); ++i) {
      args.push_back(tokens[i]);
    }

    nlohmann::json j;

    if (command == "add_money") {
      if (args.size() != 1) {
        Console::log("Usage: add_money <amount>", LogType::ERROR);
      } else {
        try {
          double amount = std::stod(args[0]);
          if (amount >= 0) {
            j["type"] = "ADD_MONEY";
            j["payload"]["amount"] = amount;
            return j.dump();
          } else {
            Console::log("Invalid amount: " + args[0], LogType::ERROR);
          }
        } catch (const std::invalid_argument &e) {
          Console::log("Invalid amount: " + args[0], LogType::ERROR);
        } catch (const std::exception &e) {
          Console::log(e.what(), LogType::ERROR);
        }
      }
    } else if (command == "step_game") {
      if (args.size() != 1) {
        Console::log("Usage: step_game <times>", LogType::ERROR);
      } else {
        try {
          int times = std::stoi(args[0]);
          if (times >= 0) {
            j["type"] = "STEP_GAME";
            j["payload"]["times"] = times;
            return j.dump();
          } else {
            Console::log("Invalid times: " + args[0], LogType::ERROR);
          }
        } catch (const std::invalid_argument &e) {
          Console::log("Invalid times: " + args[0], LogType::ERROR);
        } catch (const std::exception &e) {
          Console::log(e.what(), LogType::ERROR);
        }
      }

    } else {
      Console::log("Unknown command: " + command, LogType::ERROR);
    }

  } catch (const std::exception &e) {
    Console::log(e.what(), LogType::ERROR);
    return "";
  }
  return "";
}

void Console::log(const std::string &s) {
  logs.push_back(Log(s, LogType::INFO));
}
void Console::log(const std::string &s, LogType type) {
  logs.push_back(Log(s, type));
}
void Console::log(Log log) { logs.push_back(log); }

std::string Console::readLog(size_t index) {
  if (index < logs.size()) {
    return logs[index].message;
  }
  return "";
}
Log Console::getLog(size_t index) {
  if (index < logs.size()) {
    return logs[index];
  }
  return Log();
}

void Console::clearLogs() { logs.clear(); }
void Console::removeLog(size_t index) {
  if (index < logs.size()) {
    logs.erase(logs.begin() + index);
  }
}
size_t Console::getLogCount() { return logs.size(); }

std::vector<Log> Console::getLogs() { return logs; }
std::vector<std::string> Console::getLogMessages() {
  std::vector<std::string> messages;
  for (const auto &log : logs) {
    messages.push_back(log.message);
  }

  return messages;
}