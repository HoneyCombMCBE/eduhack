#pragma once
#include <cstdint>
#include <string>

namespace edu {

enum class PacketID : uint32_t {
    Unknown = 0,
    KeepAlive = 1,
    Text = 9,
    MovePlayer = 0x13,
    ClientCacheStatus = 129,
    PlayerAuthInput = 144,
};

class Packet {
public:
    virtual ~Packet() = default;
    virtual PacketID getId() = 0;
    virtual std::string getName() = 0;
};

} // namespace edu
