#include "Map.h"
#include <cmath>

void Map::init(const std::string& mId, const std::vector<uint32_t>& pxData) {
    if (!MapManager::maps.count(mId)) return;
    
    const auto& def = MapManager::maps.at(mId);
    
    this->mapId = mId;
    this->width = def.width;
    this->height = def.height;
    this->pixels = pxData; // Pikselleri kopyala

    // NOT: Eğer JSON'da "Şu renk = Şu Node" diye bir tanım varsa burada map'i doldururuz.
    // Eğer yoksa ve sadece koordinat varsa (senin debug map gibi), 
    // pikseller sadece görsel arkaplan olur.
    
    // Şimdilik senin debug map'inde renk tanımı yok, sadece node koordinatı var.
    // İleride burayı "provinces.png" mantığına göre açarız.
}

std::string Map::getNodeInstanceAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return "";
    
    // 1. Yöntem: Renk Haritası (Provinces.png varsa)
    if (!pixels.empty()) {
        uint32_t color = pixels[y * width + x];
        if (colorToNodeMap.count(color)) {
            return colorToNodeMap.at(color);
        }
    }

    // 2. Yöntem: Koordinat bazlı (Debug Map için)
    // Bu çok yavaştır (O(N)), oyun büyüyünce Yöntem 1'e geçmeliyiz.
    if (MapManager::maps.count(mapId)) {
        const auto& def = MapManager::maps.at(mapId);
        for (const auto& node : def.nodes) {
            // Basit bir "Hitbox" kontrolü (örn: 5 piksel yarıçap)
            float dist = std::sqrt(std::pow(x - node.x, 2) + std::pow(y - node.y, 2));
            if (dist < 10.0f) { // 10 piksel yakınına tıklandıysa
                return node.instance_id;
            }
        }
    }

    return "";
}