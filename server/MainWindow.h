#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <WinSock2.h>
#include <QPlainTextEdit>

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
    SOCKET listenSocket;

    void systemLog(const QString &string);
    void systemLog(const QStringList &messages);
    void log(QPlainTextEdit *logEditor, const QStringList &messages);

private slots:
    void initWinsock();
    void startServer();
    void stopServer();
    void cleanWinsock();
};

#endif // MAINWINDOW_H
