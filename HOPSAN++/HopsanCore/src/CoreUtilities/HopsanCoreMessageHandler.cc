/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HopsanCoreMessageHandler.cc
//! @author <peter.nordin@liu.se>
//! @date   2010-03-03
//!
//! @brief Contains the Classes for hopsancore -> main program message exchange
//!
//$Id$
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"

#include <cmath>
#include <cstdlib>

#ifdef USETBB
#include "tbb/mutex.h"
#endif

using namespace std;
using namespace hopsan;

HopsanCoreMessageHandler::HopsanCoreMessageHandler()
{
    mMaxQueueSize = 10000;
#ifdef USETBB
    mpMutex = new tbb::mutex;
#endif
}

HopsanCoreMessageHandler::~HopsanCoreMessageHandler()
{
    clear();
#ifdef USETBB
    delete mpMutex;
#endif
}

//! @brief Adds a message to the message queue
//! @param [in] type The message type identifier
//! @param [in] rPreFix A string to add before the message
//! @param [in] rMessage The message string
//! @param [in] rTag A tag describing the message
//! @param [in] debuglevel The debuglevel for the message
void HopsanCoreMessageHandler::addMessage(const int type, const HString &rPreFix, const HString &rMessage, const HString &rTag, const int debuglevel)
{
#ifdef USETBB
    mpMutex->lock();
#endif
    HopsanCoreMessage* pMsg = new HopsanCoreMessage;
    pMsg->mType = type;
    pMsg->mDebugLevel = debuglevel;
    pMsg->mMessage = rPreFix + rMessage;
    pMsg->mTag = rTag;
    mMessageQueue.push(pMsg);
    if (mMessageQueue.size() > mMaxQueueSize)
    {
        //If the queue is to long delete old unhandled messages
        delete mMessageQueue.front();
        mMessageQueue.pop();
    }
#ifdef USETBB
    mpMutex->unlock();
#endif
}

//! @brief Clears the message queue
void HopsanCoreMessageHandler::clear()
{
#ifdef USETBB
    mpMutex->lock();
#endif
    while(mMessageQueue.size() > 0)
    {
        // First delete the message itself
        delete mMessageQueue.front();
        // Now pop dangling pointer
        mMessageQueue.pop();
    }
#ifdef USETBB
    mpMutex->unlock();
#endif
}

//! @brief Convenience function to add info message
//! @param [in] rMessage The message string
//! @param [in] rTag A tag describing the message
//! @param [in] dbglevel The debuglevel for the message
void HopsanCoreMessageHandler::addInfoMessage(const HString &rMessage, const HString &rTag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::Info, "Info: ", rMessage, rTag, dbglevel);
}

//! @brief Convenience function to add warning message
//! @param [in] rMessage The message string
//! @param [in] rTag A tag describing the message
//! @param [in] dbglevel The debuglevel for the message
void HopsanCoreMessageHandler::addWarningMessage(const HString &rMessage, const HString &rTag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::Warning, "Warning: ", rMessage, rTag, dbglevel);
}

//! @brief Convenience function to add error message
//! @param [in] rMessage The message string
//! @param [in] rTag A tag describing the message
//! @param [in] dbglevel The debuglevel for the message
void HopsanCoreMessageHandler::addErrorMessage(const HString &rMessage, const HString &rTag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::Error, "Error: ", rMessage, rTag, dbglevel);
}

//! @brief Convenience function to add debug message
//! @param [in] rMessage The message string
//! @param [in] rTag A tag describing the message
//! @param [in] dbglevel The debuglevel for the message
void HopsanCoreMessageHandler::addDebugMessage(const HString &rMessage, const HString &rTag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::Debug, "Debug: ", rMessage, rTag, dbglevel);
}


//! @brief Convenience function to add fatal message. Also tells the receiver of the message to close program in a controlled way.
//! @param [in] rMessage The message string
//! @param [in] rTag A tag describing the message
//! @param [in] dbglevel The debuglevel for the message
void HopsanCoreMessageHandler::addFatalMessage(const HString &rMessage, const HString &rTag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::Fatal, "Fatal error: ", rMessage, rTag, dbglevel);
}


//! @brief Returns the next, (pops) message on the message queue
//! @param [out] rMessage The message string
//! @param [out] rType The message type(Info, Error, Warning...)
//! @param [out] rTag A tag describing the message
void HopsanCoreMessageHandler::getMessage(HString &rMessage, HString &rType, HString &rTag)
{
#ifdef USETBB
    mpMutex->lock();
#endif

    if (mMessageQueue.size() > 0)
    {
        rMessage = mMessageQueue.front()->mMessage;
        rTag = mMessageQueue.front()->mTag;

        switch (mMessageQueue.front()->mType)
        {
        case HopsanCoreMessage::Fatal:
            rType = "fatal";
            break;
        case HopsanCoreMessage::Error:
            rType = "error";
            break;
        case HopsanCoreMessage::Warning:
            rType = "warning";
            break;
        case HopsanCoreMessage::Info:
            rType = "info";
            break;
        case HopsanCoreMessage::Debug:
            rType = "debug";
            break;
        }

        delete mMessageQueue.front();
        mMessageQueue.pop();
    }
    else
    {
        rMessage = "Error: You requested a message even though the message queue is empty";
        rTag = "";
        rType = "error";
    }

#ifdef USETBB
    mpMutex->unlock();
#endif
}

//! @brief Returns the number of waiting messages on the message queue
size_t HopsanCoreMessageHandler::getNumWaitingMessages() const
{
#ifdef USETBB
    mpMutex->lock();
    size_t num = mMessageQueue.size();
    mpMutex->unlock();
    return num;
#else
    return mMessageQueue.size();
#endif
}
