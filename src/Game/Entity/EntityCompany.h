// EntityCompany.h

#include "Core/Components/InventoryComponent.h"
#include "Core/ECS/Entity.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/PerkComponent.h"
#include "Economy/Components/TechTreeComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/WorkforceComponent.h"
#include "Registry/CompanyManager.h"
#include "Registry/NameManager.h"
#include "Game/IdUtils.h"
#include "Registry/PerkManager.h"

inline Entity* createCompany(const CompanyTemplate &tmpl, const std::string &name ="") {
    Entity* company = new Entity();
    company->SetId(IdUtils::generateUuid());
    company->SetType("company");

    auto n = (name != "") ? name : NameManager::getRandomName(tmpl.name_pool_id);
    company->SetName(name);

    company->AddComponent(new WalletComponent);
    company->GetComponent<WalletComponent>("WalletComponent")->balance = tmpl.start_capital;
    company->AddComponent(new TechTreeComponent);
    for (const auto &tid : tmpl.start_techs) {
        company->GetComponent<TechTreeComponent>("TechTreeComponent")->unlock(tid);
    }
    company->AddComponent(new WorkforceComponent);
    company->GetComponent<WorkforceComponent>("WorkforceComponent")->currentWorkers = tmpl.start_manpower;

    company->AddComponent(new InventoryComponent);
    for (const auto &item : tmpl.start_inventory) {
        company->GetComponent<InventoryComponent>("Storage")->Add(item.id, item.quantity);
    }

    company->AddComponent(new PerkComponent);
    for (const auto &pid : tmpl.start_perks) {
        auto perk = PerkManager::perks.at(pid);
        company->GetComponent<PerkComponent>("PerkComponent")->addPerk(pid, perk.default_duration);
    }
    
    company->AddComponent(new AssetOwnerComponent);

    return company;
}