#pragma once
#include "Core/ECS/Component.h"
#include "uuid/uuid.h"

class MarketMemberComponent : public Component {
public:
    uuids::uuid marketId;

    MarketMemberComponent(uuids::uuid mId) : marketId(mId) {}

    std::string GetComponentType() const override { return "MarketMemberComponent"; }

    nlohmann::json ToJson() const override {
        return {{"marketId", uuids::to_string(marketId)}};
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        if (j.contains("marketId")) {
            auto id = uuids::uuid::from_string(j["marketId"].get<std::string>());
            if (id) marketId = *id;
        }
    }
};