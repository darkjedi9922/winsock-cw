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
};

#endif // MAINWINDOW_H
