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
#include <QCompleter>
#include <QTimer>

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

private slots:
    void insertCompletion(QString completionResult);
    void resetBackgroundColor();

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

    QCompleter *mpCompleter;

    QTimer mBackgroundColorTimer;
};

#endif // HCOMWIDGET_H
