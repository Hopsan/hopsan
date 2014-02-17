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
//! @file   MessageWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-10-xx
//!
//! @brief Contains the MessageWidget that dissplays messages to the user
//!
//$Id$

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QWidget>
#include "MessageHandler.h"

// Forward Declaration
class QTextEdit;
class QCheckBox;

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    MessageWidget(QWidget *pParent=0);
    void addText(const QString &rText);
    QSize sizeHint() const;
    void loadConfig();
    bool textEditHasFocus();

public slots:
    void receiveMessage(const GUIMessage &rMessage);
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDebugMessages(bool value);
    void clear();
    void copy();

protected:
    void mouseMoveEvent(QMouseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void printNewMessagesOnly();
    void reprintEverything();
    void determineMessageColor(MessageTypeEnumT type);
    void printOneMessage(const GUIMessage &rMessage);
    QList< GUIMessage > mNewMessageList;
    QList< GUIMessage > mPrintedMessageList;
    bool mGroupByTag;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDebugMessages;
    QString mLastTag;
    int mSubsequentTags;

    QTextEdit *mpTextEdit;
    QCheckBox *mpGroupByTagCheckBox;
};




#endif // MESSAGEWIDGET_H
