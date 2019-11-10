#include "Server.h"

#include <QDebug>

using namespace std;

Server::Server(ServerSocket *socket) :
    QObject(),
    socket(socket),
    bufferSize(0)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("db.sqlite");
    if (!db.open()) throw db.lastError().text();
    if (db.tables().size() == 0) createDb();

    auto eventManager = socket->getEventManager();
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &Server::onDataRecieved);
    QObject::connect(eventManager, &SocketEventManager::socketClosed,
                     this, &Server::onClientClosed);
}

Server::~Server()
{
    db.close();
}

void Server::start(string port)
{
    socket->listen(port);
}

void Server::stop()
{
    socket->close();
    saveBuffer();
    auto controllers = this->controllers;
    for (auto &item : controllers) onClientClosed(item.first);
}

const ControllerInfo& Server::getController(SOCKET socket) const
{
    return controllers.at(socket);
}

size_t Server::getBufferSize() const
{
    return bufferSize;
}

void Server::setBufferSize(size_t size) noexcept
{
    bufferSize = size;
}

void Server::execDbQuery(const QString &query)
{
    db.exec(query);
    if (db.lastError().isValid()) throw QString("Query: " + query + "\n" +
                                                "Error: " + db.lastError().text());
}

void Server::createDb()
{
    execDbQuery("CREATE TABLE data ("
                "   type TINYINT NOT NULL,"
                "   time UNSIGNED BIG INT NOT NULL,"
                "   speed1 INTEGER DEFAULT NULL,"
                "   speed2 INTEGER DEFAULT NULL,"
                "   temp1 INTEGER DEFAULT NULL,"
                "   temp2 INTEGER DEFAULT NULL,"
                "   mass INTEGER DEFAULT NULL,"
                "   length INTEGER DEFAULT NULL"
                ");");
}

void Server::saveBuffer()
{
    for (auto &item : buffer) {
        if (controllers.find(item.first) != controllers.end()) {
            controllers[item.first].savedData += 1;
            emit controllerUpdated(item.first);
        }

        auto *msg = &item.second;
        execDbQuery(QString("INSERT INTO data ("
                            "   type, time, speed1, speed2, "
                            "   temp1, temp2, mass, length"
                            ") VALUES ("
                            "   %1, %2, %3, %4,"
                            "   %5, %6, %7, %8"
                            ");")
                    .arg(ControllerInfo::typeFromNumber(msg->controllerNumber))
                    .arg(msg->time)
                    .arg(msg->speed1).arg(msg->speed2)
                    .arg(msg->temp1).arg(msg->temp2)
                    .arg(msg->mass).arg(msg->length)
        );
    }

    buffer.clear();
}

void Server::onDataRecieved(SOCKET from, char *buffer, int) noexcept
{
    Message* msg = reinterpret_cast<Message*>(buffer);
    try {
        switch (msg->type)
        {
        case Message::CONTROLLER_HELLO:
            handleHello(from, reinterpret_cast<ControllerInfoMessage*>(msg));
            break;
        case Message::CONTROLLER_DATA:
            handleData(from, reinterpret_cast<ControllerDataMessage*>(msg));
            break;
        }
    } catch (const QString &msg) {
        emit errorRaised(msg);
    }
}

void Server::onClientClosed(SOCKET socket) noexcept
{
    controllers.erase(socket);
}

void Server::handleHello(SOCKET from, const ControllerInfoMessage *msg)
{
    ControllerInfo controller;
    controller.ip = socket->getClientIp(from);
    controller.number = msg->controllerNumber;
    controller.recievedData = 0;
    controller.savedData = 0;
    controller.timeDiff = msg->time - time(nullptr);
    controllers[from] = controller;
    emit controllerConnected(from);

    if (controller.timeDiff != 0) sendControllerTimeDiff(from);
}

void Server::handleData(SOCKET from, const ControllerDataMessage *msg)
{
    ControllerInfo controller = controllers[from];
    controller.timeDiff = msg->time - time(nullptr);
    controller.recievedData += 1;
    controllers[from] = controller;

    buffer.push_back({from, *msg});
    if (buffer.size() >= bufferSize) saveBuffer();
    else emit controllerUpdated(from);

    if (controller.timeDiff != 0) sendControllerTimeDiff(from);
}

void Server::sendControllerTimeDiff(SOCKET socket)
{
    auto controller = controllers[socket];
    TimeDiffMessage message;
    message.timediff = controller.timeDiff;
    message.time = time(nullptr);
    int size = sizeof(TimeDiffMessage);
    int bytes = this->socket->send(socket, reinterpret_cast<char*>(&message), size);
    emit controllerTimeDiffSent(socket, bytes);
}
