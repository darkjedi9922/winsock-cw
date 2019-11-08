#include "ServerWindow.h"
#include "ui_ServerWindow.h"
#include <WS2tcpip.h>
#include <vector>

using namespace std;

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

void ServerWindow::tableClient(SOCKET client) noexcept
{
    int index = ui->clientsTable->rowCount();
    ui->clientsTable->insertRow(index);

    vector<QString> columns;
    columns.push_back(QString("%1").arg(index + 1));
    columns.push_back(QString::fromStdString(socket->getClientIp(client)));
    columns.push_back(QString("%1").arg(client));
    columns.push_back("?");
    columns.push_back("+00:00");
    columns.push_back("0");
    columns.push_back("0");
    columns.push_back("0");

    for (size_t i = 0; i < columns.size(); ++i) {
        int col = static_cast<int>(i);
        ui->clientsTable->setItem(index, col, new QTableWidgetItem(columns[i]));
    }

    updateClientCount();
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

void ServerWindow::onErrorRaised(const QString &msg) noexcept
{
    systemLogger->write(msg);
}

void ServerWindow::onConnectionAsked() noexcept
{
    SOCKET newClient = socket->acceptClient();
    tableClient(newClient);
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
