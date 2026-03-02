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

    void UpdateFromJson(const nlohmann::json& j) override {
        ownedAssets.clear();
        if (j.contains("ownedAssets")) {
            for (const auto& idStr : j["ownedAssets"]) {
                auto id = uuids::uuid::from_string(idStr.get<std::string>());
                if (id) ownedAssets.push_back(*id);
            }
        }
    }

    nlohmann::json ToJson() const override {
        std::vector<std::string> ids;
        for (const auto& id : ownedAssets) ids.push_back(uuids::to_string(id));
        return nlohmann::json{{"ownedAssets", ids}};
    }
};