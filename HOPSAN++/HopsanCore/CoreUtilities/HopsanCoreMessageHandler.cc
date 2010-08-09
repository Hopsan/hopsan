//!
//! @file   HopsanCoreMessageHandler.cc
//! @author <peter.nordin@liu.se>
//! @date   2010-03-03
//!
//! @brief Contains the Classes for hopsancore -> main program message exchange
//!
//$Id$
#include "HopsanCoreMessageHandler.h"

using namespace std;
using namespace hopsan;

HopsanCoreMessageHandler::HopsanCoreMessageHandler()
{
    mMaxQueueSize = 20;
}

void HopsanCoreMessageHandler::addMessage(int type, string preFix, string message, int debuglevel)
{
    HopsanCoreMessage msg;
    msg.type = type;
    msg.debuglevel = debuglevel;
    msg.message = preFix + message;
    mMessageQueue.push(msg);
    if (mMessageQueue.size() > mMaxQueueSize)
    {
        //If the que is to long delete old unhandled messages
        mMessageQueue.pop();
    }
}


void HopsanCoreMessageHandler::addInfoMessage(string message, int dbglevel)
{
    addMessage(HopsanCoreMessage::INFO, "Info: ", message, dbglevel);
}

void HopsanCoreMessageHandler::addWarningMessage(string message, int dbglevel)
{
    addMessage(HopsanCoreMessage::WARNING, "Warning: ", message, dbglevel);
}

void HopsanCoreMessageHandler::addErrorMessage(string message, int dbglevel)
{
    addMessage(HopsanCoreMessage::ERROR, "Error: ", message, dbglevel);
}


HopsanCoreMessage HopsanCoreMessageHandler::getMessage()
{
    HopsanCoreMessage msg;
    if (mMessageQueue.size() > 0)
    {
        msg = mMessageQueue.front();
        mMessageQueue.pop();
        return msg;
    }
    else
    {
        msg.type = HopsanCoreMessage::ERROR;
        msg.debuglevel = 0;
        msg.message = "Error: You requested a message even though the message queue is empty";
        return msg;
    }
}

size_t HopsanCoreMessageHandler::nWaitingMessages()
{
    return mMessageQueue.size();
}

HopsanCoreMessageHandler hopsan::gCoreMessageHandler;
DLLIMPORTEXPORT HopsanCoreMessageHandler* hopsan::getCoreMessageHandlerPtr()
{
    return &gCoreMessageHandler;
}
