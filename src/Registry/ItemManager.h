#pragma once

#include "../../lib/nlohmann/json.hpp"
#include <string>
#include <vector>

struct ItemData {
  std::string id;
  std::string name;
  std::string description;
  std::vector<std::string> categories;
  size_t base_price;
};

class ItemManager {
public:
  static inline std::unordered_map<std::string, ItemData> items;

  static void load_from_json(const nlohmann::json &j,
                             const std::string &source) {
    try {
      auto items_array = j.at("entries");
      for (const auto &item : items_array) {
        std::string id = item.at("id").get<std::string>();
        std::string name = item.at("name").get<std::string>();
        std::string description_id = item["description"];
        std::vector<std::string> categories = item["categories"];
        size_t base_price = item.value("base_price", 0);

        // SPECIAL NOTE ( TODO ): LINK DESC ID TO LOCALE FILE AND TAKE LOCALE
        // FILE AS PARAMETER OR FIND ANOTHER SOLUTITON FOR IT, MAYBE CALL FUNC
        // LIKE (GET LOCALES (BLA BLA)) ETC.
        items[id] = {id, name, description_id, categories, base_price};
      }
    } catch (std::exception &e) {
      // std::cout << "Error while loading file " << source << " because: " <<
      // e.what << std::endl;
    }
  }
};