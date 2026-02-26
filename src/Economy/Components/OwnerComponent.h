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
};