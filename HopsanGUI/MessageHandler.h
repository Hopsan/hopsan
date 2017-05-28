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
//! @file   MessageHandler.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014-02-07
//!
//! @brief Contains the MessageHanlder and GUIMessage classes
//!
//$Id$

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMutex>


// Forward declaration
class CoreMessagesAccess;

enum MessageTypeEnumT {Info, Warning, Error, Fatal, Debug, UndefinedMessageType};
class GUIMessage
{
public:
    GUIMessage() {}
    GUIMessage(const GUIMessage &rOther);
    GUIMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT type, bool doTimeStamp=true);
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
    GUIMessageHandler(QObject *pParent=0);
    ~GUIMessageHandler();
    void clear();
    void startPublish();
    void stopPublish();

    void publishWaitingMessages();

public slots:
    void collectHopsanCoreMessages();
    void addMessageFromCore(QString &rType, QString &rTag, QString &rMessage);
    void addInfoMessage(QString message, QString tag=QString(), bool doTimeStamp=true);
    void addWarningMessage(QString message, QString tag=QString(), bool doTimeStamp=true);
    void addErrorMessage(QString message, QString tag=QString(), bool doTimeStamp=true);
    void addDebugMessage(QString message, QString tag=QString(), bool doTimeStamp=true);

signals:
    void newAnyMessage(const GUIMessage &rMessage);
    void newInfoMessage(const GUIMessage &rMessage);
    void newWarningMessage(const GUIMessage &rMessage);
    void newErrorMessage(const GUIMessage &rMessage);
    void newDebugMessage(const GUIMessage &rMessage);

private:
    void addMessage(const QString &rMessage, const QString &rTag, const MessageTypeEnumT type, bool doTimeStamp=true);

    CoreMessagesAccess *mpCoreAccess;
    QList<GUIMessage> mMessageList;
    bool mIsPublishing;
    QMutex mMutex;
    QMutex mCoreMutex;
};

#endif // MESSAGEHANDLER_H
