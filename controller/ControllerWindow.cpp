#include "ControllerWindow.h"
#include "ui_ControllerWindow.h"

ControllerWindow::ControllerWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControllerWindow),
    winsock(nullptr),
    client(nullptr),
    controller(nullptr)
{
    ui->setupUi(this);
    ui->disconnectButton->hide();
    ui->connectedLabel->hide();
    ui->stopSendingButton->hide();
    ui->sendingLabel->hide();

    systemLogger = new Logger(ui->systemLog);

    try {
        winsock = new WinSock;
        client = new ClientSocket(winsock);
        controller = new Controller(client);
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
    QObject::connect(ui->startSendingButton, SIGNAL(clicked()),
                     this, SLOT(startSending()));
    QObject::connect(ui->stopSendingButton, SIGNAL(clicked()),
                     this, SLOT(stopSending()));

    auto eventManager = client->getEventManager();
    QObject::connect(eventManager, &SocketEventManager::errorRaised,
                     this, &ControllerWindow::onSocketError);
    QObject::connect(eventManager, &SocketEventManager::socketClosed,
                     this, &ControllerWindow::onSocketClosed);
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &ControllerWindow::onDataRecieved);

    QObject::connect(controller, &Controller::errorRaised,
                     this, &ControllerWindow::onSocketError);
    QObject::connect(controller, &Controller::sent,
                     this, &ControllerWindow::onDataSent);
}

ControllerWindow::~ControllerWindow()
{
    if (controller != nullptr) {
        delete controller;
        controller = nullptr;
    }
    if (client != nullptr) {
        client->close();
        delete client;
        client = nullptr;
    }
    delete winsock;
    delete systemLogger;
    delete ui;
}

void ControllerWindow::checkConnectPossibility() noexcept
{
    auto ipRegexp = QRegExp("^([0-9]{1,3}[.]){3}[0-9]{1,3}$");
    auto portRegexp = QRegExp("^[0-9]{1,5}$");
    bool ipOk = ui->ipInput->text().contains(ipRegexp);
    bool portOk = ui->portInput->text().contains(portRegexp);
    ui->connectButton->setEnabled(ipOk && portOk);
}

void ControllerWindow::connect() noexcept
{
    try {
        int number = ui->clientSpinBox->value();
        auto ip = ui->ipInput->text().toStdString();
        auto port = ui->portInput->text().toStdString();
        client->connect(ip, port);

        systemLogger->write(QString("Controller number %1 was connected.").arg(number));
        ui->connectButton->hide();
        ui->disconnectButton->show();
        ui->disconnectedLabel->hide();
        ui->connectedLabel->show();
        ui->ipInput->setEnabled(false);
        ui->portInput->setEnabled(false);
        ui->clientSpinBox->setEnabled(false);
        ui->sendInterval->setEnabled(true);
        ui->startSendingButton->setEnabled(true);
    }
    catch (const QString &msg) {
        systemLogger->write(msg);
    }
}

void ControllerWindow::disconnect() noexcept
{
    if (ui->sendingLabel->isVisible()) stopSending();

    client->close();

    int number = ui->clientSpinBox->value();
    systemLogger->write(QString("Controller number %1 was disconnected.").arg(number));

    ui->disconnectButton->hide();
    ui->connectButton->show();
    ui->connectedLabel->hide();
    ui->disconnectedLabel->show();
    ui->ipInput->setEnabled(true);
    ui->portInput->setEnabled(true);
    ui->clientSpinBox->setEnabled(true);
    ui->sendInterval->setEnabled(false);
    ui->startSendingButton->setEnabled(false);
}

void ControllerWindow::startSending() noexcept
{
    controller->startSending(ui->sendInterval->value() * 1000);
    ui->startSendingButton->hide();
    ui->stopSendingButton->show();
    ui->sendingLabel->show();
    ui->sendInterval->setEnabled(false);
    systemLogger->write("Data sending was started.");
}

void ControllerWindow::stopSending() noexcept
{
    controller->stopSending();
    ui->sendingLabel->hide();
    ui->stopSendingButton->hide();
    ui->startSendingButton->show();
    ui->sendInterval->setEnabled(true);
    systemLogger->write("Data sending was stopped");
}

void ControllerWindow::onSocketError(const QString &msg) noexcept
{
    systemLogger->write(msg);
}

void ControllerWindow::onSocketClosed() noexcept
{
    disconnect();
}

void ControllerWindow::onDataRecieved(SOCKET, char *, int bytes) noexcept
{
    systemLogger->write(QString("%1 bytes recieved from the server.").arg(bytes));
}

void ControllerWindow::onDataSent(int bytes) noexcept
{
    int sentStructures = ui->sentStructures->text().toInt() + 1;
    ui->sentStructures->setText(QString("%1").arg(sentStructures));
    systemLogger->write(QString("%1 bytes was sent.").arg(bytes));
}
