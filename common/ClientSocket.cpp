#include "ClientSocket.h"
#include <WS2tcpip.h>

using namespace std;

ClientSocket::ClientSocket(WinSock *) :
    QObject(),
    socket(INVALID_SOCKET),
    eventManager(new SocketEventManager)
{}

ClientSocket::~ClientSocket()
{
    close();
    delete eventManager;
}

void ClientSocket::connect(string ip, string port)
{
    if (socket != INVALID_SOCKET) throw QString("This socket was connected already.");

    struct addrinfo *result = nullptr, *ptr = nullptr, hints;
    int iResult;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
    if (iResult != 0) {
        throw QString("getaddrinfo failed with error: %1").arg(iResult);
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        socket = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (socket == INVALID_SOCKET) {
            throw QString("socket failed with error: %1").arg(WSAGetLastError());
        }

        // Connect to server.
        iResult = ::connect(socket, ptr->ai_addr,
                            static_cast<int>(ptr->ai_addrlen));

        if (iResult == SOCKET_ERROR) {
            closesocket(socket);
            socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (socket == INVALID_SOCKET) {
        throw QString("Unable to connect to server!");
    }

    eventManager->subscribe(socket, FD_READ | FD_CLOSE);
}

int ClientSocket::send(const char *buffer, int bufferlen)
{
    int result = ::send(socket, buffer, bufferlen, 0);
    if (result == SOCKET_ERROR) {
        throw QString("Send failed with error: %1").arg(WSAGetLastError());
    }

    return result;
}

void ClientSocket::close() noexcept
{
    if (socket == INVALID_SOCKET) return;
    eventManager->unsubscribe(socket);
    shutdown(socket, SD_BOTH);
    closesocket(socket);
    socket = INVALID_SOCKET;
}

SocketEventManager* ClientSocket::getEventManager() const
{
    return eventManager;
}
