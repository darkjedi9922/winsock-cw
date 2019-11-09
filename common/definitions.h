#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <sstream>
#include <iomanip>

struct ControllerInfo
{
    enum Type { TYPE_1, TYPE_2, UNKNOWN };

    short number;
    std::string ip;
    time_t diffTime;
    unsigned long recievedBytes;
    unsigned long savedBytes;

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
        stream << (diffTime < 0 ? '-' : '+');
        stream << std::setw(2) << std::setfill('0') << abs(diffTime) / 3600 << ':';
        stream << std::setw(2) << std::setfill('0') << (abs(diffTime) / 60) % 60 << ':';
        stream << std::setw(2) << std::setfill('0') << abs(diffTime) % 60;
        return stream.str();
    }
};

struct Message
{
    enum Type { CONTROLLER_DATA };

    Type type;
    time_t time;
};

struct ControllerData : Message
{
    short number;

    ControllerData() {
        type = Message::CONTROLLER_DATA;
    }
};

struct ControllerType1Data : ControllerData
{
    unsigned speed1, speed2;
    unsigned temp1, temp2;
    unsigned mass;
};

struct ControllerType2Data : ControllerData
{
    unsigned speed1;
    unsigned temp1, temp2;
    unsigned mass;
    unsigned length;
};

#endif // DEFINITIONS_H
