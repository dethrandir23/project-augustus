#include "Entity.h"


Entity::~Entity() {
        for (auto& [key, component] : components) {
            delete component;
        }
        components.clear();
    }

void Entity::AddComponent(Component* component, const std::string& customKey) {
        if (!component) return;

        std::string key = customKey;
        if (key.empty()) {
            key = component->GetComponentType();
        }

        if (components.find(key) != components.end()) {
            delete components[key];
        }

        components[key] = component;
    }

    template <typename T>
    T* Entity::GetComponent(const std::string& key) {
        
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

    uuids::uuid Entity::GetId() const { return id; }
    void Entity::SetId(const uuids::uuid& newId) { id = newId; }

    std::string Entity::GetName() const { return name; }
    void Entity::SetName(const std::string& newName) { name = newName; }

    const std::unordered_map<std::string, Component*>& Entity::GetAllComponents() const { return components; }

    void Entity::SetType(std::string t) { type = t; }
    std::string Entity::GetType() const { return type; }

    void to_json(nlohmann::json& j, const Entity& e) {
        j = nlohmann::json{
            {"id", uuids::to_string(e.GetId())},
            {"name", e.GetName()},
            {"type", e.GetType()},
            {"components", nlohmann::json::object()}
        };

        for (const auto& [key, comp] : e.GetAllComponents()) {
            j["components"][key] = comp->ToJson(); 
        }
    }

    void from_json(const nlohmann::json& j, Entity& e) {}