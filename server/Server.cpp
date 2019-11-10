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
    case Message::CONTROLLER_DATA:
        auto data = reinterpret_cast<ControllerDataMessage*>(msg);
        bool isNew = controllers.find(from) == controllers.end();
        ControllerInfo controller;

        // This is the first message from this controller.
        if (isNew) {
            controller.ip = socket->getClientIp(from);
            controller.number = data->controllerNumber;
            controller.savedData = 0;
            controller.recievedData = 0;
        } else {
            controller = controllers[from];
        }

        controller.diffTime = time(nullptr) - data->time;

        execDbQuery(QString("INSERT INTO data ("
                            "   type, time, speed1, speed2, "
                            "   temp1, temp2, mass, length"
                            ") VALUES ("
                            "   %1, %2, %3, %4,"
                            "   %5, %6, %7, %8"
                            ");")
                    .arg(controller.type()).arg(data->time)
                    .arg(data->speed1).arg(data->speed2)
                    .arg(data->temp1).arg(data->temp2)
                    .arg(data->mass).arg(data->length)
        );

        controller.recievedData += 1;
        controller.savedData += 1;

        controllers[from] = controller;
        if (isNew) emit controllerConnected(from);
        else emit controllerUpdated(from);
        break;
    }
}

void Server::onClientClosed(SOCKET socket) noexcept
{
    controllers.erase(socket);
}
