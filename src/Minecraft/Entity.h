#pragma once
#include <cstdint>
#include <entt/entt.hpp>

struct EntityId {
    uint32_t rawId;

    constexpr bool operator==(const EntityId&) const = default;
    constexpr operator uint32_t() const { return rawId; }
};

struct EntityIdTraits {
    using value_type     = EntityId;
    using entity_type    = uint32_t;
    using version_type   = uint16_t;
    static constexpr entity_type  entity_mask  = 0x3FFFF;
    static constexpr entity_type  version_mask = 0x3FFF;
};

template<>
struct entt::entt_traits<EntityId> : entt::basic_entt_traits<EntityIdTraits> {
    static constexpr std::size_t page_size = 2048;
};

struct IEntityComponent {};

template<std::derived_from<IEntityComponent> Type>
struct entt::component_traits<Type> {
    using type = Type;
    static constexpr bool in_place_delete = true;
    static constexpr std::size_t page_size = 128 * !std::is_empty_v<Type>;
};

template<typename Type>
struct entt::storage_type<Type, EntityId> {
    using type = entt::basic_storage<Type, EntityId>;
};
