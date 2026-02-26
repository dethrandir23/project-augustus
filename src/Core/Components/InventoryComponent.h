// InventoryComponent.h
#pragma once
#include "Core/ECS/Component.h"
#include "Core/Inventory.h"

class InventoryComponent : public Component {
public:
    InventoryComponent() = default;
    InventoryComponent(double maxCapacity) : inventory(maxCapacity) {}

    std::string GetComponentType() const override {
        return "InventoryComponent";
    }
    
    void Add(const std::string& itemId, double amount) {
        inventory.add(itemId, amount);
    }

    bool Remove(const std::string& itemId, double amount) {
        return inventory.remove(itemId, amount);
    }
    
    bool Has(const std::string& itemId, double amount) const {
        return inventory.has(itemId, amount);
    }

    void Clear() {
        inventory.clear();
    }

    std::vector<ItemStack> GetAllItems() const {
        return inventory.getAllItems();
    }

    Inventory& GetInternalInventory() { return inventory; }

    friend void to_json(nlohmann::json& j, const InventoryComponent& c) {
        j = c.inventory; 
        j["type"] = c.GetComponentType();
    }

    friend void from_json(const nlohmann::json& j, InventoryComponent& c) {
        c.inventory = j.get<Inventory>();
    }

private:
    Inventory inventory; 
};