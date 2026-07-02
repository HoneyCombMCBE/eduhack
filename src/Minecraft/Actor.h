#pragma once
#include "EntityContext.h"
#include <libhat/access.hpp>

class Actor {
public:
    EntityContext& getEntity() {
        return hat::member_at<EntityContext>(this, 0x8);
    }

    const EntityContext& getEntity() const {
        return hat::member_at<EntityContext>(this, 0x8);
    }
};
