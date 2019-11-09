#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <WinSock2.h>
#include <QObject>
#include <string>
#include "WinSock.h"
#include "SocketEventManager.h"

class ClientSocket : public QObject
{
    Q_OBJECT

public:
    ClientSocket(WinSock *);
    ~ClientSocket();

    void connect(std::string ip, std::string port);
    int send(const char *buffer, int bufferlen);
    void close() noexcept;

    SocketEventManager* getEventManager() const;

private:
    SOCKET socket;
    SocketEventManager *eventManager;
};

#endif // CLIENTSOCKET_H
