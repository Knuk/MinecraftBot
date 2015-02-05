#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include "packet.h"

class Handshake : public Packet
{
public:
    Handshake(const uint8_t protocolVersion, const string &serverAdress, const uint16_t serverPort, const uint8_t nextState);
    ~Handshake();
    QByteArray packPacket();
private:
    uint8_t protocolVersion;
    string serverAdress;
    uint16_t serverPort;
    uint8_t nextState;
};

#endif // HANDSHAKE_H