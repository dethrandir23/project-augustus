#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <optional>

struct CachedNode {
    std::string instance_id;
    int x, y;
    float radius = 10.0f;
};

class Map {
public:
    Map() = default;

    void init(const std::string& mapId);

    std::string getNodeInstanceAt(int x, int y) const;

    float getDistance(const std::string& nodeId1, const std::string& nodeId2) const;

    // Getterlar
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    std::string getId() const { return mapId; }

private:
    std::string mapId;
    int width = 0;
    int height = 0;

    static constexpr int CELL_SIZE = 50; 
    int gridCols = 0;
    int gridRows = 0;

    std::vector<std::vector<int>> grid; 

    std::vector<CachedNode> localNodes;
    
    int getCellIndex(int x, int y) const;
};