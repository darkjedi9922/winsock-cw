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

private:
    Ui::ControllerWindow *ui;
    Logger *systemLogger;
    WinSock *winsock;
    ClientSocket* client;
};

#endif // MAINWINDOW_H
