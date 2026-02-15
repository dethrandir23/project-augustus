/**
 * @file ConfigUtils.h
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Utility class for handling TOML configuration files.
 * @version 0.1
 * @date 2025-02-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "toml/toml.hpp"
#include <string>
#include <string_view>

/**
 * @class ConfigUtils
 * @brief A static utility class for loading, saving, and manipulating configuration data using toml++.
 */
class ConfigUtils {
public:
  ConfigUtils() = delete;

  /**
   * @brief Loads configuration from a TOML file.
   * 
   * @param filepath The path to the configuration file.
   * @return toml::table The parsed configuration table. Returns an empty table on failure.
   */
  static toml::table loadConfig(const std::string& filepath);

  /**
   * @brief Saves configuration to a TOML file.
   * 
   * @param config The configuration table to save.
   * @param filepath The destination file path.
   * @return true If the file was successfully saved.
   * @return false If the file could not be opened or written to.
   */
  static bool saveConfig(const toml::table& config, const std::string& filepath);

  /**
   * @brief Helper to find or create a table at the given path.
   * 
   * Traverses the configuration table based on a dotted path (e.g., "server.logging").
   * Creates nested tables if they do not exist.
   * 
   * @param config The root configuration table.
   * @param path The dotted path string view.
   * @return toml::table* Pointer to the found or created table, or nullptr if a path segment exists but is not a table.
   */
  static toml::table* getOrCreateTable(toml::table& config, std::string_view path);

  /**
   * @brief Gets a value from the config.
   * 
   * Supports dotted paths (e.g., "server.port").
   * 
   * @tparam T The type of the value to retrieve.
   * @param config The configuration table.
   * @param key The key or dotted path to the value.
   * @param default_value The value to return if the key is missing or type mismatch occurs.
   * @return T The retrieved value or the default value.
   */
  template <typename T>
  static T get(const toml::table& config, const std::string& key, const T& default_value) {
      return config.at_path(key).value_or(default_value);
  }
  
  /**
   * @brief Alias for get().
   * 
   * @tparam T The type of the value to retrieve.
   * @param config The configuration table.
   * @param key The key or dotted path to the value.
   * @param default_value The value to return if the key is missing or type mismatch occurs.
   * @return T The retrieved value or the default value.
   */
  template <typename T>
  static T read(const toml::table& config, const std::string& key, const T& default_value) {
      return get(config, key, default_value);
  }

  /**
   * @brief Checks if a key exists in the configuration.
   * 
   * @param config The configuration table.
   * @param key The key or dotted path to check.
   * @return true If the key exists.
   * @return false If the key does not exist.
   */
  static bool has(const toml::table& config, const std::string& key) {
      return !!config.at_path(key);
  }

  /**
   * @brief Sets a value in the config.
   * 
   * Creates nested tables for dotted keys if they don't exist.
   * 
   * @tparam T The type of the value to set.
   * @param config The configuration table to modify.
   * @param key The key or dotted path where the value should be set.
   * @param value The value to set.
   */
  template <typename T>
  static void set(toml::table& config, const std::string& key, const T& value) {
      size_t last_dot = key.rfind('.');
      if (last_dot == std::string::npos) {
          config.insert_or_assign(key, value);
      } else {
          std::string_view table_path(key.data(), last_dot);
          std::string_view leaf_key(key.data() + last_dot + 1);
          if (auto tbl = getOrCreateTable(config, table_path)) {
              tbl->insert_or_assign(leaf_key, value);
          }
      }
  }

  /**
   * @brief Alias for set().
   * 
   * @tparam T The type of the value to set.
   * @param config The configuration table to modify.
   * @param key The key or dotted path where the value should be set.
   * @param value The value to set.
   */
  template <typename T>
  static void add(toml::table& config, const std::string& key, const T& value) {
      set(config, key, value);
  }
};