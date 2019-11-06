#include "WinSock.h"
#include <WinSock2.h>
#include <QString>

WinSock::WinSock()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) throw QString("WSAStartup failed with error: %1").arg(result);
}

WinSock::~WinSock() noexcept
{
    WSACleanup();
}
