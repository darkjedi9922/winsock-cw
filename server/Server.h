#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <map>
#include "ServerSocket.h"
#include "definitions.h"

class Server : public QObject
{
    Q_OBJECT

public:
    Server(ServerSocket *socket) noexcept;

    const ControllerInfo& getController(SOCKET socket) const;

signals:
    void controllerConnected(SOCKET socket);
    void controllerUpdated(SOCKET socket);

private:
    ServerSocket *socket;
    std::map<SOCKET, ControllerInfo> controllers;

    void onDataRecieved(SOCKET from, char* buffer, int bytes) noexcept;
    void onClientClosed(SOCKET socket) noexcept;
};

#endif // SERVER_H
