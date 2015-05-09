#ifndef SERVERSTATUSMESSAGE_H
#define SERVERSTATUSMESSAGE_H

#include<string>

typedef struct
{
    int numFreeSlots;
    int numTotalSlots;
    std::string startTime;
    std::string stopTime;
    bool isReady;
}ServerStatusT;

#endif // SERVERSTATUSMESSAGE_H
