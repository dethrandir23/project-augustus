#pragma once
#include "Core/ECS/Component.h"

class WalletComponent : public Component {
public:
    double balance = 0.0;
    double debt = 0.0;

    WalletComponent(double startBalance = 0.0) : balance(startBalance) {}

    std::string GetComponentType() const override { return "WalletComponent"; }

    bool trySpend(double amount) {
        if (balance >= amount) {
            balance -= amount;
            return true;
        }
        return false;
    }
    
    void addMoney(double amount) { balance += amount; }

    nlohmann::json ToJson() const override {
        return {{"balance", balance}, {"debt", debt}};
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        balance = j.value("balance", 0.0);
        debt = j.value("debt", 0.0);
    }
};