#include "SocketEventManager.h"

SocketEventManager::SocketEventManager() :
    count(0)
{
    timerId = startTimer(1000);
}

SocketEventManager::~SocketEventManager()
{
    unsubscribeAll();
    killTimer(timerId);
}

void SocketEventManager::subscribe(SOCKET socket, long events)
{
    WSAEVENT newEvent = WSACreateEvent();
    WSAEventSelect(socket, newEvent, events);
    sockets[count] = socket;
    this->events[count] = newEvent;
    count += 1;
}

/**
 * Remove subscriber from sockets and events arrays and !compress! them
 * (all elements are shifted left by one).
 */
void SocketEventManager::unsubscribe(SOCKET socket) noexcept
{
    size_t index;
    for (index = 0; index < count; ++index) {
        if (sockets[index] == socket) break;
    }

    if (index == count) return;

    for (auto i = index; i < count - 1; ++i) {
        sockets[i] = sockets[i + 1];
        events[i] = events[i + 1];
    }

    count -= 1;
}

void SocketEventManager::unsubscribeAll() noexcept
{
    while (count != 0) unsubscribe(sockets[0]);
}

void SocketEventManager::timerEvent(QTimerEvent *)
{
    DWORD index = WSAWaitForMultipleEvents(count, events, false, 0, false);
    if (index - WSA_WAIT_EVENT_0 >= count) return;

    WSANETWORKEVENTS networkEvents;
    SOCKET socket = sockets[index - WSA_WAIT_EVENT_0];
    WSAEVENT event = events[index - WSA_WAIT_EVENT_0];
    WSAEnumNetworkEvents(socket, event, &networkEvents);

    if (networkEvents.lNetworkEvents & FD_ACCEPT) {
        if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
            emit errorRaised(
                        QString("FD_ACCEPT failed with error %1")
                        .arg(networkEvents.iErrorCode[FD_ACCEPT_BIT])
            );
            return;
        }
        handleAccept(socket);
    }

    if (networkEvents.lNetworkEvents & FD_READ) {
        if (networkEvents.iErrorCode[FD_READ_BIT] != 0) {
            emit errorRaised(
                        QString("FD_READ failed with error %1")
                        .arg(networkEvents.iErrorCode[FD_READ_BIT])
            );
        }
        handleRead(socket);
    }

    if (networkEvents.lNetworkEvents & FD_CLOSE) {
        if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
            emit errorRaised(
                        QString("FD_CLOSE failed with error %1")
                        .arg(networkEvents.iErrorCode[FD_CLOSE_BIT])
            );
            return;
        }
        handleClose(socket);
    }
}

void SocketEventManager::handleAccept(SOCKET socket)
{
    if (count + 1 > WSA_MAXIMUM_WAIT_EVENTS) {
        emit errorRaised("There are too many connections already.");
        return;
    }

    SOCKET client = accept(socket, nullptr, nullptr);
    subscribe(client, FD_READ | FD_WRITE | FD_CLOSE);
    emit socketAccepted(client);
}

void SocketEventManager::handleRead(SOCKET socket)
{
    char buffer[512];
    int bytesRecieved = recv(socket, buffer, sizeof(buffer), 0);
    emit dataRecieved(socket, buffer, bytesRecieved);
}

void SocketEventManager::handleClose(SOCKET socket)
{
    unsubscribe(socket);
    closesocket(socket);
    emit socketClosed(socket);
}
