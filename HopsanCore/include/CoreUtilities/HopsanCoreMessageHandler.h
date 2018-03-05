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
//! @file   HopsanCoreMessageHandler.h
//! @author <peter.nordin@liu.se>
//! @date   2010-03-03
//!
//! @brief Contains the Classes for hopsancore -> main program message exchange
//!
//$Id$

#ifndef HOPSANCOREMESSAGEHANDLER_H
#define HOPSANCOREMESSAGEHANDLER_H

#include <deque>
#include "HopsanTypes.h"
#include "win32dll.h"

namespace hopsan {

class HopsanCoreMessage
{
public:
    enum MessageEnumT {Info, Warning, Error, Debug, Fatal};
    HopsanCoreMessage(const MessageEnumT type, const HString &rMessage, const HString &rTag, const int debugLevel) :
        mType(type), mMessage(rMessage), mTag(rTag), mDebugLevel(debugLevel) {}
    HopsanCoreMessage() : mType(0), mDebugLevel(0) {}

    HopsanCoreMessage &operator=(const HopsanCoreMessage &src)
    {
        mType = src.mType;
        mMessage = src.mMessage;
        mTag = src.mTag;
        mDebugLevel = src.mDebugLevel;
        return (*this);
    }

    int mType;
    HString mMessage;
    HString mTag;
    int mDebugLevel;
};

class HopsanCoreMessageHandlerPrivates;

class HOPSANCORE_DLLAPI HopsanCoreMessageHandler
{
private:
    std::deque<HopsanCoreMessage*> mMessageQueue;
    void addMessage(const HopsanCoreMessage::MessageEnumT, const HString &rPreFix, const HString &rMessage, const HString &rTag, const int debuglevel=0);
    void clear();
    size_t mMaxQueueSize, mNumInfoMessages, mNumWarningMessages, mNumErrorMessages, mNumFatalMessages, mNumDebugMessages;

    HopsanCoreMessageHandlerPrivates *mpPrivates;

public:
    HopsanCoreMessageHandler();
    ~HopsanCoreMessageHandler();

    void addInfoMessage(const HString &rMessage, const HString &rTag="", const int dbglevel=0);
    void addWarningMessage(const HString &rMessage, const HString &rTag="", const int dbglevel=0);
    void addErrorMessage(const HString &rMessage, const HString &rTag="", const int dbglevel=0);
    void addDebugMessage(const HString &rMessage, const HString &rTag="", const int dbglevel=0);
    void addFatalMessage(const HString &rMessage, const HString &rTag="", const int dbglevel=0);

    void getMessage(HString &rMessage, HString &rType, HString &rTag);
    size_t getNumWaitingMessages() const;
    size_t getNumInfoMessages() const;
    size_t getNumWarningMessages() const;
    size_t getNumErrorMessages() const;
    size_t getNumDebugMessages() const;
    size_t getNumFatalMessages() const;

    void printMessagesToStdOut();
};
}

#endif // HOPSANCOREMESSAGEHANDLER_H
