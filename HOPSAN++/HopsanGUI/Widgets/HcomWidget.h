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
//! @file   HcomWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-11-19
//!
//! @brief Contains the HcomWidget that dissplays messages to the user
//!
//$Id$

#ifndef HCOMWIDGET_H
#define HCOMWIDGET_H

#include <QtGui>

#include "Widgets/MessageWidget.h"

class MainWindow;
class Port;
class ModelObject;
class LogVariableData;
class TerminalConsole;
class HcomHandler;
class HcomCommand;
class CoreMessagesAccess;

typedef QSharedPointer<LogVariableData> SharedLogVariableDataPtrT;

class TerminalWidget : public QWidget
{
    Q_OBJECT
public:
    TerminalWidget(QWidget *pParent=0);
    QSize sizeHint() const;
    void loadConfig();
    void saveConfig();
    TerminalConsole *mpConsole;
    HcomHandler *mpHandler;
    void setEnabledAbortButton(bool enable);

public slots:
    void checkMessages();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QPushButton *mpClearMessageWidgetButton;
    QPushButton *mpAbortHCOMWidgetButton;
    QToolButton *mpShowErrorMessagesButton;
    QToolButton *mpShowWarningMessagesButton;
    QToolButton *mpShowInfoMessagesButton;
    QToolButton *mpShowDebugMessagesButton;
    QCheckBox *mpGroupByTagCheckBox;
};


class TerminalConsole : public QTextEdit
{
    Q_OBJECT

    friend class TerminalWidget;
    friend class HcomHandler;

public:
    TerminalConsole(TerminalWidget *pParent=0);
    void printFirstInfo();
    HcomHandler *getHandler();

    void printCoreMessages();
    void printFatalMessage(QString message);
    void printErrorMessage(QString message, QString tag="", bool timeStamp=true);
    void printWarningMessage(QString message, QString tag="", bool timeStamp=true);
    void printInfoMessage(QString message, QString tag="", bool timeStamp=true);
    void printDebugMessage(QString message, QString tag="", bool timeStamp=true);
    void print(QString message);
    void updateNewMessages();
    void appendOneMessage(GUIMessage msg, bool timeStamp="");

    TerminalWidget *mpTerminal;

public slots:
    void checkMessages();
    void clear();
    void abortHCOM();
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDebugMessages(bool value);
    void setDontPrint(bool value);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

private:
    //Output
    void setOutputColor(QString type);

    //Cursor & keypress functions
    bool isOnLastLine();
    void handleEnterKeyPress();
    void handleUpKeyPress();
    void handleDownKeyPress();
    void handleTabKeyPress();
    void handleHomeKeyPress();
    void handleEscapeKeyPress();
    void cancelAutoComplete();
    void cancelRecentHistory();

    CoreMessagesAccess *mpCoreAccess;

    //Recent history (up and down keys)
    QStringList mHistory;
    int mCurrentHistoryItem;

    //Autocomplete
    QString mAutoCompleteFilter;
    QStringList mAutoCompleteResults;
    int mCurrentAutoCompleteIndex;

    QList< GUIMessage > mNewMessageList;
    QList< GUIMessage > mPrintedMessageList;
    QString mLastTag;
    int mSubsequentTags;
    bool mGroupByTag;
    bool mDontPrint;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDebugMessages;

    //Audio
    //QSound *mpErrorSound;
};

#endif // HCOMWIDGET_H
