// src/World/Components/LocationComponent.h
#include "Core/ECS/Component.h"
#include "uuid/uuid.h"
#include "Core/Types.h"

// Note: In the future, map and location features can be generalized as N-dimensional. 
// This way, there will be no need to deal with adding dimensions manually.
class LocationComponent : public Component {
public:
    LocationComponent(double x, double y) {
        location = Location(x, y);
    }

    std::string GetComponentType() const override { return "LocationComponent"; }

    double GetX() const { return location.x; }
    double GetY() const { return location.y; }

    void SetX(double x) { location.x = x; }
    void SetY(double y) { location.y = y; }

    void SetPosition(double x, double y) {
        location.x = x;
        location.y = y;
    }

    void ResetPosition() {
        location.x = 0;
        location.y = 0;
    }

    void Move(double dx, double dy) {
        location.x += dx;
        location.y += dy;
    }

    nlohmann::json ToJson() const override {
        return {{"x", GetX()}, {"y", GetY()}};
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        SetPosition(j.value("x", 0.0), j.value("y", 0.0));
    }

private:
    Location location;
};