#include "Entity.h"
#include "Core/Components/InventoryComponent.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/ManpowerPoolComponent.h"
#include "Economy/Components/OwnerComponent.h"
#include "Economy/Components/PerkComponent.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/TechTreeComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/MarketMemberComponent.h"
#include "World/Components/LocationComponent.h"
#include "World/Components/DemographicsComponent.h"
#include "AI/Components/AIControllerComponent.h"


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

    void Entity::RemoveComponent(const std::string& key) {
        auto it = components.find(key);
        if (it != components.end()) {
            delete it->second;
            components.erase(it);
        }
        
    }

    uuids::uuid Entity::GetId() const { return id; }
    void Entity::SetId(const uuids::uuid& newId) { id = newId; }

    std::string Entity::GetName() const { return name; }
    void Entity::SetName(const std::string& newName) { name = newName; }

    const std::unordered_map<std::string, Component*>& Entity::GetAllComponents() const { return components; }

    void Entity::SetType(std::string t) { type = t; }
    std::string Entity::GetType() const { return type; }

    nlohmann::json Entity::ToJson() {
        nlohmann::json j = nlohmann::json{
            {"id", uuids::to_string(GetId())},
            {"name", GetName()},
            {"type", GetType()},
            {"components", nlohmann::json::object()}
        };

        for (const auto& [key, comp] : GetAllComponents()) {
            j["components"][key] = comp->ToJson(); 
        }
        return j;
    }

    Component* CreateComponentByType(const std::string& type) {
        if (type == "AssetOwnerComponent")     return new AssetOwnerComponent();
        if (type == "InventoryComponent")      return new InventoryComponent();
        if (type == "ManpowerPoolComponent") return new ManpowerPoolComponent();
        if (type == "OwnerComponent")        return new OwnerComponent();
        if (type == "PerkComponent")           return new PerkComponent();
        if (type == "ProductionComponent")     return new ProductionComponent("");
        if (type == "TechTreeComponent")     return new TechTreeComponent();
        if (type == "WalletComponent")       return new WalletComponent();
        if (type == "WorkforceComponent")    return new WorkforceComponent();
        if (type == "DemographicsComponent") return new DemographicsComponent(0); 
        if (type == "LocationComponent")     return new LocationComponent(0, 0);
        if (type == "MarketComponent")       return new MarketComponent();
        if (type == "MarketMemberComponent") return new MarketMemberComponent(uuids::uuid{});
        if (type == "AIControllerComponent") return new AIControllerComponent();
        return nullptr;
    }

    void Entity::UpdateFromJson(const nlohmann::json& j) {
        // 1. Temel verileri çek
        if (j.contains("id")) {
            auto id = uuids::uuid::from_string(j.at("id").get<std::string>());
            if (id) SetId(*id);
        }
        
        SetName(j.value("name", "Unknown Entity"));
        SetType(j.value("type", "generic"));

        // 2. Komponentleri işle
        if (j.contains("components") && j["components"].is_object()) {
            for (auto it = j["components"].begin(); it != j["components"].end(); ++it) {
                std::string compType = it.key();
                nlohmann::json compData = it.value();

                // Fabrikadan yeni komponent iste
                Component* newComp = CreateComponentByType(compType);
                
                if (newComp) {
                    newComp->UpdateFromJson(compData);
                    AddComponent(newComp, compType);
                }
            }
        }
    }