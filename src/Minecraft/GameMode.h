#pragma once
#include "../Client/VFunc.h"

class Actor;

class GameMode {
public:
    bool attack(Actor& entity) {
        return edu::callVFunc<15, bool(Actor&)>(this, entity);
    }
};
