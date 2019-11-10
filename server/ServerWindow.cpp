#include "ServerWindow.h"
#include "ui_ServerWindow.h"
#include <WS2tcpip.h>
#include <vector>

using namespace std;

ServerWindow::ServerWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerWindow),
    winsock(nullptr),
    socket(nullptr),
    server(nullptr)
{
    ui->setupUi(this);
    ui->clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->stopButton->hide();
    ui->startedLabel->hide();

    systemLogger = new Logger(ui->systemLog);

    try {
        winsock = new WinSock;
        socket = new ServerSocket(winsock);
        server = new Server(socket);
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

    QObject::connect(server, &Server::controllerConnected,
                     this, &ServerWindow::onControllerConnected);
    QObject::connect(server, &Server::controllerUpdated,
                     this, &ServerWindow::onControllerUpdated);
}

ServerWindow::~ServerWindow()
{
    if (server) {
        delete server;
        server = nullptr;
    }
    if (socket) {
        socket->close();
        delete socket;
        socket = nullptr;
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

void ServerWindow::tableClient(SOCKET client) noexcept
{
    auto info = server->getController(client);
    int row = ui->clientsTable->rowCount();
    ui->clientsTable->insertRow(row);

    vector<QString> columns;
    columns.push_back(QString("%1").arg(row + 1));
    columns.push_back(QString::fromStdString(info.ip));
    columns.push_back(QString("%1").arg(client));
    columns.push_back(QString("КОМ %1").arg(info.number));
    columns.push_back(QString::fromStdString(info.formatDiffTime()));
    columns.push_back(QString("%1").arg(info.recievedData));
    columns.push_back(QString("%2").arg(info.savedData));
    columns.push_back("-");

    for (size_t i = 0; i < columns.size(); ++i) {
        int col = static_cast<int>(i);
        ui->clientsTable->setItem(row, col, new QTableWidgetItem(columns[i]));
    }

    updateClientCount();
}

void ServerWindow::updateClient(SOCKET client) noexcept
{
    auto info = server->getController(client);
    int row = findClientTableRow(client);
    ui->clientsTable->item(row, 4)->setText(QString::fromStdString(info.formatDiffTime()));
    ui->clientsTable->item(row, 5)->setText(QString("%1").arg(info.recievedData));
    ui->clientsTable->item(row, 6)->setText(QString("%1").arg(info.savedData));
}

void ServerWindow::untableClient(SOCKET client) noexcept
{
    int row;
    for (row = 0; row < ui->clientsTable->rowCount(); ++row) {
        if (ui->clientsTable->item(row, 2)->text().toUInt() == client) break;
    }

    ui->clientsTable->removeRow(row);
    updateClientCount();
}

void ServerWindow::updateClientCount() noexcept
{
    int count = ui->clientsTable->rowCount();
    auto color = QString("color: %1").arg(count ? "green" : "red");
    ui->clientCount->setNum(count);
    ui->clientCount->setStyleSheet(color);
}

int ServerWindow::findClientTableRow(SOCKET client) noexcept
{
    for (int i = 0; i < ui->clientsTable->rowCount(); ++i) {
        if (ui->clientsTable->item(i, 2)->text().toUInt() == client) return i;
    }
    return -1;
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
    untableClient(socket);
    systemLogger->write(QString("Socket %1 was closed.").arg(socket));
}

void ServerWindow::onDataRecieved(SOCKET from, char *, int bytes) noexcept
{
    systemLogger->write(
        QString("There was %1 bytes recieved from %2 socket").arg(bytes).arg(from));
}

void ServerWindow::onControllerConnected(SOCKET client) noexcept
{
    tableClient(client);
}

void ServerWindow::onControllerUpdated(SOCKET client) noexcept
{
    updateClient(client);
}
