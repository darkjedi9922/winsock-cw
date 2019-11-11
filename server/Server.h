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

    void start(std::string port);
    void stop();

    const ControllerInfo& getController(SOCKET socket) const;
    size_t getWorkstationSentData(SOCKET socket) const;
    size_t getBufferSize() const;
    void setBufferSize(size_t size) noexcept;

signals:
    void errorRaised(const QString &msg);
    void controllerConnected(SOCKET socket);
    void controllerUpdated(SOCKET socket);
    void controllerTimeDiffSent(SOCKET socket, int bytes);
    void workstationConnected(SOCKET socket);
    void workstationUpdated(SOCKET socket);
    void socketClosed(SOCKET socket);

private:
    ServerSocket *socket;
    std::map<SOCKET, size_t> workstations;
    std::map<SOCKET, ControllerInfo> controllers;
    std::list<std::pair<SOCKET, ControllerDataMessage>> buffer;
    size_t bufferSize;
    QSqlDatabase db;

    void execDbQuery(const QString &query);
    void createDb();
    void saveBuffer();

    void onDataRecieved(SOCKET from, char* buffer, int bytes) noexcept;
    void onClientClosed(SOCKET socket) noexcept;

    void handleWorkstationHello(SOCKET from, const Message *msg);
    void handleControllerHello(SOCKET from, const ControllerInfoMessage *msg);
    void handleData(SOCKET from, const ControllerDataMessage *msg);

    void sendControllerTimeDiff(SOCKET socket);
};

#endif // SERVER_H
