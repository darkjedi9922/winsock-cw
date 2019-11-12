#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "Logger.h"
#include "ClientSocket.h"
#include "Controller.h"

namespace Ui {
class ControllerWindow;
}

class ControllerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ControllerWindow(QWidget *parent = nullptr);
    ~ControllerWindow();

private:
    Ui::ControllerWindow *ui;
    Logger *systemLogger;
    Logger *recieveLogger;
    Logger *sendLogger;
    WinSock *winsock;
    ClientSocket* client;
    Controller *controller;

private slots:
    void checkConnectPossibility() noexcept;
    void connect() noexcept;
    void disconnect() noexcept;
    void startSending() noexcept;
    void stopSending() noexcept;

    void onDataRecieved(SOCKET, char *buffer, int bytes) noexcept;
    void onDataSent(int bytes) noexcept;
};

#endif // MAINWINDOW_H
