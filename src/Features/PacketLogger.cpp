#include "PacketLogger.h"
#include "NoFall.h"
#include "Criticals.h"
#include "Derp.h"

#include "AntiHunger.h"
#include "Blink.h"
#include "Crasher.h"
#include "Freecam.h"
#include "Fly.h"
#include "../Client/ClientInstance.h"
#include "../Client/ClientStore.h"
#include "../Minecraft/PacketSender.h"
#include "../Client/Chat.h"
#include <MinHook.h>

namespace edu::features {

bool g_packetLoggerEnabled = false;

using SendFn = void(__fastcall*)(PacketSender*, Packet*);
static SendFn o_send = nullptr;
static void* g_sendTarget = nullptr;

static void __fastcall hk_send(PacketSender* self, Packet* packet) {
    if (packet) {
        // Dispatch to packet-based modules
        processNoFall(packet);
        processCriticals(packet);
        processDerp(packet);

        processAntiHunger(packet);

        bool shouldSend = true;
        processBlink(packet, shouldSend);
        processCrasher(packet, shouldSend);
        processFreecam(packet, shouldSend);
        processFlyPacket(packet);

        // Log if enabled
        if (g_packetLoggerEnabled) {
            uint32_t id = static_cast<uint32_t>(packet->getId());
            edu::logChat("[Packet] ID: " + std::to_string(id));
        }
        if (!shouldSend) return;
    }
    o_send(self, packet);
}

void installPacketSendHook() {
    if (g_sendTarget) return; // Already installed

    auto* ci = getClientInstance();
    if (!ci) return;

    auto& sender = ci->getPacketSender();
    uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(&sender);
    if (!vtable) return;

    g_sendTarget = reinterpret_cast<void*>(vtable[2]);
    if (!g_sendTarget) return;

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

void togglePacketLogger() {
    g_packetLoggerEnabled = !g_packetLoggerEnabled;
    if (g_packetLoggerEnabled) {
        installPacketSendHook();
    }
}

} // namespace edu::features
