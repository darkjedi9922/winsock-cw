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

    lastDataTime[ControllerInfo::TYPE_1] = 0;
    lastDataTime[ControllerInfo::TYPE_2] = 0;

    auto eventManager = socket->getEventManager();
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &Server::onDataRecieved);
    QObject::connect(eventManager, &SocketEventManager::socketClosed,
                     this, &Server::onClientClosed);
}

Server::~Server()
{
    saveBuffer();
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
    auto workstations = this->workstations;
    for (auto &item : workstations) onClientClosed(item.first);
}

const ControllerInfo& Server::getController(SOCKET socket) const
{
    return controllers.at(socket);
}

size_t Server::getWorkstationSentData(SOCKET socket) const
{
    return workstations.at(socket);
}

size_t Server::getBufferSize() const
{
    return bufferSize;
}

void Server::setBufferSize(size_t size) noexcept
{
    if (size < bufferSize) saveBuffer();
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
        auto type = ControllerInfo::typeFromNumber(msg->controllerNumber);
        execDbQuery(QString("INSERT INTO data ("
                            "   type, time, speed1, speed2, "
                            "   temp1, temp2, mass, length"
                            ") VALUES ("
                            "   %1, %2, %3, %4,"
                            "   %5, %6, %7, %8"
                            ");")
                    .arg(static_cast<unsigned short>(type))
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
            handleControllerHello(from, reinterpret_cast<ControllerInfoMessage*>(msg));
            break;
        case Message::CONTROLLER_DATA:
            handleData(from, reinterpret_cast<ControllerDataMessage*>(msg));
            break;
        case Message::WORKSTATION_HELLO:
            handleWorkstationHello(from, msg);
            break;
        case Message::WORKSTATION_REQUEST:
            handleRequest(from, reinterpret_cast<WorkstationRequest*>(msg));
            break;
        default:
            return;
        }
    } catch (const QString &msg) {
        emit errorRaised(msg);
    }
}

void Server::onClientClosed(SOCKET socket) noexcept
{
    controllers.erase(socket);
    workstations.erase(socket);
    emit socketClosed(socket);
}

void Server::handleWorkstationHello(SOCKET from, const Message *)
{
    workstations[from] = 0;
    emit workstationConnected(from);
}

void Server::handleControllerHello(SOCKET from, const ControllerInfoMessage *msg)
{
    ControllerInfo controller;
    controller.ip = socket->getClientIp(from);
    controller.number = msg->controllerNumber;
    controller.recievedData = 0;
    controller.savedData = 0;
    controller.timeDiff = static_cast<short>(msg->time - time(nullptr));
    controllers[from] = controller;
    emit controllerConnected(from);

    if (controller.timeDiff != 0) sendControllerTimeDiff(from);
}

void Server::handleData(SOCKET from, const ControllerDataMessage *msg)
{
    ControllerInfo controller = controllers[from];
    controller.timeDiff = static_cast<short>(msg->time - time(nullptr));
    controller.recievedData += 1;
    controllers[from] = controller;

    if (controller.timeDiff != 0) sendControllerTimeDiff(from);

    auto type = ControllerInfo::typeFromNumber(msg->controllerNumber);
    if (lastDataTime[type] == msg->time) return;
    lastDataTime[type] = msg->time;

    buffer.push_back({from, *msg});
    if (buffer.size() >= bufferSize) saveBuffer();
    else emit controllerUpdated(from);
}

void Server::handleRequest(SOCKET from, const WorkstationRequest *request)
{
    saveBuffer();

    QSqlQuery query;
    query.exec(QString("SELECT * FROM data WHERE time BETWEEN %1 AND %2")
               .arg(request->from).arg(request->to));

    if (db.lastError().isValid())
        throw QString("Select query error: " + db.lastError().text());

    WorkstationAnswer answer;
    answer.finish = false;
    int bytes = 0;

    while (query.next()) {
        answer.dataType = static_cast<ControllerInfo::Type>(query.value("type").toInt());
        answer.data.time = static_cast<unsigned long>(query.value("time").toULongLong());
        answer.data.speed1 = static_cast<unsigned char>(query.value("speed1").toUInt());
        answer.data.speed2 = static_cast<unsigned char>(query.value("speed2").toUInt());
        answer.data.temp1 = static_cast<unsigned short>(query.value("temp1").toUInt());
        answer.data.temp2 = static_cast<unsigned short>(query.value("temp2").toUInt());
        answer.data.mass = static_cast<unsigned short>(query.value("mass").toUInt());
        answer.data.length = static_cast<unsigned char>(query.value("length").toUInt());
        answer.time = static_cast<unsigned long>(time(nullptr));

        bytes += socket->send(from, reinterpret_cast<char*>(&answer), sizeof(WorkstationAnswer));
        workstations[from] += 1;
    }

    answer.finish = true;
    answer.time = static_cast<unsigned long>(time(nullptr));
    bytes += socket->send(from, reinterpret_cast<char*>(&answer), sizeof(WorkstationAnswer));
    emit workstationAnswerSent(from, bytes);
}

void Server::sendControllerTimeDiff(SOCKET socket)
{
    auto controller = controllers[socket];
    TimeDiffMessage message;
    message.timediff = controller.timeDiff;
    message.time = static_cast<unsigned long>(time(nullptr));
    int size = sizeof(TimeDiffMessage);
    int bytes = this->socket->send(socket, reinterpret_cast<char*>(&message), size);
    emit controllerTimeDiffSent(socket, bytes);
}
