#include "ServerSocket.h"
#include <WS2tcpip.h>
#include <QString>

using namespace std;

ServerSocket::ServerSocket(WinSock *) noexcept :
    QObject(),
    listenSocket(INVALID_SOCKET),
    eventManager(new SocketEventManager)
{}

ServerSocket::~ServerSocket() noexcept
{
    close();
    delete eventManager;
}

void ServerSocket::listen(string port)
{
    if (listenSocket != INVALID_SOCKET) throw QString("Socket is listening already");
    listenSocket = bindSocket(port);
    int result = ::listen(listenSocket, SOMAXCONN);

    if (result == SOCKET_ERROR) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        throw QString("listen failed with error: %1").arg(WSAGetLastError());
    }

    eventManager->subscribe(listenSocket, FD_ACCEPT);
}

void ServerSocket::close() noexcept
{
    if (listenSocket == INVALID_SOCKET) return;   
    eventManager->unsubscribeAll();
    closeAllClients();
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;
}

SOCKET ServerSocket::acceptClient()
{
    SOCKET newClient = ::accept(listenSocket, nullptr, nullptr);
    if (newClient == INVALID_SOCKET) throw QString("There is no new client");
    clients.push_back(newClient);
    eventManager->subscribe(newClient, FD_READ | FD_WRITE | FD_CLOSE);
    return newClient;
}

void ServerSocket::closeClient(SOCKET client) noexcept
{
    eventManager->unsubscribe(client);
    clients.remove(client);
    shutdown(client, SD_BOTH);
    closesocket(client);
}

void ServerSocket::closeAllClients() noexcept
{
    while (!clients.empty()) closeClient(clients.front());
}

SocketEventManager* ServerSocket::getEventManager() const
{
    return eventManager;
}

string ServerSocket::getClientIp(SOCKET client) const
{
    SOCKADDR_IN info = {}; // For TCP/IP.
    int addrsize = sizeof(info); // This is probably too.
    getsockname(client, reinterpret_cast<struct sockaddr*>(&info), &addrsize);

    char buffer[INET_ADDRSTRLEN];
    inet_ntop(info.sin_family, &info.sin_addr, buffer, INET_ADDRSTRLEN);
    return buffer;
}

SOCKET ServerSocket::bindSocket(string port)
{
    SOCKET resultSocket = INVALID_SOCKET;
    struct addrinfo *addressInfo = nullptr;
    struct addrinfo hints;
    int iResult;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(nullptr, port.c_str(), &hints, &addressInfo);
    if (iResult != 0) {
        throw QString("getaddrinfo failed with error: %1").arg(WSAGetLastError());
    }

    // Create a SOCKET for connecting to server
    resultSocket = socket(addressInfo->ai_family,
                          addressInfo->ai_socktype,
                          addressInfo->ai_protocol);

    if (resultSocket == INVALID_SOCKET) {
        freeaddrinfo(addressInfo);
        throw QString("socket failed with error: %1").arg(WSAGetLastError());
    }

    iResult = ::bind(resultSocket,
                   addressInfo->ai_addr,
                   static_cast<int>(addressInfo->ai_addrlen));

    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(addressInfo);
        closesocket(resultSocket);
        throw QString("bind failed with error: %1").arg(WSAGetLastError());
    }

    freeaddrinfo(addressInfo);
    return resultSocket;
}
