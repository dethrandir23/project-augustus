#include "MarketDebug.h"
#include "World/Components/MarketComponent.h"
#include "World/Components/MarketMemberComponent.h"

namespace MarketDebug {

static void accumulate(OrderStats& stats, const std::vector<MarketOrder>& orders) {
    for (const auto& o : orders) {
        float rem = o.remaining();
        if (o.type == OrderType::BUY) {
            stats.buyCount++;
            stats.buyVolume += rem;
        } else {
            stats.sellCount++;
            stats.sellVolume += rem;
        }
    }
    stats.totalCount = stats.buyCount + stats.sellCount;
}

OrderStats getGlobalStats(Gamestate& gamestate) {
    OrderStats stats;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* mc = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!mc) continue;
        for (auto& [_, book] : mc->books) {
            accumulate(stats, book.buyOrders);
            accumulate(stats, book.sellOrders);
        }
    }
    return stats;
}

OrderStats getMarketStats(Gamestate& gamestate, const uuids::uuid& marketId) {
    OrderStats stats;
    Entity* market = gamestate.getEntity(marketId);
    if (!market) return stats;
    auto* mc = market->GetComponent<MarketComponent>("MarketComponent");
    if (!mc) return stats;
    for (auto& [_, book] : mc->books) {
        accumulate(stats, book.buyOrders);
        accumulate(stats, book.sellOrders);
    }
    return stats;
}

OrderStats getEntityStats(Gamestate& gamestate, const uuids::uuid& entityId) {
    OrderStats stats;
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        auto* mc = entity->GetComponent<MarketComponent>("MarketComponent");
        if (!mc) continue;
        for (auto& [_, book] : mc->books) {
            for (const auto& o : book.buyOrders) {
                if (o.ownerId == entityId) {
                    stats.buyCount++;
                    stats.buyVolume += o.remaining();
                }
            }
            for (const auto& o : book.sellOrders) {
                if (o.ownerId == entityId) {
                    stats.sellCount++;
                    stats.sellVolume += o.remaining();
                }
            }
        }
    }
    stats.totalCount = stats.buyCount + stats.sellCount;
    return stats;
}

OrderStats getEntityStatsInMarket(Gamestate& gamestate, const uuids::uuid& entityId, const uuids::uuid& marketId) {
    OrderStats stats;
    Entity* market = gamestate.getEntity(marketId);
    if (!market) return stats;
    auto* mc = market->GetComponent<MarketComponent>("MarketComponent");
    if (!mc) return stats;
    for (auto& [_, book] : mc->books) {
        for (const auto& o : book.buyOrders) {
            if (o.ownerId == entityId) {
                stats.buyCount++;
                stats.buyVolume += o.remaining();
            }
        }
        for (const auto& o : book.sellOrders) {
            if (o.ownerId == entityId) {
                stats.sellCount++;
                stats.sellVolume += o.remaining();
            }
        }
    }
    stats.totalCount = stats.buyCount + stats.sellCount;
    return stats;
}

nlohmann::json statsToJson(const OrderStats& s) {
    return {
        {"buyCount", s.buyCount},
        {"sellCount", s.sellCount},
        {"totalCount", s.totalCount},
        {"buyVolume", s.buyVolume},
        {"sellVolume", s.sellVolume}
    };
}

nlohmann::json getAllStatsAsJson(Gamestate& gamestate) {
    nlohmann::json j;
    j["global"] = statsToJson(getGlobalStats(gamestate));

    j["markets"] = nlohmann::json::object();
    for (auto* entity : gamestate.getEntitiesByType("market")) {
        j["markets"][uuids::to_string(entity->GetId())] = statsToJson(getMarketStats(gamestate, entity->GetId()));
    }

    j["entities"] = nlohmann::json::object();
    for (auto* entity : gamestate.getEntitiesByType("trade_node")) {
        j["entities"][uuids::to_string(entity->GetId())] = statsToJson(getEntityStats(gamestate, entity->GetId()));
    }
    for (auto* entity : gamestate.getEntitiesByType("company")) {
        j["entities"][uuids::to_string(entity->GetId())] = statsToJson(getEntityStats(gamestate, entity->GetId()));
    }

    return j;
}

}
