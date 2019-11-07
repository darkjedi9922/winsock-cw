#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <WinSock2.h>
#include "WinSock.h"
#include <QObject>
#include <string>

class ClientSocket : public QObject
{
    Q_OBJECT

public:
    ClientSocket(WinSock *);
    ~ClientSocket();

    void connect(std::string ip, std::string port);
    int send(const char *buffer);
    void close();

signals:
    void errorRaised(const QString &msg);

private:
    SOCKET socket;
};

#endif // CLIENTSOCKET_H
