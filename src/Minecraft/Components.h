#pragma once
#include "Entity.h"
#include <libhat/fixed_string.hpp>

struct Vec3 {
    float x, y, z;
};

struct StateVectorComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct StateVectorComponent";
    Vec3 pos;
    Vec3 posPrev;
    Vec3 velocity;
};

struct OnGroundFlagComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct OnGroundFlagComponent";
};

struct ActorIdentifierComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct ActorIdentifierComponent";
    uintptr_t data[8];
};

struct HealthComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct HealthComponent";
    int health;
    int maxHealth;
};

struct RuntimeIDComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct RuntimeIDComponent";
    int64_t runtimeId;
};

// MSVC uses "struct Foo" / "class Foo" in type names while GCC uses bare "Foo".
// Specialize entt::type_hash for all IEntityComponent types to compute the hash
// from the MSVC-formatted type_name member, ensuring compatibility with the
// game's MSVC-compiled registry.
template<std::derived_from<IEntityComponent> Type>
struct entt::type_hash<Type> {
    static constexpr entt::id_type value() noexcept {
        constexpr auto name = Type::type_name;
        return entt::hashed_string::value(name.data(), name.size());
    }
    constexpr operator entt::id_type() const noexcept { return value(); }
};
