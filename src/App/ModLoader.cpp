#include "ModLoader.h"
#include "../../lib/nlohmann/json.hpp"
#include "../DevTools/Console.h"
#include "FileLoader.h"
#include <filesystem>
#include <optional>

std::vector<Mod> ModLoader::mods;
namespace fs = std::filesystem;

bool ModLoader::loadMod(const fs::path &root) {
  Mod mod;
  try {

    auto mod_json_opt = FileLoader::readFile(root / "mod.json");

    if (!mod_json_opt.has_value()) {
      Console::log("Failed to load mod.json", LogType::ERROR);
      return false;
    }

    auto j = json::parse(mod_json_opt.value().content);

    mod.id = j.at("id").get<std::string>();
    mod.name = j.at("name").get<std::string>();
    if (j.contains("author")) {
      mod.author = j.at("author").get<std::string>();
    }
    if (j.contains("version")) {
      mod.version = j.at("version").get<std::string>();
    }
    if (j.contains("description")) {
      mod.description = j.at("description").get<std::string>();
    }
    if (j.contains("gameVersion")) {
      mod.requiredGameVersion = j.at("gameVersion").get<std::string>();
    }

    if (j.contains("dependencies")) {
      for (const auto &dep : j["dependencies"]) {
        mod.dependency_ids.push_back(dep.get<std::string>());
      }
    }

    if (j.contains("optionalDependencies")) {
      for (const auto &dep : j["optionalDependencies"]) {
        mod.optional_dependency_ids.push_back(dep.get<std::string>());
      }
    }
  } catch (const std::exception &e) {
    Console::log("Error while loading mod: ", LogType::ERROR);
    Console::log(e.what(), LogType::ERROR);
    return false;
  }

  // load it's files
  std::vector<std::string> paths;
  for (const auto path : fs::recursive_directory_iterator(root)) {
    if (path.is_regular_file()) {
      paths.push_back(path.path().string());
    }
  }

  if (!FileLoader::loadFiles(paths)) {
    Console::log("Failed to load mods: ", LogType::ERROR);
    return false;
  }

  mods.push_back(mod);
  return true;
}
bool ModLoader::loadMods(const std::filesystem::path &load_order) {
  // if one fails, all of them fails
  auto load_order_json_opt = FileLoader::readFile(load_order);

  // Security
  if (!load_order_json_opt.has_value()) {
    Console::log("Failed to load load_order.json", LogType::ERROR);
    return false;
  }

  if (load_order_json_opt.value().name != "load_order") {
    Console::log("This is not a load_order.json file", LogType::ERROR);
    return false;
  }

  if (load_order_json_opt.value().extension != ".json") {
    Console::log("load_order.json is not a JSON file", LogType::ERROR);
    return false;
  }

  auto load_order_json = json::parse(load_order_json_opt.value().content);

  for (const auto &mod : load_order_json) {
    std::string root_path = mod.at("root_path").get<std::string>();
    if (!loadMod(root_path)) {
      return false;
    }
    Console::log("Loaded mod: " + root_path, LogType::INFO);
  }
  return true;
}

bool ModLoader::isModLoaded(const std::string &id) {
  for (const auto &mod : mods) {
    if (mod.id == id) {
      return true;
    }
  }
  return false;
}
bool ModLoader::reloadMods() {
  // implement later ( tags TODO #TODO TO DO #TO DO )
  // clear files from cache in FileLoader
  // problem is unload data is not implemented in engine yet.
  return true;
}

bool ModLoader::insertDataIntoEngine(
    std::function<bool(const std::vector<std::string> &file_contents,
                       const std::vector<std::string> &file_names)>
        callback) {

  std::vector<std::string> json_contents;
  std::vector<std::string> json_names;

  // --- FIX 3: Sadece JSON dosyarini Engine'e gonderiyoruz! ---
  // Resimler (PNG, JPG) JSON parser'i patlatir.
  for (const auto &file : FileLoader::getCachedFiles()) {
    if (file.extension == ".json") {
      if (file.name == "mod" || file.name == "load_order") {
        continue;
      }

      json_contents.push_back(file.content);
      json_names.push_back(file.name); // Sadece dosya adi (ornek: "items")
    }
  }

  if (json_contents.empty()) {
    Console::log("Warning: No JSON files found to insert into engine.",
                 LogType::WARNING);
    return true;
  }

  return callback(json_contents, json_names);
}

std::optional<Mod> ModLoader::getModInfo(const std::string &id) {
  for (const auto &mod : mods) {
    if (mod.id == id) {
      return mod;
    }
  }
  return std::nullopt;
}
std::vector<Mod> ModLoader::getMods() { return mods; }
size_t ModLoader::getModCount() { return mods.size(); }