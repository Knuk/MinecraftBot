#include "encryptionrequest.h"

EncryptionRequest::EncryptionRequest(QByteArray d, MainWindow * i_ui)
{
    //Some vars for this packet
    packetID = 1;
    packetSize = data.length();
    ui = i_ui;
    data = d;
    displayColor = QColor(255, 75, 75);

    //Display the packet
    ui->displayPacket(true, packetID, packetSize, displayColor, "Encryption request");

    //The first byte, which would be the server ID, is always empty so we cut it off
    data = data.right(data.length() - 1);

    //Next is the public key's length as a varint
    int nbBytesDecoded;
    uint8_t * buffer = (uint8_t*)data.data();
    publicKeyLength = Varint::decode_unsigned_varint(buffer, nbBytesDecoded);

    //Then we cut off that part
    data = data.right(data.length() - nbBytesDecoded);

    //Public key
    publicKey = data.left(publicKeyLength);

    //Then we cut off the public key's part
    data = data.right(data.length() - publicKeyLength);

    //Same thing with token
    buffer = (uint8_t*)data.data();
    verifyTokenLength = Varint::decode_unsigned_varint(buffer, nbBytesDecoded);
    data = data.right(data.length() - nbBytesDecoded);
    verifyToken = data.left(verifyTokenLength);
}

EncryptionRequest::~EncryptionRequest()
{

}

