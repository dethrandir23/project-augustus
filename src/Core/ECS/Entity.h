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
    void RemoveComponent(const std::string& key);

    template <typename T>
    T* GetComponent(const std::string& key = ""){
        std::string searchKey = key;
        if (searchKey.empty()) {
             return nullptr; 
        }

        auto it = components.find(searchKey);
        if (it != components.end()) {
            return dynamic_cast<T*>(it->second);
        }
        return nullptr;
    }

    uuids::uuid GetId() const;
    void SetId(const uuids::uuid& newId);

    std::string GetName() const;
    void SetName(const std::string& newName);

    const std::unordered_map<std::string, Component*>& GetAllComponents() const;

    void SetType(std::string t);
    std::string GetType() const;

    nlohmann::json ToJson();
    void UpdateFromJson(const nlohmann::json& j);

private:
    uuids::uuid id;
    std::string name;
    std::string type;
    
    std::unordered_map<std::string, Component*> components;
};