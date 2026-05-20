// EventHandler.h
#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../Registry/EventManager.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

class Gamestate;

using j = nlohmann::json;

struct Event {
  const EventTemplate *tmpl = nullptr;
  bool handled = false;
  int id;

  size_t remained_steps;

  Event(const EventTemplate *t, int id) : tmpl(t), id(id) {
    if (t)
      remained_steps = t->max_steps_until_handle;
    else
      remained_steps = 0;
  }

  nlohmann::json toJson() const {
    if (!tmpl) return {};
    nlohmann::json opts = nlohmann::json::array();
    for (size_t i = 0; i < tmpl->options.size(); ++i) {
        opts.push_back({{"index", i}, {"text", tmpl->options[i].text}});
    }
    return {
        {"id",              id},
        {"name",            tmpl->name},
        {"description",     tmpl->description},
        {"event_type",      tmpl->eventType},
        {"options",         opts},
        {"remained_steps",  remained_steps}
    };
}
};

using EventCallback = std::function<void(const Event &)>;

class EventHandler {
public:
  int subscribe(EventCallback callback);
  void unsubscribe(int subscriptionId);

  void tickEvents(Gamestate &gamestate);

  void clearQueue();

  const std::vector<Event> &getQueue() const;

  void pushEvent(const std::string &eventId);

  void popEvent(int eventId);

  bool canTrigger(const std::string &eventId, int currentTurn);
  void recordTrigger(const std::string &eventId, int currentTurn);

  void markAsHandled(int eventId);

private:
  std::vector<Event> eventQueue;

  std::map<int, EventCallback> subscribers;
  int nextSubscriptionId = 0;
  int nextEventId = 0;

  std::unordered_map<std::string, int> eventHistory;
};