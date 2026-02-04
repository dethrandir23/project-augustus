#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"

class GameLoader {
public:
    using LoadHandler = std::function<void(const nlohmann::json&, const std::string&)>;
    
    void RegisterHandler(const std::string& type_name, const LoadHandler& handler) {
        handlers[type_name] = handler;
    }

    bool load_file_content(const std::string &file_content, const std::string &file_name) {
        try {
            auto j = nlohmann::json::parse(file_content);
            if (!j.contains("registry_type")) {
                // std::cerr << JSON Parse Error: e.what() << std::endl;
                return false;
            }

            std::string type = j["registry_type"];
            if (handlers.find(type) != handlers.end()) {
                handlers[type](j, file_name);
                return true;
            } else {
                // std::cerr << "Warning: Unknown registry type " << type << " in file " << file_name
            }

        } catch (const std::exception &e) {
            // std::cerr << JSON Parse Error: << e.what() << std::endl;
            return false;
        }
        return false;
    }

private:
    std::unordered_map<std::string, LoadHandler> handlers;
};