#include "Controller.h"

#include <QTimer>

Controller::Controller(ClientSocket *socket) noexcept :
    QObject(),
    socket(socket),
    number(0),
    timerId(-1),
    timediff(0)
{
    auto eventManager = socket->getEventManager();
    QObject::connect(eventManager, &SocketEventManager::dataRecieved,
                     this, &Controller::onDataRecieved);
}

short Controller::getNumber() const
{
    return number;
}

void Controller::setNumber(short number) noexcept
{
    this->number = number;
}

void Controller::startSending(int sInterval) noexcept
{
    timerId = startTimer(sInterval * 1000);
}

void Controller::stopSending() noexcept
{
    killTimer(timerId);
    timerId = -1;
}

void Controller::timerEvent(QTimerEvent *)
{
    generateAndSend();
}

void Controller::sendHello() noexcept
{
    ControllerInfoMessage data = {};
    data.controllerNumber = number;

    try {
        data.time = time(nullptr);
        int sentBytes = socket->send(reinterpret_cast<char*>(&data),
                                     sizeof(ControllerInfoMessage));
        emit sent(sentBytes);
    }
    catch (const QString &msg) {
        emit errorRaised(msg);
    }
}

void Controller::generateAndSend() noexcept
{
    srand(static_cast<unsigned int>(number));
    ControllerDataMessage data = {};

    if (ControllerInfo::typeFromNumber(number) == ControllerInfo::TYPE_1) {
        data.speed1 = 0 + rand() % ((30 + 1) - 0); // [0; 30]
        data.speed2 = 0 + rand() % ((20 + 1) - 0);
        data.temp1 = 0 + rand() % ((800 + 1) - 0);
        data.temp2 = 0 + rand() % ((1000 + 1) - 0);
        data.mass = 0 + rand() % ((500 + 1) - 0);
    } else {
        data.speed1 = 0 + rand() % ((20 + 1) - 0);
        data.temp1 = 0 + rand() % ((1000 + 1) - 0);
        data.temp2 = 0 + rand() % ((500 + 1) - 0);
        data.mass = 0 + rand() % ((1000 + 1) - 0);
        data.length = 0 + rand() % ((100 + 1) - 0);
    }

    data.time = time(nullptr) - timediff;

    try {
        int sentBytes = socket->send(reinterpret_cast<char*>(&data),
                                     sizeof(ControllerDataMessage));
        emit sent(sentBytes);
    }
    catch (const QString &msg) {
        emit errorRaised(msg);
    }
}

void Controller::onDataRecieved(SOCKET, char *buffer, int) noexcept
{
    auto message = reinterpret_cast<Message*>(buffer);
    if (message->type == Message::TIMEDIFF) {
        auto timediffMessage = reinterpret_cast<TimeDiffMessage*>(message);
        timediff += timediffMessage->timediff;
    }
}
