#include "player.h"
#include "Client.h"
#include "playerposition.h"
#include "onground.h"

/* Directions:
 * North: z--
 * South: z++
 * East: x++
 * West: x--
 */

Player::Player(Client * c)
{
    client = c;
    position_x = 0;
    position_y = 0;
    position_z = 0;
    yaw = 0;
    pitch = 0;
    onGround = true;
    positionSet = false;
}

Player::~Player()
{

}

void Player::setPositionAndLook(double posx, double posy, double posz, float y, float p, char flags)
{
    if((flags & 1) != 0) //first bit set, relative x
    {
        position_x += posx;
    }
    else //Absolute
    {
        position_x = posx;
    }

    if((flags & 2) != 0) //second bit set, relative y
    {
        position_y += posy;
    }
    else //Absolute
    {
        position_y = posy;
    }

    if((flags & 4) != 0) //third bit set, relative z
    {
        position_z += posz;
    }
    else //Absolute
    {
        position_z = posz;
    }

    if((flags & 8) != 0) //fourth bit set, relative pitch
    {
        pitch += p;
    }
    else //Absolute
    {
        pitch = p;
    }

    if((flags & 16) != 0) //fifth bit set, relative yaw
    {
        yaw += y;
    }
    else //Absolute
    {
        yaw = y;
    }

    positionSet = true;
}

void Player::move(Direction d, MyTcpSocket * socket)
{
    float speed = 0.1;
    if(positionSet)
    {
        switch(d)
        {
        case NORTH:
            position_z-=speed;
            break;
        case SOUTH:
            position_z+=speed;
            break;
        case EAST:
            position_x+=speed;
            break;
        case WEST:
            position_x-=speed;
            break;
        case NORTHEAST:
            position_z-=speed;
            position_x+=speed;
            break;
        case NORTHWEST:
            position_z-=speed;
            position_x-=speed;
            break;
        case SOUTHEAST:
            position_z+=speed;
            position_x+=speed;
            break;
        case SOUTHWEST:
            position_z+=speed;
            position_x-=speed;
            break;
        case UP:
            position_y+=speed;
            break;
        case DOWN:
            position_y-=speed;
            break;
        }
        //Update the position to the server
        PlayerPosition pp = PlayerPosition(socket, position_x, position_y, position_z, onGround);
        pp.sendPacket(client->compressionSet);
    }
}

void Player::updateGround(MyTcpSocket * socket)
{
    OnGround og = OnGround(socket, onGround);
}