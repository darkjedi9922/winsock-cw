#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <WS2tcpip.h>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    listenSocket = INVALID_SOCKET;

    ui->setupUi(this);
    ui->clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->stopButton->hide();
    ui->startedLabel->hide();

    systemLogger = new Logger(ui->systemLog);

    initWinsock();

    QObject::connect(ui->portInput, &QLineEdit::textChanged, [=] (const QString &newText) {
        ui->startButton->setEnabled(newText.contains(QRegExp("^[0-9]{1,5}$")));
    });
    QObject::connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startServer()));
    QObject::connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopServer()));
}

MainWindow::~MainWindow()
{
    stopServer();
    cleanWinsock();

    delete systemLogger;
    delete ui;
}

void MainWindow::initWinsock()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        throw std::string("WSAStartup failed with error: ") + std::to_string(result);
    }
}

void MainWindow::startServer()
{
    std::string port = ui->portInput->text().toStdString();

    struct addrinfo *addressInfo = nullptr;
    struct addrinfo hints;
    int iResult;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(nullptr, port.c_str(), &hints, &addressInfo);
    if (iResult != 0) {
        systemLogger->write(QString("getaddrinfo failed with error: %1")
                            .arg(WSAGetLastError()));
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(addressInfo->ai_family,
                          addressInfo->ai_socktype,
                          addressInfo->ai_protocol);

    if (listenSocket == INVALID_SOCKET) {
        freeaddrinfo(addressInfo);
        systemLogger->write(QString("socket failed with error: %1")
                            .arg(WSAGetLastError()));
    }

    iResult = bind(listenSocket,
                   addressInfo->ai_addr,
                   static_cast<int>(addressInfo->ai_addrlen));

    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(addressInfo);
        closesocket(listenSocket);
        systemLogger->write(QString("bind failed with error: %1")
                            .arg(WSAGetLastError()));
    }

    freeaddrinfo(addressInfo);

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        closesocket(listenSocket);
        systemLogger->write(QString("listen failed with error: %1")
                            .arg(WSAGetLastError()));
    }

    ui->startButton->hide();
    ui->stopButton->show();
    ui->stoppedLabel->hide();
    ui->startedLabel->show();
    systemLogger->write("Server was successfully started.");
}

void MainWindow::stopServer()
{
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;

    ui->stopButton->hide();
    ui->startButton->show();
    ui->startedLabel->hide();
    ui->stoppedLabel->show();
    systemLogger->write("Server was stopped.");
}

void MainWindow::cleanWinsock()
{
    WSACleanup();
}
