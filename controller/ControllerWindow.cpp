#include "ControllerWindow.h"
#include "ui_ControllerWindow.h"

ControllerWindow::ControllerWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControllerWindow),
    winsock(nullptr),
    client(nullptr)
{
    ui->setupUi(this);
    ui->disconnectButton->hide();
    ui->connectedLabel->hide();

    systemLogger = new Logger(ui->systemLog);

    try {
        winsock = new WinSock;
        client = new ClientSocket(winsock);
    } catch (const QString &msg) {
        systemLogger->write(msg);
        return;
    }

    auto checkConnectEnabling = [=] () {
        auto ipRegexp = QRegExp("^([0-9]{1,3}[.]){3}[0-9]{1,3}$");
        auto portRegexp = QRegExp("^[0-9]{1,5}$");
        bool ipOk = ui->ipInput->text().contains(ipRegexp);
        bool portOk = ui->portInput->text().contains(portRegexp);
        ui->connectButton->setEnabled(ipOk && portOk);
    };

    QObject::connect(ui->ipInput, &QLineEdit::textChanged, checkConnectEnabling);
    QObject::connect(ui->portInput, &QLineEdit::textChanged, checkConnectEnabling);
    QObject::connect(ui->connectButton, &QPushButton::clicked, [=] () {
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
        }
        catch (const QString &msg) {
            systemLogger->write(msg);
        }
    });
    QObject::connect(ui->disconnectButton, &QPushButton::clicked, [=] () {
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
    });
}

ControllerWindow::~ControllerWindow()
{
    if (client != nullptr) {
        client->close();
        delete client;
        client = nullptr;
    }
    delete winsock;
    delete systemLogger;
    delete ui;
}
