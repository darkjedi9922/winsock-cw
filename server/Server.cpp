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

void Server::onDataRecieved(SOCKET from, char *buffer, int bytes) noexcept
{
    Message* msg = reinterpret_cast<Message*>(buffer);
    switch (msg->type)
    {
    case Message::CONTROLLER_DATA:
        auto data = reinterpret_cast<ControllerData*>(msg);
        bool isNew = controllers.find(from) == controllers.end();
        ControllerInfo info;

        // This is the first message from this controller.
        if (isNew) {
            info.number = data->number;
            info.ip = socket->getClientIp(from);
            info.savedBytes = 0;
            info.recievedBytes = 0;
        } else {
            info = controllers[from];
        }

        info.diffTime = time(nullptr) - data->time;

        if (info.type() == ControllerInfo::TYPE_1) {
            auto typedData = reinterpret_cast<ControllerType1Data*>(data);
            execDbQuery(QString("INSERT INTO data ("
                                "   type, time, speed1, speed2, "
                                "   temp1, temp2, mass"
                                ") VALUES ("
                                "   %1, %2, %3, %4,"
                                "   %5, %6, %7"
                                ");")
                        .arg(1).arg(data->time)
                        .arg(typedData->speed1)
                        .arg(typedData->speed2)
                        .arg(typedData->temp1)
                        .arg(typedData->temp2)
                        .arg(typedData->mass)
            );
        } else {
            auto typedData = reinterpret_cast<ControllerType2Data*>(data);
            execDbQuery(QString("INSERT INTO data ("
                                "   type, time, speed1, "
                                "   temp1, temp2, mass, length"
                                ") VALUES ("
                                "   %1, %2, %3,"
                                "   %4, %5, %6, %7"
                                ");")
                        .arg(1).arg(data->time)
                        .arg(typedData->speed1)
                        .arg(typedData->temp1)
                        .arg(typedData->temp2)
                        .arg(typedData->mass)
                        .arg(typedData->length)
            );
        }

        info.recievedBytes += static_cast<unsigned>(bytes);
        info.savedBytes += static_cast<unsigned>(bytes);

        controllers[from] = info;
        if (isNew) emit controllerConnected(from);
        else emit controllerUpdated(from);
        break;
    }
}

void Server::onClientClosed(SOCKET socket) noexcept
{
    controllers.erase(socket);
}
