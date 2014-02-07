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
//! @file   MessageHandler.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014-02-07
//!
//! @brief Contains the MessageHanlder and GUIMessage classes
//!
//$Id: MessageWidget.h 5849 2013-09-06 08:39:07Z robbr48 $

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include <QList>
#include <QString>


// Forward declaration
class CoreMessagesAccess;

enum MessageTypeEnumT {Info, Warning, Error, Fatal, Debug, UndefinedMessageType};
class GUIMessage
{
public:
    GUIMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT mType);
    QString getMessageTypeAsString() const;

    QString mMessage;
    QString mTag;
    QString mTimestamp;
    MessageTypeEnumT mType;
};
//! @todo maybe pass messages around as shared pointers

class GUIMessageHandler : public QObject
{
    Q_OBJECT

public:
    GUIMessageHandler();
    void clear();
    void startPublish();
    void stopPublish();

    void publishWaitingMessages();

public slots:
    void collectHopsanCoreMessages();
    void addInfoMessage(QString message, QString tag=QString());
    void addWarningMessage(QString message, QString tag=QString());
    void addErrorMessage(QString message, QString tag=QString());
    void addDebugMessage(QString message, QString tag=QString());

signals:
    void newAnyMessage(const GUIMessage &rMessage);
    void newInfoMessage(const GUIMessage &rMessage);
    void newWarningMessage(const GUIMessage &rMessage);
    void newErrorMessage(const GUIMessage &rMessage);
    void newDebugMessage(const GUIMessage &rMessage);

private:
    void addMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT type);

    CoreMessagesAccess *mpCoreAccess;
    QList<GUIMessage> mMessageList;
    bool mIsPublishing;
};

#endif // MESSAGEHANDLER_H
