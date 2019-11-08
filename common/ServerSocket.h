#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include <WinSock2.h>
#include <string>
#include "WinSock.h"
#include "SocketEventManager.h"

class ServerSocket : public QObject
{
    Q_OBJECT

public:
    ServerSocket(WinSock *) noexcept;
    ~ServerSocket() noexcept;

    void listen(std::string port);
    void close() noexcept;

    SocketEventManager* getEventManager() const;

private:
    SOCKET listenSocket;
    SocketEventManager *eventManager;

    SOCKET bindSocket(std::string port);
};

#endif // SERVERSOCKET_H
