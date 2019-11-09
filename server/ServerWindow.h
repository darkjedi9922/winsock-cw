#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "Logger.h"
#include "Server.h"
#include "ServerSocket.h"

namespace Ui {
class ServerWindow;
}

class ServerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:
    Ui::ServerWindow *ui;
    Logger *systemLogger;
    WinSock *winsock;
    ServerSocket *socket;
    Server *server;

    void startListening();
    void stopListening();

    void tableClient(SOCKET client) noexcept;
    void updateClient(SOCKET client) noexcept;
    void untableClient(SOCKET client) noexcept;
    void updateClientCount() noexcept;

    int findClientTableRow(SOCKET client) noexcept;

private slots:
    void onErrorRaised(const QString &msg) noexcept;
    void onConnectionAsked() noexcept;
    void onSocketClosed(SOCKET socket) noexcept;
    void onDataRecieved(SOCKET from, char *buffer, int bytes) noexcept;

    void onControllerConnected(SOCKET socket) noexcept;
    void onControllerUpdated(SOCKET socket) noexcept;
};

#endif // MAINWINDOW_H
