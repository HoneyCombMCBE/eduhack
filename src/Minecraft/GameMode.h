#pragma once
#include "../Client/VFunc.h"
#include "Components.h"
#include <cstdint>

class Actor;

struct BlockPos {
    int x, y, z;
    BlockPos() : x(0), y(0), z(0) {}
    BlockPos(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
};

// GameMode vtable layout (Bedrock 1.21.x)
// 0:  destructor (private)
// 1:  startDestroyBlock
// 2:  destroyBlock
// 3:  continueDestroyBlock
// 4:  stopDestroyBlock
// 5:  startBuildBlock
// 6:  buildBlock
// 7:  continueBuildBlock
// 8:  stopBuildBlock
// 9:  tick
// 10: getPickRange
// 11: useItem
// 12: useItemAsAttack
// 13: useItemOn
// 14: interact
// 15: attack

class GameMode {
public:
    bool attack(Actor* entity) {
        return edu::callVFunc<15, bool(Actor*)>(this, entity);
    }

    bool startDestroyBlock(const BlockPos& pos, uint8_t face, bool& out) {
        return edu::callVFunc<1, bool(const BlockPos&, uint8_t, bool&)>(this, pos, face, out);
    }

    bool destroyBlock(const BlockPos& pos, uint8_t face) {
        return edu::callVFunc<2, bool(const BlockPos&, uint8_t)>(this, pos, face);
    }

    void continueDestroyBlock(const BlockPos& pos, uint8_t face, bool& out) {
        edu::callVFunc<3, void(const BlockPos&, uint8_t, bool&)>(this, pos, face, out);
    }

    void stopDestroyBlock(const BlockPos& pos) {
        edu::callVFunc<4, void(const BlockPos&)>(this, pos);
    }

    bool buildBlock(const BlockPos& pos, uint8_t face, bool isSimTick = true) {
        return edu::callVFunc<6, bool(const BlockPos&, uint8_t, bool)>(this, pos, face, isSimTick);
    }

    void continueBuildBlock(const BlockPos& pos, uint8_t face) {
        edu::callVFunc<7, void(const BlockPos&, uint8_t)>(this, pos, face);
    }

    void tick() {
        edu::callVFunc<9, void()>(this);
    }
};
