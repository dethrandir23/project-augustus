#pragma once
#include <string>
#include <memory>
#include "nlohmann/json.hpp"

class Entity;
class Gamestate;

class AIBrain {
public:
    virtual ~AIBrain() = default;

    virtual std::string getType() const = 0;

    virtual void execute(Entity& entity, Gamestate& gamestate) = 0;

    virtual nlohmann::json toJson() const;

    virtual void fromJson(const nlohmann::json& j);
};
