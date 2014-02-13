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
//! @file   MessageHandler.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014-02-07
//! @version $Id: MessageWidget.cpp 6081 2013-11-04 11:45:46Z petno25 $
//!
//! @brief Contains the MessageHanlder and GUIMessage classes
//!

#include <QTime>
#include <QSound>
#include <QMessageBox>

#include "common.h"
#include "global.h"
#include "MessageHandler.h"
#include "CoreAccess.h"

//! @brief Constructor for the GUIMessage class
GUIMessage::GUIMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT type)
{
    QTime time = QTime::currentTime();
    mMessage = rMessage;
    mTag = rTag;
    mTimestamp = time.toString();
    mType = type;
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



GUIMessageHandler::GUIMessageHandler()
{
    mIsPublishing = false;
    mpCoreAccess = new CoreMessagesAccess;
}

void GUIMessageHandler::clear()
{
    mMessageList.clear();
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
    if (mpCoreAccess)
    {
        bool playErrorSound = false;
        int nmsg = mpCoreAccess->getNumberOfMessages();
        for (int idx=0; idx<nmsg; ++idx)
        {
            QString message, type, tag;
            mpCoreAccess->getMessage(message, type, tag);

            // Interpret HopsanCore message type string
            if (type == "info")
            {
                addMessage(message, tag, Info);
            }
            else if( type == "warning")
            {
                addMessage(message, tag, Warning);
            }
            else if( type == "error")
            {
                addMessage(message, tag, Error);
                playErrorSound = true;
            }
            else if( type == "fatal")
            {
                addMessage(message, tag, Fatal);
                //QSound::play(QString(SOUNDSPATH) + "error.wav");
                QMessageBox::critical(gpMainWindowWidget, "Fatal Error", message+"\n\nProgram is unstable and MUST BE RESTARTED!", "Ok");
                playErrorSound = true;
            }
            else if ( type == "debug")
            {
                addMessage(message, tag, Debug);
            }
            else
            {
                addMessage(message, tag, UndefinedMessageType);
            }
        }
//        if(playErrorSound)
//        {
//            QSound::play(QString(SOUNDSPATH) + "error.wav");
//        }
    }
}

void GUIMessageHandler::addInfoMessage(QString message, QString tag)
{
    addMessage(message.prepend("Info: "), tag, Info);
}

void GUIMessageHandler::addWarningMessage(QString message, QString tag)
{
    addMessage(message.prepend("Warning: "), tag, Warning);
}

void GUIMessageHandler::addErrorMessage(QString message, QString tag)
{
    addMessage(message.prepend("Error: "), tag, Error);
    //QSound::play(QString(SOUNDSPATH) + "error.wav");
}

void GUIMessageHandler::addDebugMessage(QString message, QString tag)
{
    addMessage(message.prepend("Debug: "), tag, Debug);
}

void GUIMessageHandler::addMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT type)
{
    mMessageList.append(GUIMessage(rMessage, rTag, type));
    publishWaitingMessages();
}

void GUIMessageHandler::publishWaitingMessages()
{
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
}
