#pragma once
// ClientInstance: a C++ mirror of the game's ClientInstance object (Minecraft
// Bedrock). We model it as a real type - the object's first pointer-sized word
// is the vtable, and every virtual is reachable by vtable index.
//
// Only the vtable slots we currently use are named in VIndex. Add more as the
// client grows; we don't need the full ~424-entry layout.
//
// This type does NOT own its lifecycle utility (capture/get of the live
// instance). Those live in ClientStore.h as free functions, so this header
// stays a pure type definition.

#include <cstdint>

namespace edu {

// Named vtable indices. Offset = index * 8. Extend as virtuals are typed.
enum VIndex : int {
    V_update                 = 24,   // 0xC0  bool update(bool isInitFinished)
    V_getRegion              = 30,   // 0xF0
    V_getLocalPlayer         = 31,   // 0xF8
    V_getLocalUser           = 37,   // 0x128
    V_getLevel               = 169,  // 0x548
    V_hasLevel               = 171,  // 0x558
    V_isInMultiplayerGame    = 173,  // 0x568
    V_getOptions             = 175,  // 0x578
    V_getGameRenderer        = 186,  // 0x5D0
    V_getLevelRenderer       = 187,  // 0x5D8
    V_getGuiData             = 217,  // 0x6C8
    V_getPacketSender        = 293,  // 0x928
    V_getSoundEngine         = 316,  // 0x9E0
    V_getEventCoordinator    = 376,  // 0xBC0
    V_isPlaying              = 387,  // 0xC18
    V_getClientInstanceState = 388,  // 0xC20
};

// The object's vtable: a flat array of function pointers. We don't enumerate
// every entry; we address by index through vfunc().
struct ClientInstanceVTable {
    void* slots[432];   // covers the full known dump (max index 423)
};

// C++ mirror of ClientInstance. The game object's layout begins with the
// vtable pointer; fields after it are added here as they're reversed.
class ClientInstance {
public:
    // Object layout begins with the vtable pointer (offset 0x0).
    // Real instance fields go below as they're reversed; for now only the
    // vtable is modeled.
    ClientInstanceVTable* vftable;

    void* vfunc(VIndex idx) const {
        return vftable ? vftable->slots[idx] : nullptr;
    }

    // --- Typed virtual helpers ---------------------------------------------
    // These call through the REAL vtable, so they invoke whatever the game
    // installed - including, for update(), a MinHook-patched entry. Calling
    // these for an *unhooked* virtual (getLevel, getLocalPlayer, ...) is the
    // normal way to reach the game. For an *unhooked* virtual like update(),
    // calling it from inside the hook recurses - use the detour's trampoline
    // (o_update) there instead.

    using UpdateFn = bool (__fastcall*)(ClientInstance*, bool);

    bool update(bool isInitFinished) {
        auto fn = reinterpret_cast<UpdateFn>(vfunc(V_update));
        return fn ? fn(this, isInitFinished) : false;
    }

    using GetLocalPlayerFn = void* (__fastcall*)(ClientInstance*);

    void* getLocalPlayer() {
        auto fn = reinterpret_cast<GetLocalPlayerFn>(vfunc(V_getLocalPlayer));
        return fn ? fn(this) : nullptr;
    }

    using HasLevelFn = bool (__fastcall*)(ClientInstance*);

    bool hasLevel() {
        auto fn = reinterpret_cast<HasLevelFn>(vfunc(V_hasLevel));
        return fn && fn(this);
    }

    using GetLevelFn = void* (__fastcall*)(ClientInstance*);

    void* getLevel() {
        auto fn = reinterpret_cast<GetLevelFn>(vfunc(V_getLevel));
        return fn ? fn(this) : nullptr;
    }
};

} // namespace edu
