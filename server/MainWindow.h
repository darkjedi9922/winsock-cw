#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "ServerSocket.h"
#include "Logger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Logger *systemLogger;
    WinSock *winsock;
    ServerSocket *socket;

    void startListening();
    void stopListening();
};

#endif // MAINWINDOW_H
