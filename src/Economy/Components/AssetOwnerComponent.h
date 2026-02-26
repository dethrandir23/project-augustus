#pragma once
#include "Core/ECS/Component.h"
#include "uuid/uuid.h"
#include <vector>
#include <algorithm>

class AssetOwnerComponent : public Component {
public:
    std::vector<uuids::uuid> ownedAssets; 

    std::string GetComponentType() const override { return "AssetOwnerComponent"; }

    void addAsset(const uuids::uuid& id) {
        ownedAssets.push_back(id);
    }

    void removeAsset(const uuids::uuid& id) {
        ownedAssets.erase(std::remove(ownedAssets.begin(), ownedAssets.end(), id), ownedAssets.end());
    }

    
};