// src/Systems/Components/MarketComponent.h
#pragma once

#include "Core/ECS/Component.h"
#include "Economy/Orderbook.h"
#include <unordered_map>
#include <string>

class MarketComponent : public Component {
public:
    // Sadece OrderBook'lar. Pressure yok, Price Cache yok, Node Listesi yok.
    std::unordered_map<std::string, OrderBook> books;
    
    float tariffRate = 0.10f; // %10 gümrük vergisi (yabancı marketten alımlarda)

    std::string GetComponentType() const override { return "MarketComponent"; }

    // OrderBook helper'ları buraya gelir (addOrder vb.)
    OrderBook* getBook(const std::string& itemId) {
        if (books.find(itemId) == books.end()) {
             books[itemId].itemId = itemId;
        }
        return &books[itemId];
    }
    
    // Fiyatı öğrenmek isteyen OrderBook'un 'lastTradedPrice'ına bakar.
    double getPrice(const std::string& itemId) const {
        if (books.find(itemId) == books.end()) return 0.0;
        return books.at(itemId).lastTradedPrice;
    }

    nlohmann::json ToJson() const override {
        nlohmann::json j_books = nlohmann::json::object();
        for (const auto& [itemId, book] : books) {
            double buyVolume = 0, sellVolume = 0;
            for (const auto& o : book.buyOrders) buyVolume += o.remaining();
            for (const auto& o : book.sellOrders) sellVolume += o.remaining();
            j_books[itemId] = {
                {"itemId", book.itemId},
                {"lastTradedPrice", book.lastTradedPrice},
                {"bestBid", book.getBestBid()},
                {"bestAsk", book.getBestAsk()},
                {"buyVolume", buyVolume},
                {"sellVolume", sellVolume},
                {"buyOrders", book.buyOrders},
                {"sellOrders", book.sellOrders}
            };
        }
        return {
            {"tariffRate", tariffRate},
            {"books", j_books}
        };
    }

    void UpdateFromJson(const nlohmann::json& j) override {
        tariffRate = j.value("tariffRate", 0.10f);
        if (j.contains("books") && j["books"].is_object()) {
            for (auto it = j["books"].begin(); it != j["books"].end(); ++it) {
                auto& book = books[it.key()];
                book.itemId = it.value().value("itemId", it.key());
                book.lastTradedPrice = it.value().value("lastTradedPrice", 0.0);
                if (it.value().contains("buyOrders"))
                    book.buyOrders = it.value()["buyOrders"].get<std::vector<MarketOrder>>();
                if (it.value().contains("sellOrders"))
                    book.sellOrders = it.value()["sellOrders"].get<std::vector<MarketOrder>>();
            }
        }
    }

};