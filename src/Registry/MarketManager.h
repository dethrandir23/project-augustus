#pragma once
#include "../../lib/nlohmann/json.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct MarketTemplate {
  std::string name;
  std::string id;
};

class MarketManager {
public:
  static inline std::unordered_map<std::string, MarketTemplate> markets;

  static void load_from_json(const nlohmann::json &j, const std::string &src) {
    try {
      for (const auto &entry : j.at("entries")) {
        MarketTemplate m;
        m.name = entry.at("name").get<std::string>();
        m.id = entry.at("id").get<std::string>();
        markets[m.id] = m;
      }
    } catch (const std::exception &e) {
      // std::cerr << "
      // Market Load Error in " << src << ": " << e.what() << std::endl;
    }
  }
};