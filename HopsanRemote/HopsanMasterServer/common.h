#ifndef COMMON_H
#define COMMON_H

#define PRINTSERVER "HopsanMasterServer; "

#include <ctime>
#include <chrono>
#include <string>

std::string nowDateTime()
{
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buff[64];
    std::strftime(buff, sizeof(buff), "%b %d %H:%M:%S", std::localtime(&now_time));
    return string(&buff[0]);
}

#endif // COMMON_H
