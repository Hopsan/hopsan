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
#include "win32dll.h"

using namespace std;

DLLIMPORTEXPORT class HopsanCoreMessage
{
public:
    enum MessageTypes {INFO, WARNING, ERROR};
    int type;
    int debuglevel;
    string message;
};

DLLIMPORTEXPORT class HopsanCoreMessageHandler
{
private:
    queue<HopsanCoreMessage> mMessageQueue;
    void addMessage(int type, string preFix, string message, int debuglevel=0);

public:
    HopsanCoreMessageHandler();
    void addInfoMessage(string message, int dbglevel=0);
    void addWarningMessage(string message, int dbglevel=0);
    void addErrorMessage(string message, int dbglevel=0);

    //const HopsanCoreMessage &peakMessage();
    HopsanCoreMessage getMessage();
    size_t nWaitingMessages();

};

extern HopsanCoreMessageHandler gHopsanCoreMessageHandler;
DLLIMPORTEXPORT HopsanCoreMessageHandler* getCoreMessageHandlerPtr();

#endif // HOPSANCOREMESSAGEHANDLER_H
