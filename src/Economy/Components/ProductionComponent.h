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

    void addPipeline(const std::string& id) {
        if (std::find(activePipelineIds.begin(), activePipelineIds.end(), id) == activePipelineIds.end()) {
            activePipelineIds.push_back(id);
        }
    }

    void removePipeline(const std::string& id) {
        activePipelineIds.erase(std::remove(activePipelineIds.begin(), activePipelineIds.end(), id), activePipelineIds.end());
    }

    nlohmann::json ToJson() const override {
        return nlohmann::json{
            {"templateId", templateId},
            {"pipelines", activePipelineIds},
            {"efficiency", efficiency},
            {"paused", isPaused}
        };
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        templateId = j.value("templateId", "");
        activePipelineIds = j.at("pipelines").get<std::vector<std::string>>();
        efficiency = j.value("efficiency", 1.0f);
        isPaused = j.value("paused", false);
    }
};