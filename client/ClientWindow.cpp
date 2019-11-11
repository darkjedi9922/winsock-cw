#include "ClientWindow.h"
#include "ui_ClientWindow.h"

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

//    auto eventManager = socket->getEventManager();
//    QObject::connect(eventManager, &SocketEventManager::errorRaised,
//                     this, &ClientWindow::onSocketError);
//    QObject::connect(eventManager, &SocketEventManager::socketClosed,
//                     this, &ClientWindow::onSocketClosed);
//    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
//                     this, &ClientWindow::onDataRecieved);
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
//        sendHello();

        systemLogger->write("The workstation was connected.");
        ui->connectButton->hide();
        ui->disconnectButton->show();
        ui->disconnectedLabel->hide();
        ui->connectedLabel->show();
        ui->ipInput->setEnabled(false);
        ui->portInput->setEnabled(false);
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
}
