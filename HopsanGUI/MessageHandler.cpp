/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   MessageHandler.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014-02-07
//! @version $Id$
//!
//! @brief Contains the MessageHanlder and GUIMessage classes
//!

#include <QTime>
//#include <QSound>
#include <QMessageBox>

#include "common.h"
#include "global.h"
#include "MessageHandler.h"
#include "CoreAccess.h"

//! @brief Constructor for the GUIMessage class
GUIMessage::GUIMessage(const GUIMessage &rOther)
{
    mMessage = rOther.mMessage;
    mTag = rOther.mTag;
    mType = rOther.mType;
    mTimestamp = rOther.mTimestamp;
}

GUIMessage::GUIMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT type, bool doTimeStamp)
{
    mMessage = rMessage;
    mTag = rTag;
    mType = type;
    if (doTimeStamp)
    {
        mTimestamp = QTime::currentTime().toString();
    }
}

QString GUIMessage::getMessageTypeAsString() const
{
    switch (mType) {
    case Info:
        return "Information";
    case Warning:
        return "Warning";
    case Error:
        return "Error";
    case Debug:
        return "Debug";
    default:
        return "Undefined Message Type";
    }
}



GUIMessageHandler::GUIMessageHandler(QObject *pParent) :
    QObject(pParent)
{
    mIsPublishing = false;
    mpCoreAccess = new CoreMessagesAccess;
}

GUIMessageHandler::~GUIMessageHandler()
{
    delete mpCoreAccess;
}

void GUIMessageHandler::clear()
{
    mMutex.lock();
    mMessageList.clear();
    mMutex.unlock();
}

void GUIMessageHandler::startPublish()
{
    if (!mIsPublishing)
    {
        mIsPublishing = true;
        publishWaitingMessages();
    }
}

void GUIMessageHandler::stopPublish()
{
    mIsPublishing = false;
}

void GUIMessageHandler::collectHopsanCoreMessages()
{
    mCoreMutex.lock();
    if (mpCoreAccess)
    {
        //bool playErrorSound = false;
        int nmsg = mpCoreAccess->getNumberOfMessages();
        for (int idx=0; idx<nmsg; ++idx)
        {
            QString message, type, tag;
            mpCoreAccess->getMessage(message, type, tag);
            addMessageFromCore(type, tag, message);
        }
    }
    mCoreMutex.unlock();
}

void GUIMessageHandler::addMessageFromCore(QString &rType, QString &rTag, QString &rMessage)
{
    // Interpret HopsanCore message type string
    if (rType == "info")
    {
        addMessage(rMessage, rTag, Info);
    }
    else if( rType == "warning")
    {
        addMessage(rMessage, rTag, Warning);
    }
    else if( rType == "error")
    {
        addMessage(rMessage, rTag, Error);
        //playErrorSound = true;
    }
    else if( rType == "fatal")
    {
        addMessage(rMessage, rTag, Fatal);
        //QSound::play(QString(SOUNDSPATH) + "error.wav");
        QMessageBox::critical(gpMainWindowWidget, "Fatal Error", rMessage+"\n\nProgram is unstable and MUST BE RESTARTED!", "Ok");
        //playErrorSound = true;
    }
    else if ( rType == "debug")
    {
        addMessage(rMessage, rTag, Debug);
    }
    else
    {
        addMessage(rMessage, rTag, UndefinedMessageType);
    }

    //        if(playErrorSound)
    //        {
    //            QSound::play(QString(SOUNDSPATH) + "error.wav");
    //        }
}


void GUIMessageHandler::addInfoMessage(QString message, QString tag, bool doTimeStamp)
{
    addMessage(message.prepend("Info: "), tag, Info, doTimeStamp);
}

void GUIMessageHandler::addWarningMessage(QString message, QString tag, bool doTimeStamp)
{
    addMessage(message.prepend("Warning: "), tag, Warning, doTimeStamp);
}

void GUIMessageHandler::addErrorMessage(QString message, QString tag, bool doTimeStamp)
{
    addMessage(message.prepend("Error: "), tag, Error, doTimeStamp);
    //QSound::play(QString(SOUNDSPATH) + "error.wav");
}

void GUIMessageHandler::addDebugMessage(QString message, QString tag, bool doTimeStamp)
{
    addMessage(message.prepend("Debug: "), tag, Debug, doTimeStamp);
}

void GUIMessageHandler::addMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT type, bool doTimeStamp)
{
    mMutex.lock();
    mMessageList.append(GUIMessage(rMessage, rTag, type, doTimeStamp));
    mMutex.unlock();
    publishWaitingMessages();
}

void GUIMessageHandler::publishWaitingMessages()
{
    mMutex.lock();
    while (mIsPublishing && !mMessageList.isEmpty())
    {
        switch (mMessageList.front().mType)
        {
        case Info:
            emit newInfoMessage(mMessageList.front());
            break;
        case Warning:
            emit newWarningMessage(mMessageList.front());
            break;
        case Error:
        case Fatal:
            emit newErrorMessage(mMessageList.front());
            break;
        case Debug:
            emit newDebugMessage(mMessageList.front());
            break;
        default:
            // We use info message for undefined messages (should note exist)
            emit newInfoMessage(mMessageList.front());
            break;
        }
        // Emmit the any message signal
        emit newAnyMessage(mMessageList.front());

        // Now remove the message as everyone has been notified
        mMessageList.removeFirst();
    }
    mMutex.unlock();
}
