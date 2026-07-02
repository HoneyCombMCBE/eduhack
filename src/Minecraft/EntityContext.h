#pragma once
#include "Entity.h"
#include <string>
#include <memory>

struct EntityRegistry : std::enable_shared_from_this<EntityRegistry> {
    std::string                 name;
    entt::basic_registry<EntityId> registry;
    uint32_t                    id;
};

struct EntityContext {
    EntityRegistry&                registry;
    entt::basic_registry<EntityId>& enttRegistry;
    EntityId                       entity;

    template<std::derived_from<IEntityComponent> T>
    T* tryGetComponent() {
        return enttRegistry.try_get<T>(entity);
    }

    template<std::derived_from<IEntityComponent> T>
    const T* tryGetComponent() const {
        return enttRegistry.try_get<T>(entity);
    }

    template<std::derived_from<IEntityComponent> T>
    bool hasComponent() const {
        return enttRegistry.all_of<T>(entity);
    }
};
