#include "Controller.h"
#include "definitions.h"

#include <QDebug>

Controller::Controller(ClientSocket *socket) noexcept :
    QObject(),
    socket(socket),
    timerId(-1)
{}

void Controller::startSending(int msInterval) noexcept
{
    timerId = startTimer(msInterval);
}

void Controller::stopSending() noexcept
{
    killTimer(timerId);
    timerId = -1;
}

void Controller::timerEvent(QTimerEvent *)
{
    ControllerType1Data data = {};
    data.number = 1;
    data.speed1 = 100;
    data.speed2 = 200;
    data.temp1 = 50;
    data.temp2 = 42;
    data.mass = 44;
    data.time = time(nullptr);

    qDebug() << data.time;

    try {
        int sentBytes = socket->send(reinterpret_cast<char*>(&data),
                                     sizeof(ControllerType1Data));
        emit sent(sentBytes);
    }
    catch (const QString &msg) {
        emit errorRaised(msg);
    }
}
