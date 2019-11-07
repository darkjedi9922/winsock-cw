#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include "WinSock.h"
#include <WinSock2.h>
#include <string>

class ServerSocket : public QObject
{
    Q_OBJECT

public:
    ServerSocket(WinSock *) noexcept;

    void listen(std::string port);
    void close() noexcept;

signals:
    void errorRaised(const QString &message);
    void clientAccepted(SOCKET client);
    void clientClosed(SOCKET client);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    SOCKET listenSocket;
    SOCKET subscribedSockets[WSA_MAXIMUM_WAIT_EVENTS];
    WSAEVENT subscribedEvents[WSA_MAXIMUM_WAIT_EVENTS];
    size_t subscribersCount;
    int timerId;

    SOCKET bindSocket(std::string port);
    void subscribe(SOCKET socket, long events);
    void removeSubscribe(SOCKET socket) noexcept;
};

#endif // SERVERSOCKET_H
