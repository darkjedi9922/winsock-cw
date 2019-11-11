#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "Logger.h"
#include "WinSock.h"
#include "ClientSocket.h"

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

    void sendHello();

private slots:
    void checkConnectPossibility() noexcept;
    void connect() noexcept;
    void disconnect() noexcept;

    void onDataSent(int bytes) noexcept;
};

#endif // MAINWINDOW_H
