// EventHandler.cpp

#include "EventHandler.h"
#include "../../lib/nlohmann/json.hpp"
#include "../Registry/EventManager.h"
#include "Gamestate.h"
#include "InputHandler.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;

bool checkTriggerCondition(Gamestate &state, const Trigger &trigger) {
  // Burası senin oyun mantığına göre dolacak kanka.
  // Örnekler:
  if (trigger.type == "CHANCE") {
    // Value 0.05 ise %5 şans
    if (std::holds_alternative<double>(*trigger.value)) {
      double chance = std::get<double>(*trigger.value);
      // Basit bir random check (0.0 ile 1.0 arası)
      return ((double)rand() / (RAND_MAX)) < chance;
    }
  } else if (trigger.type == "SABOTAGE") {
    // return state.hasSabotageHappened(); NOT IMPLEMENTED YET
  } else if (trigger.type == "TURN_GREATER_THAN") {
    if (std::holds_alternative<int>(*trigger.value)) {
      return state.getCurrentTurn() > std::get<int>(*trigger.value);
    }
  }

  return false;
}

int EventHandler::subscribe(EventCallback callback) {
  int subscriptionId = nextSubscriptionId++;
  subscribers[subscriptionId] = callback;
  return subscriptionId;
}
void EventHandler::unsubscribe(int subscriptionId) {
  subscribers.erase(subscriptionId);
}

/*
    future performance note, provided for 60 fps game:


    static int checkTimer = 0;
    checkTimer++;

    // 1: Sadece her 10 tickte bir (veya saniyede bir) trigger kontrolü yap
    if (checkTimer >= 10) {
        checkTimer = 0;

        // ... Buraya yukarıdaki "Gulp Gulp" for döngüsü gelecek ...
    }


*/

void EventHandler::tickEvents(Gamestate &gamestate) {
  // four steps

  // 1: Push new events that their conditions meet
  for (const auto &pair : EventManager::events) {
    const auto &tmpl = pair.second;

    // Zaten kuyrukta olan (active) eventleri tekrar tetiklememek lazım
    // (Bunun için ayrı bir kontrol eklenebilir ama şimdilik geçiyorum)

    bool shouldTrigger = false;

    // Trigger Groups Logic: (Group1 OR Group2 OR ...)
    for (const auto &group : tmpl.triggerGroups) {
      bool groupMet = true;

      // Group Logic: (Condition1 AND Condition2 AND ...)
      for (const auto &trigger : group) {
        if (!checkTriggerCondition(gamestate, trigger)) {
          groupMet = false;
          break; // Bu grup yattı, diğer gruba bak
        }
      }

      if (groupMet) {
        shouldTrigger = true;
        break; // Bir grup tuttuysa yeterli!
      }
    }

    if (shouldTrigger) {
      this->pushEvent(tmpl.id);
    }
  }

  // 2: Tick remained times of events
  for (auto &event : eventQueue) {
    event.remained_steps--;
  }

  // 3: Auto-decide events has <= 0 remained time.
for (auto &event : eventQueue) {
    // Süresi bitti
    if (event.remained_steps <= 0) {
      
      // Eğer otomatik cevaplanacaksa cevapla
      if (event.tmpl->auto_handle) {
          event.handled = true;
          auto option = event.tmpl->options[event.tmpl->auto_handle_option_index];
          for (const auto &input : option.inputs) {
            InputHandler::handleInput(gamestate, input);
          }
      } 
      // Eğer auto_handle FALSE ise, event sadece kaybolur (handled işaretlenir ama input çalışmaz)
      else {
          event.handled = true; // Sadece listeden silinsin diye işaretliyoruz
      }
    }
  }
  // 4: Pop events that has handled flag
  for (auto it = eventQueue.begin(); it != eventQueue.end();) {
    if (it->handled) {
      it = eventQueue.erase(it);
    } else {
      ++it;
    }
  }
}

void EventHandler::clearQueue() { eventQueue.clear(); }

const std::vector<Event> &EventHandler::getQueue() const { return eventQueue; }

void EventHandler::pushEvent(const std::string &eventId) {
  if (EventManager::events.find(eventId) != EventManager::events.end()) {
    const EventTemplate &tmpl = EventManager::events.at(eventId);
    nextEventId++;

    Event newEvent(&tmpl, nextEventId);
    eventQueue.push_back(newEvent);

    for (const auto &[id, callback] : subscribers) {
      callback(newEvent);
    }
  }
}

void EventHandler::popEvent(int eventId) {
  eventQueue.erase(std::remove_if(eventQueue.begin(), eventQueue.end(),
                                  [eventId](const Event &event) {
                                    return event.id == eventId;
                                  }),
                   eventQueue.end());
}