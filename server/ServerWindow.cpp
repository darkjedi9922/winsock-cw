#include "ServerWindow.h"
#include "ui_ServerWindow.h"
#include <WS2tcpip.h>
#include <string>

ServerWindow::ServerWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerWindow),
    winsock(nullptr),
    socket(nullptr)
{
    ui->setupUi(this);
    ui->clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->stopButton->hide();
    ui->startedLabel->hide();

    systemLogger = new Logger(ui->systemLog);

    try {
        winsock = new WinSock;
        socket = new ServerSocket(winsock);
    }
    catch (const QString &msg) {
        systemLogger->write(msg);
        return;
    }

    QObject::connect(ui->portInput, &QLineEdit::textChanged, [=] (const QString &newText) {
        ui->startButton->setEnabled(newText.contains(QRegExp("^[0-9]{1,5}$")));
    });
    QObject::connect(ui->startButton, &QPushButton::clicked,
                     this, &ServerWindow::startListening);
    QObject::connect(ui->stopButton, &QPushButton::clicked,
                     this, &ServerWindow::stopListening);

    QObject::connect(socket, &ServerSocket::errorRaised, [=] (const QString &msg) {
         systemLogger->write(msg);
    });

    QObject::connect(socket, &ServerSocket::clientAccepted, [=] (SOCKET client) {
        systemLogger->write(QString("New socket %1 was accepted.").arg(client));
    });
    QObject::connect(socket, &ServerSocket::clientClosed, [=] (SOCKET client) {
        systemLogger->write(QString("Socket %1 was closed.").arg(client));
    });
}

ServerWindow::~ServerWindow()
{
    if (socket) {
        socket->close();
        delete socket;
    }
    delete winsock;
    delete systemLogger;
    delete ui;
}

void ServerWindow::startListening()
{
    try {
       socket->listen(ui->portInput->text().toStdString());

       ui->portInput->setReadOnly(true);
       ui->startButton->hide();
       ui->stopButton->show();
       ui->stoppedLabel->hide();
       ui->startedLabel->show();

       systemLogger->write("Server was successfully started.");
    }
    catch (const QString &msg) {
        systemLogger->write(msg);
    }
}

void ServerWindow::stopListening()
{
    try {
        socket->close();

        ui->stopButton->hide();
        ui->startButton->show();
        ui->startedLabel->hide();
        ui->stoppedLabel->show();
        ui->portInput->setReadOnly(false);

        systemLogger->write("Server was stopped.");
    } catch (const QString &msg) {
        systemLogger->write(msg);
    }
}
