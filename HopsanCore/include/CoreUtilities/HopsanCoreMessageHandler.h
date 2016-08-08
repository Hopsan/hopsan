/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
#if __cplusplus > 199711L
#include <mutex>
#endif
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

class DLLIMPORTEXPORT HopsanCoreMessageHandler
{
private:
    std::deque<HopsanCoreMessage*> mMessageQueue;
    void addMessage(const HopsanCoreMessage::MessageEnumT, const HString &rPreFix, const HString &rMessage, const HString &rTag, const int debuglevel=0);
    void clear();
    size_t mMaxQueueSize, mNumInfoMessages, mNumWarningMessages, mNumErrorMessages, mNumFatalMessages, mNumDebugMessages;

#if __cplusplus > 199711L
    std::mutex *mpMutex;
#endif

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
