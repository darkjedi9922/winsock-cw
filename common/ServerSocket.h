#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include "WinSock.h"
#include <WinSock2.h>
#include <string>

class ServerSocket
{
public:
    ServerSocket(WinSock *) noexcept;

    void listen(std::string port);
    void close() noexcept;

private:
    SOCKET listenSocket;

    SOCKET bindSocket(std::string port);
};

#endif // SERVERSOCKET_H
