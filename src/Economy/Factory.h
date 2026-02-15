#pragma once

#include "../../lib/nlohmann/json.hpp"
#include "../../lib/uuid/uuid.h"
#include "../Core/Types.h"
#include "../Registry/FactoryManager.h"
#include "Core/Inventory.h"
#include <string>
#include <vector>

class Factory {
public:
    Factory(const std::string& defId, uuids::uuid ownerId);

    void produce(double globalModifiers = 1.0);

    void addInput(const std::string& itemId, float amount);
    Inventory& getInputStorage() { return inputStorage; }
    Inventory& getOutputStorage() { return outputStorage; }
    
    std::vector<ItemStack> collectOutputs();

    std::vector<ItemStack> getMissingInputs() const;

    size_t addEmployees(size_t count);
    void setEmployeeCount(size_t count);
    size_t getEmployeeCount() const;

    uuids::uuid getId() const { return id; }
    uuids::uuid getOwnerId() const { return ownerId; }
    std::string getDefinitionId() const { return definitionId; }
    
    std::string getName() const { return customName; }
    void setName(const std::string& name) { customName = name; }

    // JSON Serialization
    friend void to_json(nlohmann::json& j, const Factory& f);
    // from_json'u sonra yazarız load game için.

private:
    uuids::uuid id;
    uuids::uuid ownerId;
    
    std::string definitionId; 
    std::string customName;

    size_t employeeCount = 0;
    
    Inventory inputStorage;
    Inventory outputStorage;
};