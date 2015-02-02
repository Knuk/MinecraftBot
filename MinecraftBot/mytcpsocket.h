/* mytcpsocket.h
 * Header file for the mytcpsocket class
 * Taken from http://www.bogotobogo.com/Qt/Qt5_QTcpSocket_Signals_Slots.php
 * */
#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDebug>
#include <vector>

using namespace std;

class MainWindow;
class Client;
class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = 0);
    
    void doConnect(const QString &ip, const int port);
    MainWindow * ui; //For writing in the console
    Client * client; //To send the client the packets
    void write(QByteArray data);
    bool connectedBool;

signals:

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    QTcpSocket *socket;
    
};

#endif // MYTCPSOCKET_H
