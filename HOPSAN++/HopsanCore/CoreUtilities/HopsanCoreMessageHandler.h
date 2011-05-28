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
#include <string>
#include "../win32dll.h"

namespace hopsan {

    class DLLIMPORTEXPORT HopsanCoreMessage
    {
    public:
        enum MessageTypes {INFO, WARNING, ERROR, DEBUG};
        int type;
        int debuglevel;
        std::string message;
        std::string tag;
    };

    class DLLIMPORTEXPORT HopsanCoreMessageHandler
    {
    private:
        std::queue<HopsanCoreMessage> mMessageQueue;
        void addMessage(int type, std::string preFix, std::string message, std::string tag, int debuglevel=0);
        size_t mMaxQueueSize;

    public:
        HopsanCoreMessageHandler();
        void addInfoMessage(std::string message, std::string tag="", int dbglevel=0);
        void addWarningMessage(std::string message, std::string tag="", int dbglevel=0);
        void addErrorMessage(std::string message, std::string tag="", int dbglevel=0);
        void addDebugMessage(std::string message, std::string tag="", int dbglevel=0);

        //const HopsanCoreMessage &peakMessage();
        HopsanCoreMessage getMessage();
        size_t nWaitingMessages();
    };

    extern HopsanCoreMessageHandler gCoreMessageHandler;
    DLLIMPORTEXPORT HopsanCoreMessageHandler* getCoreMessageHandlerPtr();
}

#endif // HOPSANCOREMESSAGEHANDLER_H
