// Map.h
#pragma once

#include "Game/IdUtils.h"
#include "uuid/uuid.h"
#include <unordered_map>
#include "Core/Types.h"

class Gamestate;

class Map {
public:
    Map() = default;

    Map(int min_size, int max_size);

    void SetSize(int min_size, int max_size);

    int GetMinSize() const;
    int GetMaxSize() const;

    void GetLocations(const Gamestate &gamestate);
    void SetLocation(Gamestate &gamestate, uuids::uuid entityId, double x, double y);

    void MoveEntity(Gamestate &gamestate, uuids::uuid entityId, double dx, double dy);

    double GetDistance(const Gamestate &gamestate, uuids::uuid entityId1, uuids::uuid entityId2);

    void OptimiseCache(Gamestate &gamestate);
    
    void ClearCache();

private:
    uuids::uuid id;
    int max_size = 0;
    int min_size = 0;

    std::unordered_map<uuids::uuid, Location> cached_locations;

};