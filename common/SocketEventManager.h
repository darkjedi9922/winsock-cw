#ifndef SOCKETEVENTMANAGER_H
#define SOCKETEVENTMANAGER_H

#include <QObject>
#include <WinSock2.h>

class SocketEventManager : public QObject
{
    Q_OBJECT
public:
    SocketEventManager();
    ~SocketEventManager();

    void subscribe(SOCKET socket, long events) noexcept;
    void unsubscribe(SOCKET socket) noexcept;
    void unsubscribeAll() noexcept;

signals:
    void errorRaised(const QString &message);
    void connectionAsked();
    void socketClosed(SOCKET socket);
    void dataRecieved(SOCKET from, char *buffer, int bytes);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    SOCKET sockets[WSA_MAXIMUM_WAIT_EVENTS];
    WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
    size_t count;
    int timerId;

    void handleAccept() noexcept;
    void handleRead(SOCKET socket) noexcept;
    void handleClose(SOCKET socket) noexcept;
};

#endif // SOCKETEVENTMANAGER_H
