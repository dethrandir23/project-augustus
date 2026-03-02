// OwnerComponent.h
#pragma once

#include "Core/ECS/Component.h"
#include "uuid/uuid.h"

class OwnerComponent : public Component {
public:
    uuids::uuid ownerId;

    void setOwnerId(const uuids::uuid& id) { ownerId = id; }
    uuids::uuid getOwnerId() const { return ownerId; }

    std::string GetComponentType() const override { return "OwnerComponent"; }

    void UpdateFromJson(const nlohmann::json& j) override {
        if (j.contains("ownerId")) {
            auto id = uuids::uuid::from_string(j["ownerId"].get<std::string>());
            if (id) ownerId = *id;
        }
    }

    nlohmann::json ToJson() const override {
        return nlohmann::json{{"ownerId", uuids::to_string(ownerId)}};
    }
};