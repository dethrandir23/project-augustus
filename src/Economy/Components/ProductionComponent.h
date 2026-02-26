// src/Economy/Components/ProductionComponent.h
#pragma once
#include "Core/ECS/Component.h"
#include <vector>
#include <string>
#include <algorithm>

class ProductionComponent : public Component {
public:
    std::string templateId;
    std::vector<std::string> activePipelineIds;

    float efficiency = 1.0f;

    bool isPaused = false;

    ProductionComponent(std::string tId = "") : templateId(tId) {}
    ProductionComponent() = default;

    std::string GetComponentType() const override { return "ProductionComponent"; }

    // --- Helpers ---
    void addPipeline(const std::string& id) {
        if (std::find(activePipelineIds.begin(), activePipelineIds.end(), id) == activePipelineIds.end()) {
            activePipelineIds.push_back(id);
        }
    }

    void removePipeline(const std::string& id) {
        activePipelineIds.erase(std::remove(activePipelineIds.begin(), activePipelineIds.end(), id), activePipelineIds.end());
    }

    // --- Serialization ---
    friend void to_json(nlohmann::json& j, const ProductionComponent& p) {
        j = nlohmann::json{
            {"pipelines", p.activePipelineIds},
            {"efficiency", p.efficiency},
            {"paused", p.isPaused}
        };
    }

    friend void from_json(const nlohmann::json& j, ProductionComponent& p) {
        p.activePipelineIds = j.at("pipelines").get<std::vector<std::string>>();
        p.efficiency = j.value("efficiency", 1.0f);
        p.isPaused = j.value("paused", false);
    }
};