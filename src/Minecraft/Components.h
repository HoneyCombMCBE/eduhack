#pragma once
#include "Entity.h"
#include <libhat/fixed_string.hpp>

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;
};

struct StateVectorComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct StateVectorComponent";
    Vec3 pos;
    Vec3 posPrev;
    Vec3 posDelta;
};

struct OnGroundFlagComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct OnGroundFlagComponent";
};

struct MoveInputState {
    uint32_t mFlagValues;
    char _pad[12];
};

struct MoveInputComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct MoveInputComponent";
    MoveInputState mInputState;       // 0x0
    MoveInputState mRawInputState;    // 0x10
    uint8_t  mHoldAutoJumpInWaterTicks; // 0x20
    char     _pad1[3];
    Vec2     mMove;                   // 0x24
    Vec2     mLookDelta;              // 0x2c
    Vec2     mInteractDir;            // 0x34
    Vec3     mDisplacement;           // 0x3c
    Vec3     mDisplacementDelta;      // 0x48
    Vec3     mCameraOrientation;      // 0x54
    uint16_t mFlagValues;             // 0x60
};

class Actor;

class ActorOwnerComponent : public IEntityComponent {
public:
    static constexpr hat::fixed_string type_name = "class ActorOwnerComponent";
    std::unique_ptr<Actor> mActor;
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

enum class MovementAbilities : int {
    Flying                  = 0,
    MayFly                  = 1,
    Instabuild              = 2,
    IgnoresBorderCollisions = 3,
    NoClip                  = 4,
    WorldBuilder            = 5,
    FlySpeed                = 6,
    VerticalFlySpeed        = 7,
    Count                   = 8,
};

struct MovementAbilitiesComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct MovementAbilitiesComponent";
    bool flying;
    bool mayFly;
    bool instabuild;
    bool ignoresBorderCollisions;
    bool noClip;
    bool worldBuilder;
    float flySpeed;
    float verticalFlySpeed;
};

// LayeredAbilities: PermissionsHandler(24) + std::array<Abilities,6>(1440) = 1464 bytes
// We treat it as an opaque blob — only setAbilities needs the reference.
struct LayeredAbilities {
    char data[1464];
};

// The ECS component that holds the player's LayeredAbilities.
struct AbilitiesComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct AbilitiesComponent";
    LayeredAbilities abilities;
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

struct EntityRegistry : std::enable_shared_from_this<EntityRegistry> {
    std::string name;
    entt::basic_registry<EntityId> registry;
    uint32_t id;
};

struct EntityContext {
    EntityRegistry& registry;
    entt::basic_registry<EntityId>& enttRegistry;
    EntityId entity;
    template<std::derived_from<IEntityComponent> T>
    [[nodiscard]] T* tryGetComponent() {
        return this->enttRegistry.try_get<T>(this->entity);
    }

    template<std::derived_from<IEntityComponent> T>
    [[nodiscard]] const T* tryGetComponent() const {
        return this->enttRegistry.try_get<T>(this->entity);
    }

    template<std::derived_from<IEntityComponent> T>
    [[nodiscard]] bool hasComponent() const {
        return this->enttRegistry.all_of<T>(this->entity);
    }

    template<std::derived_from<IEntityComponent> T>
    T& getOrAddComponent() {
        return this->enttRegistry.get_or_emplace<T>(this->entity);
    }

    template<std::derived_from<IEntityComponent> T>
    void removeComponent() {
        this->enttRegistry.remove<T>(this->entity);
    }
};

struct AABB {
    Vec3 min;
    Vec3 max;
};

struct AABBShapeComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct AABBShapeComponent";
    AABB mAABB;
    Vec2 mBBDim;
};