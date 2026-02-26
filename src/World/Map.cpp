#include "Map.h"
#include "Game/Gamestate.h"
#include "Components/LocationComponent.h"

    Map::Map(int min_size, int max_size) : min_size(min_size), max_size(max_size) {
        id = IdUtils::generateUuid();
    }

    void Map::SetSize(int min_size, int max_size) {
        this->min_size = min_size;
        this->max_size = max_size;
    }

    int Map::GetMinSize() const { return min_size; }
    int Map::GetMaxSize() const { return max_size; }

    void Map::GetLocations(const Gamestate &gamestate) {
        for (const auto& [id, entity] : gamestate.getEntities()) {
            auto* marketComp = entity->GetComponent<LocationComponent>("MarketComponent");
            if (!marketComp) return;
            cached_locations[id] = Location(marketComp->GetX(), marketComp->GetY());
        }
    }

    void Map::SetLocation(Gamestate &gamestate, uuids::uuid entityId, double x, double y) {

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

    void Map::MoveEntity(Gamestate &gamestate, uuids::uuid entityId, double dx, double dy) {
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

    double Map::GetDistance(const Gamestate &gamestate, uuids::uuid entityId1, uuids::uuid entityId2) {
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

    void Map::OptimiseCache(Gamestate &gamestate) {
        // if it has location in cached_locations but not in gamestate, remove it
        for (auto it = cached_locations.begin(); it != cached_locations.end();) {
            if (gamestate.getEntity(it->first) == nullptr) {
                it = cached_locations.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void Map::ClearCache() {
        cached_locations.clear();
    }