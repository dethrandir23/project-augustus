#pragma once
#include "Core/ECS/Component.h"
#include "uuid/uuid.h"

class MarketMemberComponent : public Component {
public:
    uuids::uuid marketId;

    MarketMemberComponent(uuids::uuid mId) : marketId(mId) {}

    std::string GetComponentType() const override { return "MarketMemberComponent"; }
};