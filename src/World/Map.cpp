#include "Map.h"
#include "../Registry/MapManager.h"
#include <iostream>

void Map::init(const std::string &mId) {
    if (!MapManager::maps.count(mId)) {
        std::cerr << "Map ID not found: " << mId << std::endl;
        return;
    }

    const auto &def = MapManager::maps.at(mId);
    this->mapId = mId;
    this->width = def.width;
    this->height = def.height;

    // 1. Grid Boyutlarını Hesapla
    // Tavan yuvarlama yapıyoruz ki harita kenarları taşmasın
    this->gridCols = (width + CELL_SIZE - 1) / CELL_SIZE;
    this->gridRows = (height + CELL_SIZE - 1) / CELL_SIZE;
    
    // Grid'i sıfırla ve boyutlandır
    grid.clear();
    grid.resize(gridCols * gridRows);
    localNodes.clear();

    // 2. Node'ları Grid'e Yerleştir (Pre-processing)
    int nodeIdx = 0;
    for (const auto &nodeDef : def.nodes) {
        // Local Cache'e ekle
        CachedNode cNode;
        cNode.instance_id = nodeDef.instance_id;
        cNode.x = nodeDef.x;
        cNode.y = nodeDef.y;
        localNodes.push_back(cNode);

        // Hangi hücreye denk geliyor?
        int cellX = nodeDef.x / CELL_SIZE;
        int cellY = nodeDef.y / CELL_SIZE;

        // Sınır kontrolü (Map dışına node koyulmuşsa patlamasın)
        if (cellX >= 0 && cellX < gridCols && cellY >= 0 && cellY < gridRows) {
            int gridIndex = cellY * gridCols + cellX;
            grid[gridIndex].push_back(nodeIdx);
        }
        
        // Kenar durumu: Node tam çizgi üzerindeyse komşu hücrelere de ekleyebilirsin
        // ama şimdilik basit tutalım.
        
        nodeIdx++;
    }
    
    // std::cout << "Map Initialized via Spatial Grid. Cells: " << grid.size() << std::endl;
}

std::string Map::getNodeInstanceAt(int x, int y) const {
    // Harita dışı kontrolü
    if (x < 0 || y < 0 || x >= width || y >= height) return "";

    // 1. Hangi hücredeyiz?
    int cellX = x / CELL_SIZE;
    int cellY = y / CELL_SIZE;
    int gridIndex = cellY * gridCols + cellX;

    // 2. Sadece o hücredeki node'ları kontrol et (Optimization)
    if (gridIndex < grid.size()) {
        const auto& candidateIndices = grid[gridIndex];
        
        for (int idx : candidateIndices) {
            const auto& node = localNodes[idx];
            
            // Mesafe hesabı (Squared Distance daha hızlıdır, sqrt gerekmez)
            int dx = x - node.x;
            int dy = y - node.y;
            float distSq = (float)(dx*dx + dy*dy);
            
            // 10 birim yarıçap -> karesi 100
            if (distSq <= (node.radius * node.radius)) {
                return node.instance_id;
            }
        }
    }

    return "";
}

float Map::getDistance(const std::string& nodeId1, const std::string& nodeId2) const {
    // Burada basit bir lookup lazım, şimdilik O(N) yapıyorum ama
    // normalde id -> coordinate map'i de tutabilirsin.
    // Lojistik hesaplamaları için burası kritik olacak.
    
    // ... implementation ...
    return 0.0f;
}