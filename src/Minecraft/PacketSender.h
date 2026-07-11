#pragma once
#include "Packet.h"

namespace edu {

class PacketSender {
public:
    virtual ~PacketSender() = default;
    virtual bool isInitialized() = 0;       // Index 1
    virtual void send(Packet* packet) = 0;   // Index 2
    virtual void sendTo() = 0;               // Index 3
    virtual void sendToServer(Packet* packet) = 0; // Index 4
};

} // namespace edu
