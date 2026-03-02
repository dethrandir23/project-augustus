// ModLoader.h
#pragma once

#include "../../lib/nlohmann/json.hpp"
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;

struct Mod {
    std::string id;
    std::string name;
    std::optional<std::string> author;
    std::optional<std::string> version;
    std::optional<std::string> description;
    std::optional<std::string> requiredGameVersion;

    std::vector<std::string> dependency_ids;
    std::vector<std::string> optional_dependency_ids;

    bool is_loaded = false;
};

class ModLoader {
public:
  ModLoader() = delete;

  static bool loadMod(const std::filesystem::path &root);
  static bool loadMods(const std::filesystem::path &load_order);

  static bool isModLoaded(const std::string &id);
  static bool reloadMods();

  static bool insertDataIntoEngine(std::function<bool(const std::vector<std::string> &file_contents,
                   const std::vector<std::string> &file_names)> callback);

  static std::optional<Mod> getModInfo(const std::string &id);
  static std::vector<Mod> getMods();
  static size_t getModCount();

private:

    static std::vector<Mod> mods;
};