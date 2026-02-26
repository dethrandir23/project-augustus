#pragma once
#include <string>
#include "nlohmann/json.hpp"

class Component {
public:
    virtual ~Component() = default;

    virtual std::string GetComponentType() const = 0;

    virtual nlohmann::json ToJson() const = 0;

    virtual void UpdateFromJson(const nlohmann::json& j) = 0;
};