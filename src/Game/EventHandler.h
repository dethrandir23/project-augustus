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

private:
  std::vector<Event> eventQueue;

  std::map<int, EventCallback> subscribers;
  int nextSubscriptionId = 0;
  int nextEventId = 0;

  std::unordered_map<std::string, int> eventHistory;
};