#include "commandmanager.h"
#include "Client.h"

CommandManager::CommandManager(Client * c, MainWindow * i_ui)
{
    client = c;
    ui = i_ui;
    waitingForCoords = false;
}

CommandManager::~CommandManager()
{

}

void CommandManager::readCommand(QString command, QString username)
{
    QStringList args = command.split(' ');
    if(args[0].toLower().compare("!sethome") == 0)
    {
        client->sendMessage("/tp "+username);
        waitingForCoords = true;
        lastUsername = username;
        //From there, we wait to get the coords to set the home
    }
    else if(args[0].toLower().compare("!home") == 0)
    {
        home(username);
    }
    else if(args[0].toLower().compare("!spawn") == 0)
    {
        spawn(username);
    }
    else if(args[0].toLower().compare("!move") == 0)
    {
        if(args.length() == 3)
        {
            bool validDistance;
            double distance = args[2].toDouble(&validDistance);
            if(validDistance)
            {
                if(args[1].toLower().compare("north") == 0)
                {
                    client->movePlayer(NORTH, distance);
                }
                else if(args[1].toLower().compare("south") == 0)
                {
                    client->movePlayer(SOUTH, distance);
                }
                else if(args[1].toLower().compare("east") == 0)
                {
                    client->movePlayer(EAST, distance);
                }
                else if(args[1].toLower().compare("west") == 0)
                {
                    client->movePlayer(WEST, distance);
                }
                else if(args[1].toLower().compare("up") == 0)
                {
                    client->movePlayer(UP, distance);
                }
                else if(args[1].toLower().compare("down") == 0)
                {
                    client->movePlayer(DOWN, distance);
                }
                else
                {
                    client->sendMessage("Direction not recognized.");
                }
            }
            else
            {
                client->sendMessage("Incorrect distance");
            }
        }
        else
        {
            client->sendMessage("Command usage: !move north/south/east/west distance");
        }
    }
    else if(args[0].toLower().compare("!getblock") == 0)
    {
        if(args.length() == 4)
        {
            Block block = client->world->getBlock(Position(args[1].toDouble(), args[2].toDouble(), args[3].toDouble()));
            client->sendMessage(QString::number(block.getType()));
        }
    }
    else if(args[0].toLower().compare("!goto") == 0)
    {
        if(args.length() == 4)
        {
            client->player->goTo(Position(args[1].toDouble(), args[2].toDouble(), args[3].toDouble()));
        }
    }
    else
    {
        client->sendMessage("Command not recognized! ");
    }
}

void CommandManager::setHome(double x, double y, double z)
{
    waitingForCoords = false;
    //client->sendMessage("Setting home for " + lastUsername + " at coords " + QString::number(x) + "," + QString::number(y) + "," + QString::number(z));

    std::fstream homeFile;
    std::vector<QString> homes;
    homeFile.open("homes_" + client->ip.toStdString() + ".txt", std::ios::in);
    string line;

    bool playerFound = false;
    while(getline(homeFile, line))
    {
        QString qline = QString::fromStdString(line).simplified();
        QStringList qlines = qline.split(' ');
        if(qlines.at(0).compare(lastUsername) == 0)
        {//Update home for the player
            playerFound = true;
            homes.push_back(lastUsername + " " + QString::number(x) + " " + QString::number(y) + " " + QString::number(z));
        }
        else
        {
            homes.push_back(qline);
        }

    }
    if(!playerFound)
    {
        homes.push_back(lastUsername + " " + QString::number(x) + " " + QString::number(y) + " " + QString::number(z));
    }
    homeFile.close();

    //Write the file back
    homeFile.open("homes_" + client->ip.toStdString() + ".txt", std::ios::out);
    for(size_t i = 0; i < homes.size(); i++)
    {
        homeFile << homes[i].toStdString() << std::endl;
    }
    homeFile.close();
}

void CommandManager::home(QString username)
{
    std::fstream homeFile;
    homeFile.open("homes_" + client->ip.toStdString() + ".txt", std::ios::in);
    string line;

    bool playerFound = false;
    while(getline(homeFile, line))
    {
        QString qline = QString::fromStdString(line).simplified();
        QStringList qlines = qline.split(' ');
        if(qlines.at(0).compare(username) == 0)
        {//Update home for the player
            playerFound = true;
            client->sendMessage("/tp "+qline);
            //client->sendMessage("Done!");
        }
    }
    if(!playerFound)
    {
        client->sendMessage("No home found for user "+username);
    }
    homeFile.close();
}

void CommandManager::spawn(QString username)
{
    std::fstream spawnFile;
    spawnFile.open("spawn_" + client->ip.toStdString() + ".txt", std::ios::in);
    string line;
    getline(spawnFile, line);
    QString qline = QString::fromStdString(line).simplified();
    if(line == "")
    {
        client->sendMessage("No spawn found!");
    }
    else
    {
        client->sendMessage("/tp "+username + " " +qline);
    }
}
