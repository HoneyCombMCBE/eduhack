#pragma once

namespace edu::features {

extern bool g_packetLoggerEnabled;
void togglePacketLogger();
void installPacketSendHook();
void removePacketSendHook();

} // namespace edu::features
