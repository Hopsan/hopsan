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
//! @brief Contains the HcomWidget that displays messages to the user
//!
//$Id$

#ifndef HCOMWIDGET_H
#define HCOMWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>

#include "MessageHandler.h"

class TerminalConsole;
class HcomHandler;
class VectorVariable;
typedef QSharedPointer<VectorVariable> SharedVectorVariableT;

class TerminalWidget : public QWidget
{
    Q_OBJECT
public:
    TerminalWidget(QWidget *pParent=0);
    QSize sizeHint() const;
    void loadConfig();
    void saveConfig();
    void setAbortButtonEnabled(bool enable);

    //! @todo should not be public
    TerminalConsole *mpConsole;
    HcomHandler *mpHandler;

public slots:
    void printMessage(const GUIMessage &rMessage);

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QPushButton *mpAbortHCOMWidgetButton;
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

    void printInfoMessage(QString message,      QString tag="", bool timeStamp=true, bool force=false);
    void printWarningMessage(QString message,   QString tag="", bool timeStamp=true, bool force=false);
    void printErrorMessage(QString message,     QString tag="", bool timeStamp=true, bool force=false);
    void printFatalMessage(QString message, bool force=false);
    void printDebugMessage(QString message,     QString tag="", bool timeStamp=true, bool force=false);
    void print(QString message, bool force=false);

    TerminalWidget *mpTerminal;

public slots:
    void printMessage(const GUIMessage &rMessage, bool timeStamp=true, bool force=false);
    void clear();
    void abortHCOM();
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDebugMessages(bool value);
    bool getDontPrint() const;
    bool getDontPrintErrors() const;
    void setDontPrint(const bool value, const bool ignoreErrors);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

private:
    //Output
    void setOutputColor(MessageTypeEnumT type);

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
    QString mHistoryFilter;

    //Autocomplete
    QString mAutoCompleteFilter;
    QStringList mAutoCompleteResults;
    int mCurrentAutoCompleteIndex;

    QString mLastTag;
    int mSubsequentTags;
    bool mGroupByTag;
    bool mDontPrint;
    bool mDontPrintErrors;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDebugMessages;

    //Audio
    //QSound *mpErrorSound;
};

#endif // HCOMWIDGET_H
