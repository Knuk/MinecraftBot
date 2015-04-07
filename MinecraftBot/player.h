//This class contains informations for the player such as its location, inventory, etc

#ifndef PLAYER_H
#define PLAYER_H

#include "mytcpsocket.h"
#include "direction.h"
#include "movementthread.h"
#include "graph.h"
#include "position.h"
#include <math.h>
#include <QTime>

class Client;
class Player : public QObject
{
    Q_OBJECT
    friend class MovementThread; //Because I'd put them in the same class
public:
    Player(Client * client, MyTcpSocket * socket);
    ~Player();
    //Set position
    void setPositionAndLook(double x, double y, double z, float yaw, float pitch, char flags);
    void move(Direction d, double distance);
    void movePath(std::vector<Direction> d);
    void updateGround(MyTcpSocket * socket);
    bool canWalk(Direction d);
    void sendMessage(QString message);
    void goTo(Position position);
public slots:
    void updateLocation();
    void removePathAction(std::vector<Direction> d);
private:
    Client * client;
    MyTcpSocket * socket;
    //Vars related to position
    Position position;
    bool onGround;
    bool positionSet;
    //Vars related to look
    float yaw;
    float pitch;


};

#endif // PLAYER_H
