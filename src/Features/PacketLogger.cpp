#include "PacketLogger.h"
#include "../Client/ClientInstance.h"
#include "../Client/ClientStore.h"
#include "../Minecraft/PacketSender.h"
#include "../Client/Chat.h"
#include <MinHook.h>

namespace edu::features {

bool g_packetLoggerEnabled = false;

void togglePacketLogger() {
    g_packetLoggerEnabled = !g_packetLoggerEnabled;
}

using SendFn = void(__fastcall*)(PacketSender*, Packet*);
static SendFn o_send = nullptr;
static void* g_sendTarget = nullptr;

static void __fastcall hk_send(PacketSender* self, Packet* packet) {
    if (g_packetLoggerEnabled && packet) {
        uint32_t id = static_cast<uint32_t>(packet->getId());
        edu::logChat("[Packet] Outgoing ID: " + std::to_string(id));
    }
    o_send(self, packet);
}

void installPacketSendHook() {
    auto* ci = getClientInstance();
    if (!ci) return;

    auto& sender = ci->getPacketSender();
    uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(&sender);
    g_sendTarget = reinterpret_cast<void*>(vtable[2]);

    MH_Initialize();
    MH_CreateHook(g_sendTarget, reinterpret_cast<void*>(&hk_send), reinterpret_cast<void**>(&o_send));
    MH_EnableHook(g_sendTarget);
}

void removePacketSendHook() {
    if (g_sendTarget) {
        MH_DisableHook(g_sendTarget);
        MH_RemoveHook(g_sendTarget);
        g_sendTarget = nullptr;
    }
    o_send = nullptr;
}

} // namespace edu::features
