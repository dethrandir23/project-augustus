#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "../Registry/MapManager.h"

using ColorID = uint32_t;

class Map {
public:
    Map() = default;

    // Haritayı başlatır (Pikselleri JS'den alır, Tanımı Manager'dan çeker)
    void init(const std::string& mapId, const std::vector<uint32_t>& pixelData);

    // Koordinattaki Node Instance ID'sini verir (örn: "core_debug_london")
    std::string getNodeInstanceAt(int x, int y) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    std::string getId() const { return mapId; }

private:
    std::string mapId;
    int width = 0;
    int height = 0;

    // Pikseller (Renk haritası)
    std::vector<uint32_t> pixels;
    
    // Renk -> Node Instance ID eşleştirmesi
    // Bunu ileride "provinces.png" kullanırsak dolduracağız.
    // Şimdilik senin "JSON'da koordinat var" mantığına göre;
    // Her pikseli tutmak yerine, tıklanan yere en yakın şehri de bulabiliriz.
    // AMA sen "renk dağılımı" istediğin için bu yapıyı koruyorum.
    std::unordered_map<ColorID, std::string> colorToNodeMap;
};