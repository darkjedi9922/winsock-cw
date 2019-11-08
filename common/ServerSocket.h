#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include <WinSock2.h>
#include <string>
#include <list>
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

    SOCKET acceptClient();
    void closeClient(SOCKET client) noexcept;
    void closeAllClients() noexcept;

    SocketEventManager* getEventManager() const;
    std::string getClientIp(SOCKET client) const;

private:
    SOCKET listenSocket;
    std::list<SOCKET> clients;
    SocketEventManager *eventManager;

    SOCKET bindSocket(std::string port);
};

#endif // SERVERSOCKET_H
