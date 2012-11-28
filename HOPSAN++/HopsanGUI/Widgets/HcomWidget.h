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




class TerminalWidget : public QWidget
{
    Q_OBJECT
public:
    TerminalWidget(MainWindow *pParent=0);
    QSize sizeHint() const;
    void loadConfig();
    void saveConfig();
    TerminalConsole *mpConsole;
    HcomHandler *mpHandler;

public slots:
    void checkMessages();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QPushButton *mpClearMessageWidgetButton;
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
    void printErrorMessage(QString message, QString tag="", bool timeStamp=true);
    void printWarningMessage(QString message, QString tag="", bool timeStamp=true);
    void printInfoMessage(QString message, QString tag="", bool timeStamp=true);
    void printDebugMessage(QString message, QString tag="", bool timeStamp=true);
    void print(QString message);
    void updateNewMessages();
    void appendOneMessage(GUIMessage msg, bool timeStamp="");

public slots:
    void checkMessages();
    void clear();
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDebugMessages(bool value);

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
    void cancelAutoComplete();
    void cancelRecentHistory();

    TerminalWidget *mpParent;

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
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDebugMessages;
};


class HcomHandler
{
public:
    enum VariableType{Scalar, DataVector};
    HcomHandler(TerminalConsole *pConsole);

    QStringList getCommands();

    //Command functions
    void executeCommand(QString cmd);
    void executeExitCommand(QString cmd);
    void executeSimulateCommand(QString cmd);
    void executePlotCommand(QString cmd);
    void executePlotLeftAxisCommand(QString cmd);
    void executePlotRightAxisCommand(QString cmd);
    void executeDisplayParameterCommand(QString cmd);
    void executeChangeParameterCommand(QString cmd);
    void executeChangeSimulationSettingsCommand(QString cmd);
    void executeHelpCommand(QString cmd);
    void executeRunScriptCommand(QString cmd);
    void executeWriteHistoryToFileCommand(QString cmd);
    void executePrintCommand(QString cmd);
    void executeChangePlotWindowCommand(QString cmd);
    void executeDisplayPlotWindowCommand(QString cmd);
    void executeDisplayVariablesCommand(QString cmd);
    void executePeekCommand(QString cmd);
    void executePokeCommand(QString cmd);
    void executeDefineAliasCommand(QString cmd);
    void executeSetCommand(QString cmd);
    void executeSaveToPloCommand(QString cmd);
    void executeLoadModelCommand(QString cmd);
    void executeLoadRecentCommand(QString cmd);
    void executePwdCommand(QString cmd);
    void executeChangeDirectoryCommand(QString cmd);
    void executeListFilesCommand(QString cmd);
    void executeCloseModelCommand(QString cmd);
    void executeChangeTabCommand(QString cmd);
    void executeAddComponentCommand(QString cmd);
    void executeConnectCommand(QString cmd);
    void executeCreateModelCommand(QString cmd);

    //Help functions
    void changePlotVariables(QString cmd, int axis);
    void addPlotCurve(QString cmd, int axis);
    void removePlotCurves(int axis);
    QString evaluateExpression(QString expr, VariableType *returnType, bool *evalOk);
    void getComponents(QString str, QList<ModelObject*> &components);
    void getParameters(QString str, ModelObject* pComponent, QStringList &parameters);
    void getParameters(QString str, QStringList &parameters);
    QString getParameterValue(QString parameter);
    void getVariables(QString str, QStringList &variables);
    QString getWorkingDirectory();
    bool evaluateArithmeticExpression(QString cmd);
    void splitAtFirst(QString str, QString c, QString &left, QString &right);
    bool containsOutsideParentheses(QString str, QString c);
    QString runScriptCommands(QStringList lines);
    LogVariableData *getVariablePtr(QString fullName);
    double getNumber(QString str, bool *ok);
    void toShortDataNames(QString &variable);
    QString getDirectory(QString cmd);

private:
    TerminalConsole *mpConsole;

    //Working directory
    QString mPwd;

    //Commands
    QList<HcomCommand> mCmdList;

    //Plotting
    QString mCurrentPlotWindow;

    //Local variables
    QMap<QString, double> mLocalVars;
};


class HcomCommand
{
public:
    void runCommand(QString cmd, HcomHandler *pHandler)
    {
        //(fnc)(cmd);
        (pHandler->*HcomCommand::fnc)(cmd);
    }

    QString cmd;
    QString help;
    void (HcomHandler::*fnc)(QString);
};

#endif // HCOMWIDGET_H
