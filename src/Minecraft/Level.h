#pragma once
#include "../Client/VFunc.h"
#include "HitResult.h"

class Level {
public:
    HitResult& getHitResult() {
        return edu::callVFunc<333, HitResult&>(this);
    }
};
