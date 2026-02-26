// src/World/Components/LocationComponent.h
#include "Core/ECS/Component.h"
#include "uuid/uuid.h"

class LocationComponent : public Component {
public:
    // Eğer haritan Grid bazlıysa (x, y) integer
    // Eğer Dünya haritasıysa (latitude, longitude) double
    float x = 0.0f;
    float y = 0.0f;
    
    // Eğer bir hiyerarşi varsa (Fabrika -> Şehrin İçinde)
    uuids::uuid parentId; // Hangi Entity'nin içinde?

    std::string GetComponentType() const override { return "LocationComponent"; }
};