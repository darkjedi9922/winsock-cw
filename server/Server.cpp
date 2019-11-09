#include "Server.h"

#include <QDebug>

using namespace std;

Server::Server(ServerSocket *socket) noexcept :
    QObject(),
    socket(socket)
{
    auto eventManager = socket->getEventManager();
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &Server::onDataRecieved);
    QObject::connect(eventManager, &SocketEventManager::socketClosed,
                     this, &Server::onClientClosed);
}

const ControllerInfo& Server::getController(SOCKET socket) const
{
    return controllers.at(socket);
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
        info.savedBytes += 0;
        info.recievedBytes += bytes;

        controllers[from] = info;
        if (isNew) emit controllerConnected(from);
        break;
    }
}

void Server::onClientClosed(SOCKET socket) noexcept
{
    controllers.erase(socket);
}
