// EntityManager.h

#include "Core/ECS/Entity.h"
#include <string>
#include <unordered_map>
#include <vector>

class EntityManager {
  std::unordered_map<std::string, std::vector<Entity *>> byType;
  std::unordered_map<uuids::uuid, Entity *> byId;

  static std::vector<Entity *> empty;

public:
  void add(Entity *entity);

  void remove(const uuids::uuid &id);

  std::vector<Entity *> &getByType(const std::string &type);

  Entity *getById(const uuids::uuid &id);

  void optimize();

  void clear();
};