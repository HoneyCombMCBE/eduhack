#pragma once
#include "EntityContext.h"
#include "../Client/VFunc.h"
#include <libhat/access.hpp>
#include <string>
#include <optional>

class GameMode;

class Actor {
public:
    EntityContext& getEntity() {
        return hat::member_at<EntityContext>(this, 0x8);
    }

    const EntityContext& getEntity() const {
        return hat::member_at<EntityContext>(this, 0x8);
    }

    bool swing(int source = 0) {
        return edu::callVFunc<111, bool(int)>(this, source);
    }

    std::unique_ptr<GameMode>& getGameMode() {
        return hat::member_at<std::unique_ptr<GameMode>>(this, 0xA78);
    }

    void setAbilities(const struct LayeredAbilities& abilities) {
        edu::callVFunc<241, void(const LayeredAbilities&)>(this, abilities);
    }

    float causeFallDamageToActor(float distance, float multiplier) {
        return edu::callVFunc<92, float(float, float)>(this, distance, multiplier);
    }

    void displayClientMessage(const std::string& message, std::optional<std::string> filtered = std::nullopt) {
        edu::callVFunc<199, void(const std::string&, std::optional<std::string>)>(this, message, filtered);
    }

    Vec3 getPosition() const {
        auto& ctx = const_cast<Actor*>(this)->getEntity();
        auto* sv = ctx.tryGetComponent<StateVectorComponent>();
        return sv ? sv->pos : Vec3{0, 0, 0};
    }

    void setPosition(const Vec3& position) {
        auto& ctx = getEntity();
        auto* sv = ctx.tryGetComponent<StateVectorComponent>();
        if (sv) {
            sv->pos = position;
            sv->posPrev = position;
            sv->posDelta = {0, 0, 0};
        }
    }
};
