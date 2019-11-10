#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <sstream>
#include <iomanip>

struct ControllerInfo
{
    enum Type { TYPE_1 = 1, TYPE_2 = 2, UNKNOWN = -1 };

    static Type typeFromNumber(short number)
    {
        return number % 2 == 0 ? TYPE_2 : TYPE_1;
    }

    short number;
    std::string ip;
    time_t timeDiff;
    unsigned long recievedData;
    unsigned long savedData;

    int type()
    {
        switch (number) {
        case 1:
        case 2:
            return TYPE_1;
        case 3:
        case 4:
            return TYPE_2;
        default:
            return UNKNOWN;
        }
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
        TIMEDIFF,
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

struct ControllerDataMessage : Message
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
        type = Message::TIMEDIFF;
    }
};

#endif // DEFINITIONS_H
