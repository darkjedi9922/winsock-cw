#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "ServerSocket.h"
#include "Logger.h"

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

    void startListening();
    void stopListening();

    void tableClient(SOCKET client) noexcept;
    void untableClient(SOCKET client) noexcept;
    void updateClientCount() noexcept;

private slots:
    void onErrorRaised(const QString &msg) noexcept;
    void onConnectionAsked() noexcept;
    void onSocketClosed(SOCKET socket) noexcept;
    void onDataRecieved(SOCKET from, char *buffer, int bytes) noexcept;
};

#endif // MAINWINDOW_H
