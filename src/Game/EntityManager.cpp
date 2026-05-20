// EntityManager.cpp
#include "EntityManager.h"
#include "Core/ECS/Entity.h"
#include <string>
#include <unordered_map>
#include <vector>

std::vector<Entity *> EntityManager::empty{};

void EntityManager::add(Entity *entity) {
    byType[entity->GetType()].push_back(entity);
    byId[entity->GetId()] = entity;
  }

  void EntityManager::remove(const uuids::uuid &id) {
    auto it = byId.find(id);
    if (it != byId.end()) {
      Entity *entity = it->second;
      // byType'tan çıkar
      auto &list = byType[entity->GetType()];
      list.erase(std::remove(list.begin(), list.end(), entity), list.end());
      byId.erase(it);
      delete entity; // Sadece bir kez, burada
    }
  }

  std::vector<Entity *>& EntityManager::getByType(const std::string &type) {
    auto it = byType.find(type);
    return (it != byType.end()) ? it->second : empty;
  }

  Entity* EntityManager::getById(const uuids::uuid &id) {
    auto it = byId.find(id);
    return (it != byId.end()) ? it->second : nullptr;
  }

  void EntityManager::optimize() {
    for (auto &[type, list] : byType) {
      list.shrink_to_fit();
    }
  }

void EntityManager::clear() {
    for (auto& [type, list] : byType) {
        for (Entity* e : list) delete e;
    }
    byType.clear();
    byId.clear(); // delete YOK, sadece map temizleniyor
}