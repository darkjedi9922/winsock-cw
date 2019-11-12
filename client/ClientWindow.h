#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "Logger.h"
#include "WinSock.h"
#include "ClientSocket.h"
#include "definitions.h"

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private:
    Ui::ClientWindow *ui;
    Logger *systemLogger;
    WinSock *winsock;
    ClientSocket* socket;

    void sendHello() noexcept;
    void addRecord(const WorkstationAnswer *answer) noexcept;

private slots:
    void checkConnectPossibility() noexcept;
    void connect() noexcept;
    void disconnect() noexcept;
    void requestData() noexcept;

    void onDataSent(int bytes) noexcept;
    void onDataRecieved(SOCKET socket, char *buffer, int bytes);
};

#endif // MAINWINDOW_H
