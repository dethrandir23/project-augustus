// Map.h
#pragma once

#include "Game/Gamestate.h"
#include "Game/IdUtils.h"
#include "uuid/uuid.h"
#include <unordered_map>

// tur dosyalari gecici olarak ayni yerde, sonra types.h falan tasinir.
struct Location {
    double x = 0;
    double y = 0;

    Location(double x, double y) : x(x), y(y) {}
    Location() = default;

    bool operator==(const Location& other) const {
        return x == other.x && y == other.y;
    }
};

struct LocationHash {
    std::size_t operator()(const Location& loc) const {
        return std::hash<double>()(loc.x) ^ (std::hash<double>()(loc.y) << 1);
    }

};

// component gecici olarak burada, tasarimi bitince tasinacak kendi dosyasina
// note: ileride location ozelligi n dimension olarak genericlesebilir. boylece bir daha boyut eklemeyle vb. ugrasilmaz.
class LocationComponent : public Component {
public:
    LocationComponent(double x, double y) {
        location = Location(x, y);
    }

    double GetX() const { return location.x; }
    double GetY() const { return location.y; }

    void SetX(double x) { location.x = x; }
    void SetY(double y) { location.y = y; }

    void SetPosition(double x, double y) {
        location.x = x;
        location.y = y;
    }

    void ResetPosition() {
        location.x = 0;
        location.y = 0;
    }

    void Move(double dx, double dy) {
        location.x += dx;
        location.y += dy;
    }

private:
    Location location;
};

class Map {
public:
    Map() = default;

    Map(int min_size, int max_size) : min_size(min_size), max_size(max_size) {
        id = IdUtils::generateUuid();
    }

    void SetSize(int min_size, int max_size) {
        this->min_size = min_size;
        this->max_size = max_size;
    }
    
    int GetMinSize() const { return min_size; }
    int GetMaxSize() const { return max_size; }

    void GetLocations(const Gamestate &gamestate) {
        for (const auto& [id, entity] : gamestate.getEntities()) {
            auto* marketComp = entity->GetComponent<LocationComponent>("MarketComponent");
            if (!marketComp) return;
            cached_locations[id] = Location(marketComp->GetX(), marketComp->GetY());
        }
    }

    void SetLocation(Gamestate &gamestate, uuids::uuid entityId, double x, double y) {

        if (x > max_size || x < min_size || y > max_size || y < min_size) {
            throw "Location out of bounds";
            return;
        }

        auto it = cached_locations.find(entityId);
        if (it != cached_locations.end()) {
            it->second.x = x;
            it->second.y = y;
        } else {
            cached_locations[entityId] = Location(x, y);
        }

        auto* entity = gamestate.getEntity(entityId);
        if (!entity) return;
        auto* locationComp = entity->GetComponent<LocationComponent>("LocationComponent");
        if (!locationComp) return;
        locationComp->SetPosition(x, y);

        if (!entity) return;
    }

    void MoveEntity(Gamestate &gamestate, uuids::uuid entityId, double dx, double dy) {
        auto it = cached_locations.find(entityId);
        if (it != cached_locations.end()) {

            if (it->second.x + dx > max_size || it->second.x + dx < min_size || it->second.y + dy > max_size || it->second.y + dy < min_size) {
                throw "Location out of bounds";
                return;
            }

            it->second.x += dx;
            it->second.y += dy;
        }

        auto* entity = gamestate.getEntity(entityId);
        if (!entity) return;
        auto* locationComp = entity->GetComponent<LocationComponent>("LocationComponent");
        if (locationComp) {
            locationComp->Move(dx, dy);
        }
    }

    double GetDistance(const Gamestate &gamestate, uuids::uuid entityId1, uuids::uuid entityId2) {
        auto it1 = cached_locations.find(entityId1);
        auto it2 = cached_locations.find(entityId2);
        if (it1 == cached_locations.end() || it2 == cached_locations.end()) {
            throw "Entity not found in cached locations";
            return -1;
        }

        double dx = it1->second.x - it2->second.x;
        double dy = it1->second.y - it2->second.y;
        return std::sqrt(dx * dx + dy * dy); // mutlak olsun diye carpiyoruz
        
    }

    void OptimiseCache(Gamestate &gamestate) {
        // if it has location in cached_locations but not in gamestate, remove it
        for (auto it = cached_locations.begin(); it != cached_locations.end();) {
            if (gamestate.getEntity(it->first) == nullptr) {
                it = cached_locations.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void ClearCache() {
        cached_locations.clear();
    }

private:
    uuids::uuid id;
    int max_size = 0;
    int min_size = 0;

    std::unordered_map<uuids::uuid, Location> cached_locations;

};