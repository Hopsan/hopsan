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

#include <QtGui>

class MainWindow;
class CoreMessagesAccess;
class GUIMessage;

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    MessageWidget(MainWindow *pParent=0);
    void printCoreMessages();
    void printGUIInfoMessage(QString message, QString tag=QString());
    void printGUIErrorMessage(QString message, QString tag=QString());
    void printGUIWarningMessage(QString message, QString tag=QString());
    void printGUIDebugMessage(QString message, QString tag=QString());
    QSize sizeHint() const;
    void loadConfig();
    bool textEditHasFocus();

public slots:
    void clear();
    void checkMessages();
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDebugMessages(bool value);
    void copy();

private:
    void setMessageColor(QString type);
    void updateNewMessagesOnly();
    void updateEverything();
    void appendOneMessage(GUIMessage msg);
    QList< GUIMessage > mNewMessageList;
    QList< GUIMessage > mPrintedMessageList;
    bool mGroupByTag;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDebugMessages;
    QString mLastTag;
    int mSubsequentTags;

    CoreMessagesAccess *mpCoreAccess;

    QTextEdit *mpTextEdit;
    QGridLayout *mpLayout;
    QPushButton *mpClearMessageWidgetButton;
    QToolButton *mpShowErrorMessagesButton;
    QToolButton *mpShowWarningMessagesButton;
    QToolButton *mpShowInfoMessagesButton;
    QToolButton *mpShowDebugMessagesButton;
    QCheckBox *mpGroupByTagCheckBox;
};


class GUIMessage
{
public:
    GUIMessage(QString message, QString type, QString tag="");
    QString message;
    QString type;
    QString tag;
    QString time;
};

#endif // MESSAGEWIDGET_H
