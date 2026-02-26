// EntityFactory.h
#include "Core/ECS/Entity.h"
#include "Economy/Components/OwnerComponent.h"
#include "Economy/Components/ProductionComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "Core/Components/InventoryComponent.h"
#include "Game/IdUtils.h"
#include "Registry/FactoryManager.h"

// inline for now, move to .cpp file later.
inline Entity* createFactory(const FactoryData &fData, const uuids::uuid &ownerId, const std::string &customName = "") {
    Entity *factory = new Entity();
    factory->SetType("factory");

    factory->SetId(IdUtils::generateUuid());
    if (!customName.empty()) {
        factory->SetName(customName);
    } else {
        factory->SetName(fData.name);
    }

    factory->AddComponent(new ProductionComponent(fData.id), "ProductionComponent");
    for (const auto &pipeId : fData.pipeline_ids) {
        factory->GetComponent<ProductionComponent>("ProductionComponent")->addPipeline(pipeId);
    }

    factory->AddComponent(new WorkforceComponent);
    factory->GetComponent<WorkforceComponent>("WorkforceComponent")->maxWorkers = fData.max_workers;

    factory->AddComponent(new InventoryComponent, "inputStorage");
    factory->AddComponent(new InventoryComponent, "outputStorage");

    factory->AddComponent(new OwnerComponent);
    factory->GetComponent<OwnerComponent>("OwnerComponent")->setOwnerId(ownerId);

    return factory;
}