#include "ServerSocket.h"
#include <WS2tcpip.h>
#include <QString>

using namespace std;

ServerSocket::ServerSocket(WinSock *) noexcept :
    QObject(),
    listenSocket(INVALID_SOCKET),
    subscribersCount(0),
    timerId(-1)
{}

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

    subscribe(listenSocket, FD_ACCEPT);
    timerId = startTimer(100);
}

void ServerSocket::close() noexcept
{
    if (listenSocket == INVALID_SOCKET) return;
    killTimer(timerId);
    removeSubscribe(listenSocket);
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;
}

void ServerSocket::timerEvent(QTimerEvent *)
{
    WSANETWORKEVENTS networkEvents;
    DWORD index = WSAWaitForMultipleEvents(subscribersCount, subscribedEvents,
                                           false, 100, false);
    WSAEnumNetworkEvents(subscribedSockets[index - WSA_WAIT_EVENT_0],
            subscribedEvents[index - WSA_WAIT_EVENT_0],
            &networkEvents);

    if (networkEvents.lNetworkEvents & FD_ACCEPT) {
        if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
            emit errorRaised(
                        QString("FD_ACCEPT failed with error %1")
                        .arg(networkEvents.iErrorCode[FD_ACCEPT_BIT])
            );
            return;
        }
    }

    if (networkEvents.lNetworkEvents & FD_CLOSE) {
        if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
            emit errorRaised(
                        QString("FD_CLOSE failed with error %1")
                        .arg(networkEvents.iErrorCode[FD_CLOSE_BIT])
            );
            return;
        }

        removeSubscribe(subscribedSockets[index - WSA_WAIT_EVENT_0]);
        closesocket(subscribedSockets[index - WSA_WAIT_EVENT_0]);
    }
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

void ServerSocket::subscribe(SOCKET socket, long events)
{
    WSAEVENT newEvent = WSACreateEvent();
    WSAEventSelect(socket, newEvent, events);
    subscribedSockets[subscribersCount] = socket;
    subscribedEvents[subscribersCount] = newEvent;
    subscribersCount += 1;
}

void ServerSocket::removeSubscribe(SOCKET socket) noexcept
{
    size_t index;
    for (index = 0; index < subscribersCount; ++index) {
        if (subscribedSockets[index] == socket) break;
    }

    if (index == subscribersCount) return;

    for (auto i = index; i < subscribersCount - 1; ++i) {
        subscribedSockets[i] = subscribedSockets[i + 1];
        subscribedEvents[i] = subscribedEvents[i + 1];
    }

    subscribersCount -= 1;
}
