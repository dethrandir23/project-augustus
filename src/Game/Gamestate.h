#pragma once
#include "World/Map.h"
#include "nlohmann/json.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/ECS/Entity.h"
#include "Game/EventHandler.h"
#include "Game/IdUtils.h"

class Gamestate {
public:
    Gamestate() = default;
    ~Gamestate() { clear(); } // ÖNEMLİ: Kapanırken entityleri RAM'den siler

    bool loadScenario(const std::string& scenarioId);
    void advanceDate();
    std::string getDateString() const;

    // --- YENİ ECS ENTITY YÖNETİMİ ---
    void addEntity(Entity* entity);
    Entity* getEntity(const uuids::uuid& id);
    void removeEntity(const uuids::uuid& id);
    
    // Tüm entityleri (System'ler için) döndürür
    const std::unordered_map<uuids::uuid, Entity*>& getEntities() const { return entities; }

    // Sadece belirli bir türdeki (örn: "company") entityleri getirir
    std::vector<Entity*> getEntitiesByType(const std::string& type);

    // --- Core Getters / Setters ---
    uuids::uuid getPlayerCompanyId() const { return playerCompanyId; }
    void setPlayerCompanyId(const uuids::uuid& id) { playerCompanyId = id; }
    
    int getCurrentTurn() const { return currentTurn; }
    int getCurrentYear() const { return currentYear; }
    int getCurrentMonth() const { return currentMonth; }
    int getCurrentDay() const { return currentDay; }

    int getGameSpeed() const { return gameSpeed; }
    void setGameSpeed(int speed) { gameSpeed = speed; }

    Map& getMap() { return map; }
    const Map& getMap() const { return map; }

    void clear();

    friend nlohmann::json serializeGamestate(const Gamestate& g);

    EventHandler& getEventHandler() { return eventHandler; }

    // Player Company artık bir Entity* dönüyor
    Entity* getPlayerCompany() {
        return getEntity(playerCompanyId);
    }

    int gameSpeed = 1;
    bool paused = true;
    float accumulator = 0.0f;
    void togglePause() { paused = !paused; }

private:
    int currentTurn = 0;
    int currentYear = 1836, currentMonth = 1, currentDay = 1;

    uuids::uuid playerCompanyId;

    // --- İŞTE O TEK LİSTE (THE MASTER LIST) ---
    std::unordered_map<uuids::uuid, Entity*> entities;
    Map map;

    std::unordered_map<std::string, uuids::uuid> instanceIdToUUID;

    EventHandler eventHandler;
};