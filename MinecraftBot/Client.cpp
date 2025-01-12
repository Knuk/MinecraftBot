//CPP file for the Client class, which is the class handling most of the network stuff

#include "Client.h"
#include "mainwindow.h"
#include "handshake.h"
#include "loginstart.h"
#include "keepalive.h"
#include "encryptionrequest.h"
#include "encryptionresponse.h"
#include "chatmessage.h"
#include "sendchatmessage.h"
#include "playerpositionandlook.h"
#include "playerposition.h"
#include "clientstatus.h"
#include "onground.h"
#include "mapchunkbulk.h"
#include "mapchunk.h"
#include "multiblockchange.h"
#include "blockchange.h"

#define PROTOCOLVERSION 47 //Version of the minecraft protocol (1.8)

Client::Client(MainWindow * i_ui, const string &i_username, const string &i_password, const QString &i_ip, const int i_port)
{
    username = i_username;
    password = i_password;
    ip = i_ip;
    port = i_port;
    ui = i_ui;
    crypt = new CryptManager();
    currentState = HANDSHAKING;
    player = new Player(this, &socket);
    commandManager = new CommandManager(this, ui);
    packetsSinceLastKA = 0;
    world = new World();
}

Client::~Client()
{
}

void Client::startConnect()
{
    compressionSet = false;
    socket.compressionSet = false;
    encrypted = false;

    socket.ui = ui; //Give the socket the interface to write to
    socket.client = this;
    socket.doConnect(ip, port);

    if(socket.connectedBool)
    {
        //Send the two first packets necessary to connect
        Handshake hs = Handshake(&socket, ui, PROTOCOLVERSION, ip.toStdString(), port, 2);
        hs.sendPacket(compressionSet);
        LoginStart ls = LoginStart(&socket, ui, username);
        ls.sendPacket(compressionSet);

        //Switch to login phase
        currentState = LOGIN;
    }
}

void Client::decodePacket(QByteArray data)
{
    handlePacket(Packet(ui, data, compressionSet));
}

void Client::handlePacket(Packet &packet) //The big switch case of doom, to handle every packet
{
    if(packet.data.length() > 0)
    {
        //ui->writeToConsole(QString::number(packet.packetID));
        switch(currentState) //Packet ID means different things depending on game state
        {
        case HANDSHAKING: //Always empty for the client as of Minecraft version 1.8.1, the client sends packets then switches to login
            break;
        case LOGIN:
            switch(packet.packetID)
            {
            case 0: //Disconnect
                //ui->writeToConsole("The server is kicking us out!");
                ui->writeToConsole(packet.data);
                break;
            case 1: //Encryption request
                enableEncryption(&packet);
                break;
            case 2: //Login Success
                ui->writeToConsole("Login successful");
                currentState = PLAY;
                break;
            case 3: //Set compression
                ui->writeToConsole("Server enabled compression");
                compressionSet = true;
                socket.compressionSet = true;
                break;
            default:
                ui->writeToConsole("Unknown packet during LOGIN phase, ID " + QString::number(packet.packetID));
                break;
            }
            break;
        case PLAY:
            packetsSinceLastKA++;
            switch(packet.packetID)
            {
            case 0: //Keep alive
                {
                    KeepAlive ka = KeepAlive(&socket, ui, packet.data);
                    //ui->writeToConsole(QString::number(packet.packetSize));
                    ka.sendPacket(compressionSet);
                    packetsSinceLastKA = 0;
                }
                break;
            case 1: //Join game
                {
                    ClientStatus cs = ClientStatus(&socket, ui, 0);
                    cs.sendPacket(compressionSet);
                    //player->updateGround(&socket);
                }
                break;
            case 2: //Chat message
                {
                    ChatMessage cm = ChatMessage(&socket, ui, packet.data, commandManager);
                }
                break;
            case 3: //Time update
                break;
            case 8: //Player position and look
                {
                    PlayerPositionAndLook ppal = PlayerPositionAndLook(&socket, ui, packet.data);
                    player->setPositionAndLook(ppal.x, ppal.y, ppal.z, ppal.yaw, ppal.pitch, ppal.flags);
                    if(commandManager->waitingForCoords)
                    {
                        commandManager->setHome(ppal.x, ppal.y, ppal.z); //For the !sethome command
                    }
                    //player->updateGround(&socket);
                }
                break;
            case 9: //Held item change
                break;
            case 10: //Spawn position
                break;
            case 11: //Player abilities
                break;
            case 33: //Map Chunk
                {
                    MapChunk mc = MapChunk(&socket, packet.data, world);
                }
                break;
            case 34: //Multi block change
                {
                    MultiBlockChange mbc = MultiBlockChange(&socket, packet.data, world);
                }
                break;
            case 35: //Block change
                {
                    BlockChange bc = BlockChange(&socket, packet.data, world);
                }
                break;
            case 38: //Map Chunk Bulk
                {
                    MapChunkBulk mcb = MapChunkBulk(&socket, packet.data, world);
                }
                break;
            case 43: //Change game state
                break;
            case 47: //Set slot
                break;
            case 48: //Window items
                break;
            case 55: //Statistics
                break;
            case 59: //Scoreboard objective
                break;
            case 60: //update score
                break;
            case 61: //Scoreboard display
                break;
            case 62: //Teams
                break;
            case 63: //plugin message
                break;
            case 65: //Server difficulty
                break;
            case 68: //World border
                break;
            case 70: //Compression
                ui->writeToConsole("Server enabled compression");
                compressionSet = true;
                socket.compressionSet = true;
                break;
            default:
                break;
            }
            if(packetsSinceLastKA > 10000) //arbitrary value, timeout protection
            {
                packetsSinceLastKA = 0;
                KeepAlive ka = KeepAlive(&socket, ui, QByteArray::number(0));
                ka.sendPacket(compressionSet);
            }
            break;
        }
    }
    else
    {
        ui->writeToConsole("Packet rejected (size 0)");
    }

}
void Client::enableEncryption(Packet * packet)
{
    EncryptionRequest er(packet->data, ui); //Decypher the data into an encryption request packet
    crypt->loadKey(er.publicKey); //Load the RSA public key into the cryptography manager
    QByteArray hash = crypt->getHash(er.publicKey, er.serverID); //Get the hash necessary for the encryption response
    ui->writeToConsole(hash);

    EncryptionResponse er2 = EncryptionResponse(
                &socket,
                ui,
                crypt->encodeRSA(crypt->sharedSecret.data()),
                crypt->encodeRSA(er.verifyToken));
    //Auth
    Authentificator auth = Authentificator();
    auth.ui = this->ui;
    auth.authentificate(username, password, hash);
    //Session

    //Finish
    er2.sendPacket(compressionSet);
    encrypted = true;
}

void Client::sendMessage(QString message)
{
    //Cut the message in 100 bytes parts
    while(message.length() > 0)
    {
        SendChatMessage scm(&socket, ui, message.left(100));
        scm.sendPacket(compressionSet);
        message.remove(0, 100);
    }
}

void Client::movePlayer(Direction d, double distance)
{
    player->move(d, distance);
}
