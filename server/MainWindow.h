#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <WinSock2.h>
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
    SOCKET listenSocket;

private slots:
    void initWinsock();
    void startServer();
    void stopServer();
    void cleanWinsock();
};

#endif // MAINWINDOW_H
