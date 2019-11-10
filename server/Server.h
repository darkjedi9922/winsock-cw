#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtSql>
#include <map>
#include "ServerSocket.h"
#include "definitions.h"

class Server : public QObject
{
    Q_OBJECT

public:
    Server(ServerSocket *socket);
    ~Server();

    const ControllerInfo& getController(SOCKET socket) const;

signals:
    void controllerConnected(SOCKET socket);
    void controllerUpdated(SOCKET socket);
    void errorRaised(const QString &msg);

private:
    ServerSocket *socket;
    std::map<SOCKET, ControllerInfo> controllers;
    QSqlDatabase db;

    void execDbQuery(const QString &query);
    void createDb();

    void onDataRecieved(SOCKET from, char* buffer, int bytes) noexcept;
    void onClientClosed(SOCKET socket) noexcept;

    void handleHello(SOCKET from, const ControllerInfoMessage *msg);
    void handleData(SOCKET from, const ControllerDataMessage *msg);
};

#endif // SERVER_H
