/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include <iostream>
#if __cplusplus > 199711L
#include <mutex>
#endif

using namespace std;
using namespace hopsan;

HopsanCoreMessageHandler::HopsanCoreMessageHandler()
{
    mMaxQueueSize = 10000;
#if __cplusplus > 199711L
    mpMutex = new std::mutex;
#endif
    clear(); // Using clear here to init the message counters (init code in one place only)
}

HopsanCoreMessageHandler::~HopsanCoreMessageHandler()
{
    clear();
#if __cplusplus > 199711L
    delete mpMutex;
#endif
}

//! @brief Adds a message to the message queue
//! @param [in] type The message type identifier
//! @param [in] rPreFix A string to add before the message
//! @param [in] rMessage The message string
//! @param [in] rTag A tag describing the message
//! @param [in] debuglevel The debuglevel for the message
void HopsanCoreMessageHandler::addMessage(const HopsanCoreMessage::MessageEnumT type, const HString &rPreFix, const HString &rMessage, const HString &rTag, const int debuglevel)
{
#if __cplusplus > 199711L
    mpMutex->lock();
#endif
    HopsanCoreMessage* pMsg = new HopsanCoreMessage(type, rPreFix+rMessage, rTag, debuglevel);
    mMessageQueue.push_back(pMsg);
    switch (type)
    {
    case HopsanCoreMessage::Fatal:
        ++mNumFatalMessages;
        break;
    case HopsanCoreMessage::Error:
        ++mNumErrorMessages;
        break;
    case HopsanCoreMessage::Warning:
        ++mNumWarningMessages;
        break;
    case HopsanCoreMessage::Info:
        ++mNumInfoMessages;
        break;
    case HopsanCoreMessage::Debug:
        ++mNumDebugMessages;
        break;
    }


    // If the queue is to long delete old unhandled messages
    if (mMessageQueue.size() > mMaxQueueSize)
    {
        // Decrease message counters
        switch (mMessageQueue.front()->mType)
        {
        case HopsanCoreMessage::Fatal:
            --mNumFatalMessages;
            break;
        case HopsanCoreMessage::Error:
            --mNumErrorMessages;
            break;
        case HopsanCoreMessage::Warning:
            --mNumWarningMessages;
            break;
        case HopsanCoreMessage::Info:
            --mNumInfoMessages;
            break;
        case HopsanCoreMessage::Debug:
            --mNumDebugMessages;
            break;
        }

        // Delete the message and pop the message pointer
        delete mMessageQueue.front();
        mMessageQueue.pop_front();
    }
#if __cplusplus > 199711L
    mpMutex->unlock();
#endif
}

//! @brief Clears the message queue
void HopsanCoreMessageHandler::clear()
{
#if __cplusplus > 199711L
    mpMutex->lock();
#endif
    // Delete each message and pop each message pointer
    while(mMessageQueue.size() > 0)
    {
        delete mMessageQueue.front();
        mMessageQueue.pop_front();
    }
    // Reset counters
    mNumInfoMessages = 0;
    mNumWarningMessages = 0;
    mNumErrorMessages = 0;
    mNumFatalMessages = 0;
    mNumDebugMessages = 0;
#if __cplusplus > 199711L
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
#if __cplusplus > 199711L
    mpMutex->lock();
#endif

    if (mMessageQueue.size() > 0)
    {
        rMessage = mMessageQueue.front()->mMessage;
        rTag = mMessageQueue.front()->mTag;

        // Set type string and decrement message counters depending on message type
        switch (mMessageQueue.front()->mType)
        {
        case HopsanCoreMessage::Fatal:
            rType = "fatal";
            --mNumFatalMessages;
            break;
        case HopsanCoreMessage::Error:
            rType = "error";
            --mNumErrorMessages;
            break;
        case HopsanCoreMessage::Warning:
            rType = "warning";
            --mNumWarningMessages;
            break;
        case HopsanCoreMessage::Info:
            rType = "info";
            --mNumInfoMessages;
            break;
        case HopsanCoreMessage::Debug:
            rType = "debug";
            --mNumDebugMessages;
            break;
        }

        // Delete the message and pop the message pointer
        delete mMessageQueue.front();
        mMessageQueue.pop_front();
    }
    else
    {
        rMessage = "Error: You requested a message even though the message queue is empty";
        rTag = "";
        rType = "error";
    }

#if __cplusplus > 199711L
    mpMutex->unlock();
#endif
}

//! @brief Returns the number of waiting messages on the message queue
size_t HopsanCoreMessageHandler::getNumWaitingMessages() const
{
#if __cplusplus > 199711L
    mpMutex->lock();
    const size_t num = mMessageQueue.size();
    mpMutex->unlock();
    return num;
#else
    return mMessageQueue.size();
#endif
}

//! @brief Returns the number of waiting info messages on the message queue
size_t HopsanCoreMessageHandler::getNumInfoMessages() const
{
#if __cplusplus > 199711L
    mpMutex->lock();
    const size_t num = mNumInfoMessages;
    mpMutex->unlock();
    return num;
#else
    return mNumInfoMessages;
#endif
}

//! @brief Returns the number of waiting warning messages on the message queue
size_t HopsanCoreMessageHandler::getNumWarningMessages() const
{
#if __cplusplus > 199711L
    mpMutex->lock();
    const size_t num = mNumWarningMessages;
    mpMutex->unlock();
    return num;
#else
    return mNumWarningMessages;
#endif
}

//! @brief Returns the number of waiting error messages on the message queue
size_t HopsanCoreMessageHandler::getNumErrorMessages() const
{
#if __cplusplus > 199711L
    mpMutex->lock();
    const size_t num = mNumErrorMessages;
    mpMutex->unlock();
    return num;
#else
    return mNumErrorMessages;
#endif
}

//! @brief Returns the number of waiting debug messages on the message queue
size_t HopsanCoreMessageHandler::getNumDebugMessages() const
{
#if __cplusplus > 199711L
    mpMutex->lock();
    const size_t num = mNumDebugMessages;
    mpMutex->unlock();
    return num;
#else
    return mNumDebugMessages;
#endif
}

//! @brief Returns the number of waiting fatal messages on the message queue
size_t HopsanCoreMessageHandler::getNumFatalMessages() const
{
#if __cplusplus > 199711L
    mpMutex->lock();
    const size_t num = mNumFatalMessages;
    mpMutex->unlock();
    return num;
#else
    return mNumFatalMessages;
#endif
}

void HopsanCoreMessageHandler::printMessagesToStdOut()
{
#if __cplusplus > 199711L
    mpMutex->lock();
#endif
    std::deque<HopsanCoreMessage*>::iterator it;
    for (it=mMessageQueue.begin(); it!=mMessageQueue.end(); ++it)
    {
        std::cout << (*it)->mType << " " << (*it)->mMessage.c_str() << std::endl;
    }
#if __cplusplus > 199711L
    mpMutex->unlock();
#endif
}
