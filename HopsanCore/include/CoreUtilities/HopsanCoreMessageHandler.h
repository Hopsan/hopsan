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
#include "HopsanTypes.h"
#include "win32dll.h"

#ifdef USETBB
// Forward declaration
namespace tbb{
class mutex;
}
#endif

namespace hopsan {

class HopsanCoreMessage
{
public:
    enum MessageEnumT {Info, Warning, Error, Debug, Fatal};
    HopsanCoreMessage()
    {
        mType = 0;
        mDebugLevel = 0;
    }

    HopsanCoreMessage &operator=(const HopsanCoreMessage &src)
    {
        mType = src.mType;
        mDebugLevel = src.mDebugLevel;
        mTag = src.mTag;
        mMessage = src.mMessage;
        return (*this);
    }

    int mType;
    int mDebugLevel;
    HString mMessage;
    HString mTag;
};

class HopsanCoreMessageHandler
{
private:
    std::queue<HopsanCoreMessage*> mMessageQueue;
    void addMessage(const int type, const HString &rPreFix, const HString &rMessage, const HString &rTag, const int debuglevel=0);
    void clear();
    size_t mMaxQueueSize, mNumInfoMessages, mNumWarningMessages, mNumErrorMessages, mNumFatalMessages, mNumDebugMessages;

#ifdef USETBB
    tbb::mutex *mpMutex;
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
};

}

#endif // HOPSANCOREMESSAGEHANDLER_H
