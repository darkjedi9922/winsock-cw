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

    auto eventManager = socket->getEventManager();
    QObject::connect(eventManager, &SocketEventManager::errorRaised,
                     this, &ServerWindow::onErrorRaised);
    QObject::connect(eventManager, &SocketEventManager::connectionAsked,
                     this, &ServerWindow::onConnectionAsked);
    QObject::connect(eventManager, &SocketEventManager::socketClosed,
                     this, &ServerWindow::onSocketClosed);
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &ServerWindow::onDataRecieved);
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

void ServerWindow::onErrorRaised(const QString &msg) noexcept
{
    systemLogger->write(msg);
}

void ServerWindow::onConnectionAsked() noexcept
{
    SOCKET newClient = socket->acceptClient();
    systemLogger->write(QString("New socket %1 was accepted.").arg(newClient));
}

void ServerWindow::onSocketClosed(SOCKET socket) noexcept
{
    systemLogger->write(QString("Socket %1 was closed.").arg(socket));
}

void ServerWindow::onDataRecieved(SOCKET from, char *, int bytes) noexcept
{
    systemLogger->write(
        QString("There was %1 bytes recieved from %2 socket").arg(bytes).arg(from));
}
