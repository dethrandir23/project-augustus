/**
 * @file ConfigUtils.cpp
 * @author Bekir Efe Öztürk (@dethrandir23)
 * @brief Implementation of ConfigUtils class.
 * @version 0.1
 * @date 2025-02-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "toml/toml.hpp"
#include "ConfigUtils.h"
#include <iostream>
#include <fstream>

toml::table ConfigUtils::loadConfig(const std::string& filepath) {
    try {
        return toml::parse_file(filepath);
    } catch (const toml::parse_error& err) {
        std::cerr << "Error parsing config file '" << filepath << "':\n" << err << std::endl;
        return toml::table{};
    } catch (const std::exception& err) {
        std::cerr << "Error loading config file '" << filepath << "': " << err.what() << std::endl;
        return toml::table{};
    }
}

bool ConfigUtils::saveConfig(const toml::table& config, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filepath << std::endl;
        return false;
    }
    file << config;
    return !file.fail();
}

toml::table* ConfigUtils::getOrCreateTable(toml::table& config, std::string_view path) {
    toml::table* current = &config;
    size_t start = 0;
    while (start < path.length()) {
        size_t end = path.find('.', start);
        if (end == std::string_view::npos) end = path.length();
        
        std::string_view segment = path.substr(start, end - start);
        
        if (!current->contains(segment)) {
            current->insert(segment, toml::table{});
        }
        
        auto node = current->get(segment);
        if (auto tbl = node->as_table()) {
            current = tbl;
        } else {
            return nullptr; // Path blocked by non-table
        }
        
        start = end + 1;
    }
    return current;
}
