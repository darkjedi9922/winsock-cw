#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "Logger.h"
#include "ClientSocket.h"

namespace Ui {
class ControllerWindow;
}

class ControllerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ControllerWindow(QWidget *parent = nullptr);
    ~ControllerWindow();

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    Ui::ControllerWindow *ui;
    Logger *systemLogger;
    WinSock *winsock;
    ClientSocket* client;
    int timerId;

private slots:
    void checkConnectPossibility() noexcept;
    void connect() noexcept;
    void disconnect() noexcept;
    void startSending() noexcept;
    void stopSending() noexcept;
};

#endif // MAINWINDOW_H
