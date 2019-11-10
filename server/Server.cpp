#include "Server.h"

#include <QDebug>

using namespace std;

Server::Server(ServerSocket *socket) :
    QObject(),
    socket(socket)
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

const ControllerInfo& Server::getController(SOCKET socket) const
{
    return controllers.at(socket);
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

void Server::onDataRecieved(SOCKET from, char *buffer, int) noexcept
{
    Message* msg = reinterpret_cast<Message*>(buffer);
    switch (msg->type)
    {
    case Message::CONTROLLER_HELLO: {
        handleHello(from, reinterpret_cast<ControllerInfoMessage*>(msg));
        break;
    }
    case Message::CONTROLLER_DATA:
        handleData(from, reinterpret_cast<ControllerDataMessage*>(msg));
        break;
    }
}

void Server::onClientClosed(SOCKET socket) noexcept
{
    controllers.erase(socket);
}

void Server::handleHello(SOCKET from, const ControllerInfoMessage *msg) noexcept
{
    ControllerInfo controller;
    controller.ip = socket->getClientIp(from);
    controller.number = msg->controllerNumber;
    controller.recievedData = 0;
    controller.savedData = 0;
    controller.diffTime = msg->time - time(nullptr);
    controllers[from] = controller;
    emit controllerConnected(from);
}

void Server::handleData(SOCKET from, const ControllerDataMessage *msg) noexcept
{
    try {
        ControllerInfo controller = controllers[from];
        controller.diffTime = msg->time - time(nullptr);

        execDbQuery(QString("INSERT INTO data ("
                            "   type, time, speed1, speed2, "
                            "   temp1, temp2, mass, length"
                            ") VALUES ("
                            "   %1, %2, %3, %4,"
                            "   %5, %6, %7, %8"
                            ");")
                    .arg(controller.type()).arg(msg->time)
                    .arg(msg->speed1).arg(msg->speed2)
                    .arg(msg->temp1).arg(msg->temp2)
                    .arg(msg->mass).arg(msg->length)
        );

        controller.recievedData += 1;
        controller.savedData += 1;

        controllers[from] = controller;
        emit controllerUpdated(from);
    }
    catch (const out_of_range &e) {
        emit errorRaised(QString("Out of Range error: %1").arg(e.what()));
    }
}
