// EntityCompany.h

#include "Core/Components/InventoryComponent.h"
#include "Core/ECS/Entity.h"
#include "Economy/Components/AssetOwnerComponent.h"
#include "Economy/Components/PerkComponent.h"
#include "Economy/Components/TechTreeComponent.h"
#include "Economy/Components/WalletComponent.h"
#include "Economy/Components/ManpowerPoolComponent.h"
#include "Registry/CompanyManager.h"
#include "Registry/NameManager.h"
#include "Game/IdUtils.h"
#include "Registry/PerkManager.h"

inline Entity* createCompany(const CompanyTemplate &tmpl, const std::string &name ="") {
    Entity* company = new Entity();
    company->SetId(IdUtils::generateUuid());
    company->SetType("company");

    auto n = (name != "") ? name : NameManager::getRandomName(tmpl.name_pool_id);
    company->SetName(n);

    company->AddComponent(new WalletComponent, "WalletComponent");
    company->GetComponent<WalletComponent>("WalletComponent")->balance = tmpl.start_capital;
    company->AddComponent(new TechTreeComponent, "TechTreeComponent");
    for (const auto &tid : tmpl.start_techs) {
        company->GetComponent<TechTreeComponent>("TechTreeComponent")->unlock(tid);
    }
    company->AddComponent(new ManpowerPoolComponent(tmpl.start_manpower), "ManpowerPoolComponent");

    company->AddComponent(new InventoryComponent, "Storage");
    for (const auto &item : tmpl.start_inventory) {
        company->GetComponent<InventoryComponent>("Storage")->Add(item.id, item.quantity);
    }

    company->AddComponent(new PerkComponent, "PerkComponent");
    for (const auto &pid : tmpl.start_perks) {
        auto perk = PerkManager::perks.at(pid);
        company->GetComponent<PerkComponent>("PerkComponent")->addPerk(pid, perk.default_duration);
    }
    
    company->AddComponent(new AssetOwnerComponent);

    return company;
}