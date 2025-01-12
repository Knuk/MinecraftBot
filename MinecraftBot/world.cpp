#include "world.h"

World::World()
{

}

World::~World()
{

}

void World::addChunk(QByteArray data) //Packet 0x21 (33)
{
    QDataStream stream(data);

    int chunkX, chunkZ;
    stream >> chunkX;
    stream >> chunkZ;

    bool groundContinuous;
    stream >> groundContinuous;

    unsigned short bitmask;
    stream >> bitmask;

    data.remove(0, 11); //Remove the 2 ints, the bool and the short

    int nbBytesDecoded;
    uint8_t * buffer = (uint8_t*)data.data();
    int size = Varint::decode_unsigned_varint(buffer, nbBytesDecoded);
    data.remove(0, nbBytesDecoded);

    //All the data left is a chunk
    if(bitmask == 0) //The server is telling us to unload the chunk specified
    {
        unloadChunk(chunkX, chunkZ);
    }
    else
    {
        ChunkColumn actualColumn = ChunkColumn(chunkX, chunkZ, bitmask);
        for (int j=0; j<16; j++) //The chunks inside a column
        {
            if (actualColumn.bitmask & (1 << j))
            {

                //Blocks
                stream.setByteOrder(stream.LittleEndian); //Blocks are read in little-endian order
                for(int y = 0; y < 16; y++)
                {
                    for(int z = 0; z < 16; z++)
                    {
                        for(int x = 0; x < 16; x++)
                        {
                            unsigned short type;
                            stream >> type;
                            Position pos = Position(actualColumn.position_x*16 + x, j*16 + y, actualColumn.position_z * 16 + z);
                            allBlocks.insert(pos, Block(type));
                        }
                    }
                }

            }
        }
        //Lights
        stream.setByteOrder(stream.BigEndian);
        for (int j=0; j<16; j++) //The chunks inside a column
        {
            if (actualColumn.bitmask & (1 << j))
            {
                for(int y = 0; y < 16; y++)
                {
                    for(int z = 0; z < 16; z++)
                    {
                        for(int x = 0; x < 16; x+=2)
                        {
                            unsigned char light;
                            stream >> light;
                            allBlocks[Position(chunkX*16 + x, j*16 + y, chunkZ * 16 + z)].light = (light >> 4); //Even index = high bits
                            allBlocks[Position(chunkX*16 + x+1, j*16 + y, chunkZ * 16 + z)].light = (light & 15); //Odd index = low bits
                        }
                    }
                }
            }
        }
        chunkColumns.insert(std::make_pair(std::make_pair(actualColumn.position_x, actualColumn.position_z), actualColumn));
    }
}

void World::addChunks(QByteArray data) //Packet 0x26 (38)
{
    bool skyLightSent = data.at(0);
    data.remove(0, 1);

    int nbBytesDecoded;
    uint8_t * buffer = (uint8_t*)data.data();
    int chunkColumnCount = Varint::decode_unsigned_varint(buffer, nbBytesDecoded);
    data.remove(0, nbBytesDecoded);

    QDataStream stream(data);
    int chunkX, chunkZ;

    std::vector<ChunkColumn> chunkColumnsVector;

    for(int i = 0; i < chunkColumnCount; i++)
    {
        stream >> chunkX;
        stream >> chunkZ;

        unsigned short bitmask;
        stream >> bitmask;

        chunkColumnsVector.push_back(ChunkColumn(chunkX, chunkZ, bitmask));

    }

    for(int i = 0; i < chunkColumnCount; i++)
    {
        ChunkColumn actualColumn = chunkColumnsVector.at(i);
        //Blocks
        for (int j=0; j<16; j++) //The chunks inside a column
        {
            if (actualColumn.bitmask & (1 << j))
            {

                //Blocks
                stream.setByteOrder(stream.LittleEndian); //Blocks are read in little-endian order
                for(int y = 0; y < 16; y++)
                {
                    for(int z = 0; z < 16; z++)
                    {
                        for(int x = 0; x < 16; x++)
                        {
                            unsigned short type;
                            stream >> type;
                            Position pos = Position(actualColumn.position_x*16 + x, j*16 + y, actualColumn.position_z * 16 + z);
                            allBlocks.insert(pos, Block(type));
                        }
                    }
                }

            }
        }
        //Lights
        stream.setByteOrder(stream.BigEndian);
        for (int j=0; j<16; j++) //The chunks inside a column
        {
            if (actualColumn.bitmask & (1 << j))
            {
                for(int y = 0; y < 16; y++)
                {
                    for(int z = 0; z < 16; z++)
                    {
                        for(int x = 0; x < 16; x+=2)
                        {
                            unsigned char light;
                            stream >> light;
                            allBlocks[Position(chunkX*16 + x, j*16 + y, chunkZ * 16 + z)].light = (light >> 4); //Even index = high bits
                            allBlocks[Position(chunkX*16 + x+1, j*16 + y, chunkZ * 16 + z)].light = (light & 15); //Odd index = low bits
                        }
                    }
                }
            }
        }
        //Skylight
        if(skyLightSent)
        {
            for (int j=0; j<16; j++) //The chunks inside a column
            {
                if (actualColumn.bitmask & (1 << j))
                {
                    for(int y = 0; y < 16; y++)
                    {
                        for(int z = 0; z < 16; z++)
                        {
                            for(int x = 0; x < 16; x+=2)
                            {
                                unsigned char light;
                                stream >> light;
                                allBlocks[Position(chunkX*16 + x, j*16 + y, chunkZ * 16 + z)].light = (light >> 4); //Even index = high bits
                                allBlocks[Position(chunkX*16 + x+1, j*16 + y, chunkZ * 16 + z)].light = (light & 15); //Odd index = low bits
                            }
                        }
                    }
                }
            }
        }
        stream.skipRawData(256); //Skip biomes
        chunkColumns.insert(std::make_pair(std::make_pair(actualColumn.position_x, actualColumn.position_z), actualColumn));
    }

}

void World::updateBlock(QByteArray data) //Packet 0x23 (35)
{
    long long xyz;
    QDataStream stream(data);
    stream >> xyz;

    //Taken on http://wiki.vg/Protocol#Position
    Position pos;
    pos.x = xyz >> 38;
    pos.y = (xyz >> 26) & 0xFFF;
    pos.z = xyz << 38 >> 38;

    data.remove(0, 8); //Remove the 3 "ints"

    //Get the id
    int nbBytesDecoded;
    uint8_t * buffer = (uint8_t*)data.data();
    int id = Varint::decode_unsigned_varint(buffer, nbBytesDecoded);

    setBlock(pos, id);

}

void World::updateBlocks(QByteArray data) //Packet 0x22 (34)
{
    int chunkX, chunkZ;
    QDataStream stream(data);

    stream >> chunkX;
    stream >> chunkZ;
    data.remove(0, 8);

    //Get the number of blocks to change
    int nbBytesDecoded;
    uint8_t * buffer = (uint8_t*)data.data();
    int nbBlocks = Varint::decode_unsigned_varint(buffer, nbBytesDecoded);

    stream.skipRawData(nbBytesDecoded); //Skip decoded bytes in the stream
    data.remove(0, nbBytesDecoded); //Remove them from the array as well

    for(int i = 0; i < nbBlocks; i++)
    {
        unsigned char xz, blockY;
        stream >> xz;
        stream >> blockY;
        data.remove(0, 2);

        unsigned char blockX = (xz >> 4);
        unsigned char blockZ = (xz & 15);

        uint8_t * buffer = (uint8_t*)data.data();
        int id = Varint::decode_unsigned_varint(buffer, nbBytesDecoded);

        data.remove(0, nbBytesDecoded);
        stream.skipRawData(nbBytesDecoded);

        Position pos = Position((chunkX * 16) + blockX, blockY, (chunkZ * 16) + blockZ);

        setBlock(pos, id);

    }

}

Block World::getBlock(Position pos)
{
    if(allBlocks.contains(pos.getFloored()))
    {
        return allBlocks[pos.getFloored()];
    }
    else
    {
        Block b = Block();
        return b;
    }
}

void World::setBlock(Position pos, int i)
{
    if(allBlocks.contains(pos.getFloored()))
    {
        allBlocks[pos.getFloored()].type = i;
    }
}

bool World::canGo(Position pos, Direction d)
{
    /* The Block::walkableBlocks array works this way:
     * 0 is walkable,
     * 1 is solid block,
     * 2 is liquid,
     * 3 is avoid because dangereous,
     * 4 doesn't exist,
     * 5 is halfblock,
     * 6 is walkable through but you don't fall
     */
    switch(d)
    {
    case UP:
    {
        char typeBlockBelow = Block::walkableBlocks[getBlock(pos+DOWN).getType()];              //The type of the block below
        char typeBlockActual = Block::walkableBlocks[getBlock(pos).getType()];                 //The type of the block where the player is
        char typeBlockAbove = Block::walkableBlocks[getBlock(pos + UP + UP).getType()];         //The type of the block above
        bool solidBelow = (typeBlockBelow == 1 || typeBlockBelow == 5 || typeBlockBelow == 6);  //The block below has to be solid
        bool liquidActual = (typeBlockBelow == 2);
        bool emptyAbove = (typeBlockAbove == 0 || typeBlockAbove == 2);                   //The block above has to be walkable through
        return ((solidBelow || liquidActual) && emptyAbove);
    }
        break;
    case DOWN:
    {
        char typeBlockBelow = Block::walkableBlocks[getBlock(pos+DOWN).getType()];      //The type of the block below
        bool emptyBelow = (typeBlockBelow == 0 || typeBlockBelow == 2);                 //The block below has to be empty to fall through
        return emptyBelow;
    }
        break;
    case NORTH: case SOUTH: case WEST: case EAST:
    {
        char typeBlockBelow = Block::walkableBlocks[getBlock(pos+DOWN).getType()];              //The type of the block below
        char typeBlockBelowDirection = Block::walkableBlocks[getBlock(pos+DOWN+d).getType()];   //The type of the block below the destination
        char typeBlockActual = Block::walkableBlocks[getBlock(pos).getType()]; //the block where the player is
        char typeBlockDirection1 = Block::walkableBlocks[getBlock(pos + d).getType()];
        char typeBlockDirection2 = Block::walkableBlocks[getBlock(pos + d + UP).getType()]; //The two blocks at the destination

        bool solidBelow = (typeBlockBelow == 1 || typeBlockBelow == 5 || typeBlockBelow == 6);
        bool solidBelowDirection = (typeBlockBelowDirection == 1 || typeBlockBelowDirection == 5 || typeBlockBelowDirection == 6);
        bool oneBlockBelowSolid = (solidBelow || solidBelowDirection); //The block has to be solid under the player or under the target to walk there
        bool bothLiquids = (typeBlockActual == 2 && typeBlockDirection1 == 2); //Can move through water
        bool blockEmpty1 = (typeBlockDirection1 == 0 || typeBlockDirection1 == 2);
        bool blockEmpty2 = (typeBlockDirection2 == 0 || typeBlockDirection2 == 2);

        return((oneBlockBelowSolid || bothLiquids) && blockEmpty1 && blockEmpty2);
    }
        break;
    default:
        return false;
    }
}

void World::unloadChunk(int x, int z) //To remove a chunk, the server tells us what to unload
{
    std::map<std::pair<int, int>, ChunkColumn>::iterator it;
    it=chunkColumns.find(std::make_pair(x, z));
    if(it != chunkColumns.end())
    {
        chunkColumns.erase(it);
        for(int xx = 0; xx < 16; xx++)
        {
            for(int zz = 0; zz < 16; zz++)
            {
                for(int yy = 0; yy < 256; yy++)
                {
                    if(allBlocks.find(Position(xx, yy, zz)) != allBlocks.end())
                    {
                        allBlocks.remove(Position(xx, yy, zz));
                    }
                }
            }
        }
    }
}
