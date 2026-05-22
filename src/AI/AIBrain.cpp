#include "AIBrain.h"

nlohmann::json AIBrain::toJson() const {
    return {{"type", getType()}};
}

void AIBrain::fromJson(const nlohmann::json& j) {
    (void)j;
}
