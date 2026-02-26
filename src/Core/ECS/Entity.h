// Entity.h
#pragma once

#include <string>
#include <unordered_map>
#include "uuid/uuid.h"
#include "Component.h"

class Entity {
public:
    Entity() {}

    virtual ~Entity();

    void AddComponent(Component* component, const std::string& customKey = "");

    template <typename T>
    T* GetComponent(const std::string& key = "");

    uuids::uuid GetId() const;
    void SetId(const uuids::uuid& newId);

    std::string GetName() const;
    void SetName(const std::string& newName);

    const std::unordered_map<std::string, Component*>& GetAllComponents() const;

    void SetType(std::string t);
    std::string GetType() const;

    friend void to_json(nlohmann::json& j, const Entity& e);
    friend Entity from_json(const nlohmann::json& j);

private:
    uuids::uuid id;
    std::string name;
    std::string type;
    
    std::unordered_map<std::string, Component*> components;
};