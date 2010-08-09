//!
//! @file   HopsanCoreMessageHandler.h
//! @author <peter.nordin@liu.se>
//! @date   2010-03-03
//!
//! @brief Contains the Classes for hopsancore -> main program message exchange
//!
//$Id$

#ifndef HOPSANCOREMESSAGEHANDLER_H
#define HOPSANCOREMESSAGEHANDLER_H

#include <queue>
#include <string>
#include "../win32dll.h"


class DLLIMPORTEXPORT HopsanCoreMessage
{
public:
    enum MessageTypes {INFO, WARNING, ERROR};
    int type;
    int debuglevel;
    std::string message;
};

class DLLIMPORTEXPORT HopsanCoreMessageHandler
{
private:
    std::queue<HopsanCoreMessage> mMessageQueue;
    void addMessage(int type, std::string preFix, std::string message, int debuglevel=0);
    size_t mMaxQueueSize;

public:
    HopsanCoreMessageHandler();
    void addInfoMessage(std::string message, int dbglevel=0);
    void addWarningMessage(std::string message, int dbglevel=0);
    void addErrorMessage(std::string message, int dbglevel=0);

    //const HopsanCoreMessage &peakMessage();
    HopsanCoreMessage getMessage();
    size_t nWaitingMessages();

};

extern HopsanCoreMessageHandler gCoreMessageHandler;
DLLIMPORTEXPORT HopsanCoreMessageHandler* getCoreMessageHandlerPtr();

#endif // HOPSANCOREMESSAGEHANDLER_H
