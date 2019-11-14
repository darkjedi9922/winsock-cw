#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <sstream>
#include <iomanip>

struct ControllerInfo
{
    enum Type { TYPE_1, TYPE_2 };

    static Type typeFromNumber(short number)
    {
        return number % 2 == 0 ? TYPE_2 : TYPE_1;
    }

    short number;
    std::string ip;
    time_t timeDiff;
    unsigned recievedData;
    unsigned savedData;

    Type type()
    {
        return ControllerInfo::typeFromNumber(number);
    }

    std::string formatDiffTime()
    {
        std::stringstream stream;
        stream << (timeDiff < 0 ? '-' : '+');
        stream << std::setw(2) << std::setfill('0') << abs(timeDiff) / 3600 << ':';
        stream << std::setw(2) << std::setfill('0') << (abs(timeDiff) / 60) % 60 << ':';
        stream << std::setw(2) << std::setfill('0') << abs(timeDiff) % 60;
        return stream.str();
    }
};

struct Message
{
    enum Type {
        CONTROLLER_HELLO,
        CONTROLLER_DATA,
        CONTROLLER_TIMEDIFF,
        WORKSTATION_HELLO,
        WORKSTATION_REQUEST,
        WORKSTATION_ANSWER
    };

    Type type;
    time_t time;
};

struct ControllerInfoMessage : Message
{
    short controllerNumber;

    ControllerInfoMessage() {
        type = Message::CONTROLLER_HELLO;
    }
};

struct ControllerDataMessage : ControllerInfoMessage
{
    unsigned short speed1, speed2;
    unsigned short temp1, temp2;
    unsigned short mass, length;

    ControllerDataMessage() {
        type = Message::CONTROLLER_DATA;
    }
};

struct TimeDiffMessage : Message
{
    time_t timediff;

    TimeDiffMessage() {
        type = Message::CONTROLLER_TIMEDIFF;
    }
};

struct WorkstationRequest : Message
{
    time_t from;
    time_t to;

    WorkstationRequest() {
        type = Message::WORKSTATION_REQUEST;
    }
};

struct WorkstationAnswer : Message
{
    ControllerDataMessage data;
    ControllerInfo::Type dataType;
    bool finish;

    WorkstationAnswer() {
        type = Message::WORKSTATION_ANSWER;
    }
};

#endif // DEFINITIONS_H
