#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <sstream>
#include <iomanip>

struct ControllerInfo
{
    enum Type : unsigned short { TYPE_1 = 1, TYPE_2 = 2 };

    static Type typeFromNumber(short number)
    {
        return number < 3 ? TYPE_1 : TYPE_2;
    }

    short number;
    std::string ip;
    short timeDiff;
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

#pragma pack(push, 1)
struct Message
{
    enum Type : unsigned char {
        CONTROLLER_HELLO,
        CONTROLLER_DATA,
        CONTROLLER_TIMEDIFF,
        WORKSTATION_HELLO,
        WORKSTATION_REQUEST,
        WORKSTATION_ANSWER
    };

    unsigned long time;
    unsigned char type;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ControllerInfoMessage : Message
{
    unsigned char controllerNumber;

    ControllerInfoMessage() {
        type = Message::CONTROLLER_HELLO;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ControllerDataMessage : ControllerInfoMessage
{
    unsigned char speed1, speed2;
    unsigned short temp1, temp2;
    unsigned short mass;
    unsigned char length;

    ControllerDataMessage() {
        type = Message::CONTROLLER_DATA;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TimeDiffMessage : Message
{
    short timediff;

    TimeDiffMessage() {
        type = Message::CONTROLLER_TIMEDIFF;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WorkstationRequest : Message
{
    unsigned long from;
    unsigned long to;

    WorkstationRequest() {
        type = Message::WORKSTATION_REQUEST;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WorkstationAnswer : Message
{
    ControllerDataMessage data;
    ControllerInfo::Type dataType;
    bool finish;

    char padding[10];

    WorkstationAnswer() {
        type = Message::WORKSTATION_ANSWER;
    }
};
#pragma pack(pop)

#endif // DEFINITIONS_H
