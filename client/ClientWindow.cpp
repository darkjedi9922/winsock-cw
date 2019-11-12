#include "ClientWindow.h"
#include "ui_ClientWindow.h"
#include <vector>

using namespace std;

ClientWindow::ClientWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientWindow),
    winsock(nullptr),
    socket(nullptr)
{
    ui->setupUi(this);
    ui->type1Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->type2Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->disconnectButton->hide();
    ui->connectedLabel->hide();
    ui->proccessingLabel->hide();
    ui->recievedLabel->hide();

    systemLogger = new Logger(ui->systemLog);

    try {
        winsock = new WinSock;
        socket = new ClientSocket(winsock);
    } catch (const QString &msg) {
        systemLogger->write(msg);
        return;
    }

    QObject::connect(ui->ipInput, SIGNAL(textChanged(const QString &)),
                     this, SLOT(checkConnectPossibility()));
    QObject::connect(ui->portInput, SIGNAL(textChanged(const QString &)),
                     this, SLOT(checkConnectPossibility()));
    QObject::connect(ui->connectButton, SIGNAL(clicked()),
                     this, SLOT(connect()));
    QObject::connect(ui->disconnectButton, SIGNAL(clicked()),
                     this, SLOT(disconnect()));
    QObject::connect(ui->recieveButton, SIGNAL(clicked()),
                     this, SLOT(requestData()));

    auto eventManager = socket->getEventManager();
    QObject::connect(eventManager, SIGNAL(errorRaised(const QString &)),
                     systemLogger, SLOT(write(const QString &)));
    QObject::connect(eventManager, &SocketEventManager::socketClosed,
                     this, &ClientWindow::disconnect);
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &ClientWindow::onDataRecieved);
}

ClientWindow::~ClientWindow()
{
    if (socket != nullptr) {
        delete socket;
        socket = nullptr;
    }
    if (winsock != nullptr) {
        delete winsock;
        winsock = nullptr;
    }
    delete systemLogger;
    delete ui;
}

void ClientWindow::sendHello()
{
    Message msg;
    msg.type = Message::WORKSTATION_HELLO;
    msg.time = time(nullptr);
    try {
        int bytes = socket->send(reinterpret_cast<char*>(&msg), sizeof(Message));
        onDataSent(bytes);
    } catch (const QString &msg) {
        systemLogger->write(msg);
    }
}

void ClientWindow::addRecord(const WorkstationAnswer *answer) noexcept
{
    QTableWidget *table;
    vector<QString> columns;

    QDateTime datetime = QDateTime::fromSecsSinceEpoch(answer->data.time);
    auto time = datetime.toString("dd.MM.yyyy hh:mm::ss");
    columns.push_back(time);

    if (answer->dataType == ControllerInfo::TYPE_1) {
        table = ui->type1Table;
        columns.push_back(QString("%1").arg(answer->data.speed1));
        columns.push_back(QString("%1").arg(answer->data.speed2));
        columns.push_back(QString("%1").arg(answer->data.temp1));
        columns.push_back(QString("%1").arg(answer->data.temp2));
        columns.push_back(QString("%1").arg(answer->data.mass));
    } else {
        table = ui->type2Table;
        columns.push_back(QString("%1").arg(answer->data.speed1));
        columns.push_back(QString("%1").arg(answer->data.temp1));
        columns.push_back(QString("%1").arg(answer->data.temp2));
        columns.push_back(QString("%1").arg(answer->data.mass));
        columns.push_back(QString("%1").arg(answer->data.length));
    }

    int row = table->rowCount();
    table->insertRow(row);

    for (size_t i = 0; i < columns.size(); ++i) {
        int col = static_cast<int>(i);
        table->setItem(row, col, new QTableWidgetItem(columns[i]));
    }
}

void ClientWindow::checkConnectPossibility() noexcept
{
    auto ipRegexp = QRegExp("^([0-9]{1,3}[.]){3}[0-9]{1,3}$");
    auto portRegexp = QRegExp("^[0-9]{1,5}$");
    bool ipOk = ui->ipInput->text().contains(ipRegexp);
    bool portOk = ui->portInput->text().contains(portRegexp);
    ui->connectButton->setEnabled(ipOk && portOk);
}

void ClientWindow::connect() noexcept
{
    try {
        auto ip = ui->ipInput->text().toStdString();
        auto port = ui->portInput->text().toStdString();
        socket->connect(ip, port);
        sendHello();

        systemLogger->write("The workstation was connected.");
        ui->connectButton->hide();
        ui->disconnectButton->show();
        ui->disconnectedLabel->hide();
        ui->connectedLabel->show();
        ui->ipInput->setEnabled(false);
        ui->portInput->setEnabled(false);

        ui->fromDate->setEnabled(true);
        ui->fromTime->setEnabled(true);
        ui->toDate->setEnabled(true);
        ui->toTime->setEnabled(true);
        ui->recieveButton->setEnabled(true);
    }
    catch (const QString &msg) {
        systemLogger->write(msg);
    }
}

void ClientWindow::disconnect() noexcept
{
    socket->close();

    systemLogger->write("The work station was disconnected.");

    ui->disconnectButton->hide();
    ui->connectButton->show();
    ui->connectedLabel->hide();
    ui->disconnectedLabel->show();
    ui->ipInput->setEnabled(true);
    ui->portInput->setEnabled(true);

    ui->fromDate->setEnabled(false);
    ui->fromTime->setEnabled(false);
    ui->toDate->setEnabled(false);
    ui->toTime->setEnabled(false);
    ui->recieveButton->setEnabled(false);
}

void ClientWindow::requestData() noexcept
{
    WorkstationRequest request;
    QDateTime from(ui->fromDate->date(), ui->fromTime->time());
    QDateTime to(ui->toDate->date(), ui->toTime->time());
    request.from = from.toSecsSinceEpoch();
    request.to = to.toSecsSinceEpoch();
    request.time = time(nullptr);

    int bytes = socket->send(reinterpret_cast<char*>(&request), sizeof(WorkstationRequest));
    onDataSent(bytes);

    ui->recievedLabel->hide();
    ui->proccessingLabel->show();
    ui->type1Table->clearContents();
    ui->type1Table->setRowCount(0);
    ui->type2Table->clearContents();
    ui->type2Table->setRowCount(0);
}

void ClientWindow::onDataSent(int bytes) noexcept
{
    systemLogger->write(QString("%1 bytes was sent.").arg(bytes));
}

void ClientWindow::onDataRecieved(SOCKET, char *buffer, int bytes)
{
    systemLogger->write(QString("%1 bytes was recieved.").arg(bytes));

    auto msg = reinterpret_cast<Message*>(buffer);
    if (msg->type == Message::WORKSTATION_ANSWER) {
        while (bytes > 0) {
            auto answer = reinterpret_cast<WorkstationAnswer*>(buffer);
            bytes -= sizeof(WorkstationAnswer);
            buffer += sizeof(WorkstationAnswer);

            if (answer->finish) {
                ui->proccessingLabel->hide();
                ui->recievedLabel->show();
            } else addRecord(answer);
        }
    }
}
