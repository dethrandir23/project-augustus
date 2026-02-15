// EventManager.h
#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../DevTools/Console.h"
#include <string>
#include <variant>
#include <vector>

using json = nlohmann::json;

enum class EventType { NOTIFICATION, DECISION };

inline std::string eventTypeToString(EventType type) {
  switch (type) {
  case EventType::NOTIFICATION:
    return "NOTIFICATION";
  case EventType::DECISION:
    return "DECISION";
  default:
    return "UNKNOWN";
  }
}

inline EventType stringToEventType(const std::string &str) {
  std::string s = str;
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  if (s == "NOTIFICATION") {
    return EventType::NOTIFICATION;
  } else if (str == "DECISION") {
    return EventType::DECISION;
  } else {
    return EventType::NOTIFICATION;
  }
}

struct Trigger {
  std::string type;
  std::optional<std::variant<int, double, std::string, bool>> value;
};

struct Option {
  std::string text;
  std::vector<json> inputs;
};

struct EventTemplate {
  std::string id;
  std::string name;
  EventType eventType;
  std::string description;
  bool auto_handle;
  size_t auto_handle_option_index;
  size_t max_steps_until_handle;

  bool unique = false;
  int cooldown = 0;

  std::vector<std::string> scope;
  std::vector<std::vector<Trigger>> triggerGroups;
  std::vector<Option> options;
};

class EventManager {
public:
  static inline std::unordered_map<std::string, EventTemplate> events;

  static void load_from_json(const json &j, const std::string &source) {
    try {
      for (const auto &entry : j.at("entries")) {
        EventTemplate e;
        e.id = entry.at("id").get<std::string>();
        e.name = entry.at("name").get<std::string>();
        e.eventType =
            stringToEventType(entry.at("event_type").get<std::string>());
        e.description = entry.value("description", "No description provided.");
        e.auto_handle = entry.value("auto_handle", false);
        e.auto_handle_option_index = entry.value("auto_handle_option_index", 0);
        e.max_steps_until_handle = entry.value("max_steps_until_handle", 0);

        // EventManager::load_from_json içinde:
        e.unique = entry.value("unique", false);
        e.cooldown = entry.value("cooldown", 0);

        if (entry.contains("scope")) {
          e.scope = entry.at("scope").get<std::vector<std::string>>();
        }

        if (entry.contains("triggers")) {
          for (const auto &groupJson : entry["triggers"]) {
            std::vector<Trigger> currentGroup;

            auto parseTrigger = [](const json &trigJson) -> Trigger {
              Trigger t;
              t.type = trigJson.at("type").get<std::string>();

              if (trigJson.contains("value")) {
                const auto &v = trigJson.at("value");
                if (v.is_boolean()) {
                  t.value = v.get<bool>();
                } else if (v.is_string()) {
                  t.value = v.get<std::string>();
                } else if (v.is_number_integer()) {
                  t.value = v.get<int>();
                } else if (v.is_number_float()) {
                  t.value = v.get<double>();
                }
              }
              return t;
            };

            if (groupJson.is_array()) {
              for (const auto &trigJson : groupJson) {
                currentGroup.push_back(parseTrigger(trigJson));
              }
            } else {
              currentGroup.push_back(parseTrigger(groupJson));
            }

            if (!currentGroup.empty()) {
              e.triggerGroups.push_back(currentGroup);
            }
          }
        }

        if (entry.contains("options")) {
          for (const auto &option : entry["options"]) {
            Option o;
            o.text = option.at("text").get<std::string>();
            o.inputs = option.at("inputs").get<std::vector<json>>();
            e.options.push_back(o);
          }
        }

        events.emplace(e.id, e);
      }

      Console::log("Loaded " + std::to_string(events.size()) + " events from " +
                       source,
                   LogType::INFO);

    } catch (const std::exception &e) {
      Console::log("Event Load Error in " + source + ": " + e.what(),
                   LogType::ERROR);
    }
  }

private:
private:
  static void parse_trigger_value(const json &j, Trigger &t) {
    if (!j.contains("value"))
      return;

    const auto &val = j.at("value");

    if (val.is_boolean()) {
      t.value = val.get<bool>();
    } else if (val.is_string()) {
      t.value = val.get<std::string>();
    } else if (val.is_number_integer()) {
      t.value = val.get<int>();
    } else if (val.is_number_float()) {
      t.value = val.get<double>();
    }
  }
};