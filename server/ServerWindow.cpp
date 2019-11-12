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
    ui->clientsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->stopButton->hide();
    ui->startedLabel->hide();

    systemLogger = new Logger(ui->systemLog);
    recieveLogger = new Logger(ui->recieveLog);
    sendLogger = new Logger(ui->sendLog);

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

    QObject::connect(ui->bufferSize, SIGNAL(valueChanged(int)),
                     this, SLOT(onBufferSizeChanged(int)));

    auto eventManager = socket->getEventManager();
    QObject::connect(eventManager, SIGNAL(errorRaised(const QString &)),
                     systemLogger, SLOT(write(const QString &)));
    QObject::connect(eventManager, &SocketEventManager::connectionAsked,
                     this, &ServerWindow::onConnectionAsked);
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &ServerWindow::onDataRecieved);

    QObject::connect(server, SIGNAL(errorRaised(const QString &)),
                     systemLogger, SLOT(write(const QString &)));
    QObject::connect(server, &Server::controllerConnected,
                     this, &ServerWindow::tableController);
    QObject::connect(server, &Server::controllerUpdated,
                     this, &ServerWindow::updateController);
    QObject::connect(server, &Server::controllerTimeDiffSent,
                     this, &ServerWindow::onControllerTimeDiffSent);
    QObject::connect(server, &Server::workstationConnected,
                     this, &ServerWindow::tableWorkstation);
    QObject::connect(server, &Server::workstationAnswerSent,
                     this, &ServerWindow::onWorkstationAnswerSent);
    QObject::connect(server, &Server::socketClosed,
                     this, &ServerWindow::onSocketClosed);
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
    delete sendLogger;
    delete recieveLogger;
    delete systemLogger;
    delete ui;
}

void ServerWindow::startListening()
{
    try {
       server->setBufferSize(static_cast<size_t>(ui->bufferSize->value()));
       server->start(ui->portInput->text().toStdString());

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
        server->stop();

        ui->stopButton->hide();
        ui->startButton->show();
        ui->startedLabel->hide();
        ui->stoppedLabel->show();
        ui->portInput->setReadOnly(false);

        ui->startButton->setFocus();

        systemLogger->write("Server was stopped.");
    } catch (const QString &msg) {
        systemLogger->write(msg);
    }
}

void ServerWindow::tableController(SOCKET client) noexcept
{
    vector<QString> columns;
    auto info = server->getController(client);
    columns.push_back(QString::fromStdString(info.ip));
    columns.push_back(QString("%1").arg(client));
    columns.push_back(QString("КОМ %1").arg(info.number));
    columns.push_back(QString::fromStdString(info.formatDiffTime()));
    columns.push_back(QString("%1").arg(info.recievedData));
    columns.push_back(QString("%2").arg(info.savedData));
    columns.push_back("-");
    tableClient(columns);
}

void ServerWindow::updateController(SOCKET client) noexcept
{
    auto info = server->getController(client);
    int row = findClientTableRow(client);
    ui->clientsTable->item(row, 4)->setText(QString::fromStdString(info.formatDiffTime()));
    ui->clientsTable->item(row, 5)->setText(QString("%1").arg(info.recievedData));
    ui->clientsTable->item(row, 6)->setText(QString("%1").arg(info.savedData));
}

void ServerWindow::tableWorkstation(SOCKET client) noexcept
{
    auto sentData = server->getWorkstationSentData(client);
    vector<QString> columns;
    columns.push_back(QString::fromStdString(socket->getClientIp(client)));
    columns.push_back(QString("%1").arg(client));
    columns.push_back("PC");
    columns.push_back("-");
    columns.push_back("-");
    columns.push_back("-");
    columns.push_back(QString("%1").arg(sentData));
    tableClient(columns);
}

void ServerWindow::updateWorkstation(SOCKET client) noexcept
{
    int row = findClientTableRow(client);
    size_t sentData = server->getWorkstationSentData(client);
    ui->clientsTable->item(row, 7)->setText(QString("%1").arg(sentData));
}

void ServerWindow::tableClient(const vector<QString> &columns) noexcept
{
    int row = ui->clientsTable->rowCount();
    ui->clientsTable->insertRow(row);
    ui->clientsTable->setItem(row, 0, new QTableWidgetItem(QString("%1").arg(row + 1)));
    for (size_t i = 0; i < columns.size(); ++i) {
        int col = static_cast<int>(i + 1);
        ui->clientsTable->setItem(row, col, new QTableWidgetItem(columns[i]));
    }
    updateClientCount();
}

void ServerWindow::untableClient(SOCKET client) noexcept
{
    bool removed = false;
    for (int row = 0; row < ui->clientsTable->rowCount(); ++row) {
        if (removed) {
            ui->clientsTable->item(row, 0)->setText(QString("%1").arg(row + 1));
            continue;
        }
        if (ui->clientsTable->item(row, 2)->text().toUInt() == client) {
            ui->clientsTable->removeRow(row);
            removed = true;
            row -= 1;
        };
    }

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

void ServerWindow::onBufferSizeChanged(int size) noexcept
{
    server->setBufferSize(static_cast<size_t>(size));
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
    recieveLogger->write(
        QString("%1 bytes recieved from %2 socket.").arg(bytes).arg(from));
}

void ServerWindow::onControllerTimeDiffSent(SOCKET client, int bytes) noexcept
{
    sendLogger->write(QString("Time difference was sent to %1 socket "
                               "controller in %2 bytes.").arg(client).arg(bytes));
}

void ServerWindow::onWorkstationAnswerSent(SOCKET client, int bytes) noexcept
{
    updateWorkstation(client);
    sendLogger->write(QString("There was %1 bytes sent to workstation of %2 socket.")
                        .arg(bytes).arg(client));
}
