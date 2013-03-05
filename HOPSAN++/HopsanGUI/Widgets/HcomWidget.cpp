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
//! @file   HcomWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-09-19
//! @version $Id$
//!
//! @brief Contains the HCOM terminal widget
//!

#include "HcomWidget.h"
#include "MainWindow.h"
#include "Configuration.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PlotWidget.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "PlotWindow.h"
#include "Widgets/MessageWidget.h"
#include <QDateTime>
#include <cmath>
#include "PlotTab.h"
#include "PlotCurve.h"
#include "PlotHandler.h"
#include "symhop/SymHop.h"
#include "Utilities/GUIUtilities.h"


TerminalWidget::TerminalWidget(MainWindow *pParent)
    : QWidget(pParent)
{
    this->setMouseTracking(true);

    mpClearMessageWidgetButton = new QPushButton("Clear Messages");
    QFont tempFont = mpClearMessageWidgetButton->font();
    tempFont.setBold(true);
    mpClearMessageWidgetButton->setFont(tempFont);
    mpClearMessageWidgetButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    mpShowErrorMessagesButton = new QToolButton();
    mpShowErrorMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowErrorMessages.png"));
    mpShowErrorMessagesButton->setCheckable(true);
    mpShowErrorMessagesButton->setChecked(true);
    mpShowErrorMessagesButton->setToolTip("Show Error Messages");

    mpShowWarningMessagesButton = new QToolButton();
    mpShowWarningMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowWarningMessages.png"));
    mpShowWarningMessagesButton->setCheckable(true);
    mpShowWarningMessagesButton->setChecked(true);
    mpShowWarningMessagesButton->setToolTip("Show Warning Messages");

    mpShowInfoMessagesButton = new QToolButton();
    mpShowInfoMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowInfoMessages.png"));
    mpShowInfoMessagesButton->setCheckable(true);
    mpShowInfoMessagesButton->setChecked(true);
    mpShowInfoMessagesButton->setToolTip("Show Info Messages");

    mpShowDebugMessagesButton = new QToolButton();
    mpShowDebugMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowDebugMessages.png"));
    mpShowDebugMessagesButton->setCheckable(true);
    mpShowDebugMessagesButton->setChecked(false);
    mpShowDebugMessagesButton->setToolTip("Show Debug Messages");

    mpGroupByTagCheckBox = new QCheckBox("Group Similar Messages");
    mpGroupByTagCheckBox->setChecked(gConfig.getGroupMessagesByTag());

    mpConsole = new TerminalConsole(this);
    mpHandler = new HcomHandler(mpConsole);

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpConsole,0,0,1,7);
    pLayout->addWidget(mpClearMessageWidgetButton,1,0,1,1);
    pLayout->addWidget(mpShowErrorMessagesButton,1,1,1,1);
    pLayout->addWidget(mpShowWarningMessagesButton,1,2,1,1);
    pLayout->addWidget(mpShowInfoMessagesButton,1,3,1,1);
    pLayout->addWidget(mpShowDebugMessagesButton,1,4,1,1);
    pLayout->addWidget(mpGroupByTagCheckBox, 1,5,1,1);
    pLayout->setContentsMargins(4,4,4,4);
    this->setLayout(pLayout);

    this->installEventFilter(this);
    this->setMouseTracking(true);
    this->setMinimumHeight(155);

    connect(mpClearMessageWidgetButton, SIGNAL(clicked()),mpConsole, SLOT(clear()));
    connect(mpShowErrorMessagesButton, SIGNAL(toggled(bool)), mpConsole, SLOT(showErrorMessages(bool)));
    connect(mpShowWarningMessagesButton, SIGNAL(toggled(bool)), mpConsole, SLOT(showWarningMessages(bool)));
    connect(mpShowInfoMessagesButton, SIGNAL(toggled(bool)), mpConsole, SLOT(showInfoMessages(bool)));
    connect(mpShowDebugMessagesButton, SIGNAL(toggled(bool)), mpConsole, SLOT(showDebugMessages(bool)));
    connect(mpGroupByTagCheckBox, SIGNAL(toggled(bool)), mpConsole, SLOT(setGroupByTag(bool)));
}


//! @brief Reimplementation of QWidget::sizeHint(), used to reduce the size of the message widget when docked
QSize TerminalWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 1; //pixels
    return size;
}


//! @brief Loads terminal widget settings from configuration object
//! This is needed because terminal widget must be created before configuration
//! (so that config can print messages), which means that message widgets cannot
//! load the config directly in the constructor.
void TerminalWidget::loadConfig()
{
    mpGroupByTagCheckBox->setChecked(gConfig.getGroupMessagesByTag());
    mpConsole->mHistory = gConfig.getTerminalHistory();
}



void TerminalWidget::saveConfig()
{
    if(mpConsole->mHistory.size() > gConfig.getTerminalHistory().size())
    {
        mpConsole->mHistory.prepend("--- "+QDateTime::currentDateTime().toString()+" ---");
        gConfig.storeTerminalHistory(mpConsole->mHistory);
    }
}


void TerminalWidget::mouseMoveEvent(QMouseEvent *event)
{
    mpConsole->setFrameShape(QFrame::NoFrame);

    //qDebug() << "Mouse move event!";

    QWidget::mouseMoveEvent(event);
}


bool TerminalWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        //qDebug() << "Ate an event!";

        mpConsole->setFrameShape(QFrame::NoFrame);

        return true;
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}


//! @brief Slot that checks messages from core and prints them
//! @todo Is this function necessary? All it does is calling another one...
void TerminalWidget::checkMessages()
{
    mpConsole->printCoreMessages();
}


//! @brief Constructor for HCOM console widget
TerminalConsole::TerminalConsole(TerminalWidget *pParent)
    : QTextEdit(pParent)
{
    mpParent = pParent;

    this->setReadOnly(false);
    this->setMouseTracking(true);

    //Setup font
    QFont monoFont = QFont("Monospace", 9, 50);
    monoFont.setStyleHint(QFont::TypeWriter);
    this->setFont(monoFont);

    //Setup colors
    //this->setTextColor(QColor("Black"));
    //this->setStyleSheet(QString::fromUtf8("QTextEdit {background-color: white; border: 1px solid gray;}"));

    mGroupByTag = gConfig.getGroupMessagesByTag();

    mShowErrorMessages = true;
    mShowWarningMessages = true;
    mShowInfoMessages = true;
    mShowDebugMessages = false;

    mCurrentHistoryItem=-1;
    this->append(">> ");
}


void TerminalConsole::printFirstInfo()
{
    this->print("--------------------------------------");
    this->print(" Hopsan HCOM Terminal                   ");
    this->print(" Write \"help\" for a list of commands. ");
    this->print("--------------------------------------");
}


HcomHandler *TerminalConsole::getHandler()
{
    return mpParent->mpHandler;
}


//! @brief Obtains messages from core and prints them in the message widget
void TerminalConsole::printCoreMessages()
{
    int nmsg = mpCoreAccess->getNumberOfMessages();
    //nmsg = 0; //!< @warning Fix for Petter should not be checked into the repository

    bool playErrorSound = false;
    for (int idx=0; idx < nmsg; ++idx)
    {
        QString message, type, tag;
        mpCoreAccess->getMessage(message, type, tag);
        if(type == "error")
            playErrorSound = true;
        mNewMessageList.append(GUIMessage(message, type, tag));
        updateNewMessages();
    }
    if(playErrorSound)
    {
        QSound::play(QString(SOUNDSPATH) + "error.wav");
    }
}


void TerminalConsole::printErrorMessage(QString message, QString tag, bool timeStamp)
{
    QSound::play(QString(SOUNDSPATH) + "error.wav");
    appendOneMessage(GUIMessage(message.prepend("Error: "), "error", tag), timeStamp);
}


void TerminalConsole::printWarningMessage(QString message, QString tag, bool timeStamp)
{
    appendOneMessage(GUIMessage(message.prepend("Warning: "), "warning", tag), timeStamp);
}


void TerminalConsole::printInfoMessage(QString message, QString tag, bool timeStamp)
{
    appendOneMessage(GUIMessage(message.prepend("Info: "), "info", tag), timeStamp);
}


void TerminalConsole::print(QString message)
{
    appendOneMessage(GUIMessage(message, "default", ""), false);
}


void TerminalConsole::printDebugMessage(QString message, QString tag, bool timeStamp)
{
    appendOneMessage(GUIMessage(message.prepend("Debug: "), "debug", tag), timeStamp);
}


void TerminalConsole::updateNewMessages()
{
        //Loop through message list and print messages
    for(int msg=0; msg<mNewMessageList.size(); ++msg)
    {
        appendOneMessage(mNewMessageList.at(msg));
    }

    mPrintedMessageList.append(mNewMessageList);
    mNewMessageList.clear();

    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}


void TerminalConsole::appendOneMessage(GUIMessage msg, bool timeStamp)
{
    if( !(msg.type == "error" && !mShowErrorMessages) &&          //Do not show message if its type shall not be shown
        !(msg.type == "warning" && !mShowWarningMessages) &&
        !(msg.type == "info" && !mShowInfoMessages) &&
        !(msg.type == "debug" && !mShowDebugMessages))
    {
        QString output = msg.message;
        if(timeStamp)
        {
            output.prepend("["+msg.time+"] ");
        }

        if(!msg.tag.isEmpty() && msg.tag == mLastTag && mGroupByTag)     //Message is tagged, and group by tag setting is active
        {
            ++mSubsequentTags;
            QString numString;
            numString.setNum(mSubsequentTags);
            this->undo();
            output.append("    (" + numString + " similar)");
        }
        else        //Message is not tagged, or group by tag setting is not active
        {
            mSubsequentTags = 1;
            mLastTag =msg.tag;
        }


        this->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
        setOutputColor(msg.type);
        this->insertPlainText(output+"\n");
        this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );

        setOutputColor("default");
    }
}



//! @brief Slot that checks messages from core and prints them
//! @todo Is this function necessary? All it does is calling another one...
void TerminalConsole::checkMessages()
{
    printCoreMessages();
}


//! @brief Clear function for message widget, this will empty the message widget and also remove all messages from the list
void TerminalConsole::clear()
{
    QTextEdit::clear();
    append(">> ");
    mPrintedMessageList.clear();
    mNewMessageList.clear();
   // updateEverything();
}


//! @brief Tells the message widget wether or not messages shall be grouped by tags
//! @param value True means that messages shall be grouped
void TerminalConsole::setGroupByTag(bool value)
{
    mGroupByTag = value;
    gConfig.setGroupMessagesByTag(value);
  //  updateEverything();
}


//! @brief Tells the message widget wether or not it shall show error messages
//! @param value True means show messages
void TerminalConsole::showErrorMessages(bool value)
{
    mShowErrorMessages = value;
 //   updateEverything();
}


//! @brief Tells the message widget wether or not it shall show warning messages
//! @param value True means show messages
void TerminalConsole::showWarningMessages(bool value)
{
    mShowWarningMessages = value;
 //   updateEverything();
}


//! @brief Tells the message widget wether or not it shall show info messages
//! @param value True means show messages
void TerminalConsole::showInfoMessages(bool value)
{
    mShowInfoMessages = value;
 //   updateEverything();
}


//! @brief Tells the message widget wether or not it shall show debug messages
//! @param value True means show messages
void TerminalConsole::showDebugMessages(bool value)
{
    mShowDebugMessages = value;
//   updateEverything();
}



void TerminalConsole::setOutputColor(QString type)
{
    if (type == "error")
    {
        this->setTextColor(QColor("Red"));
    }
    else if (type == "warning")
    {
        this->setTextColor(QColor(216, 115, 0));
    }
    else if (type == "info")
    {
        this->setTextColor("BLACK");
    }
    else if (type == "debug")
    {
        this->setTextColor("BLUE");
    }
    else
    {
        this->setTextColor("BLACK");
    }
}


bool TerminalConsole::isOnLastLine()
{
    int row = this->textCursor().blockNumber();
    int col = this->textCursor().columnNumber();
    int rows = this->document()->blockCount();

    return (row == rows-1 && col > 2);
}


void TerminalConsole::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Home)
    {
        handleHomeKeyPress();
        return;
    }

    if(isOnLastLine())
    {
        int col = this->textCursor().columnNumber();
        if((event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Left) && col == 3)
        {
            return;     //Don't allow backspace or left key if on first allowed character
        }
        else if(event->key() == Qt::Key_Up)
        {
            cancelAutoComplete();
            handleUpKeyPress();
            return;
        }
        else if(event->key() == Qt::Key_Down)
        {
            cancelAutoComplete();
            handleDownKeyPress();
            return;
        }
        else if(event->key() == Qt::Key_Tab)
        {
            handleTabKeyPress();
            return;
        }
        else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            cancelRecentHistory();
            cancelAutoComplete();
            handleEnterKeyPress();
            return;
        }
        cancelRecentHistory();
        cancelAutoComplete();
        QTextEdit::keyPressEvent(event);
    }
    else
    {
        if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Down || event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
        {
            //Move cursor back to last line
            this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
        }
    }
}


//! @brief Handles enter key press (i.e. execute command)
void TerminalConsole::handleEnterKeyPress()
{
    //Parse command
    QString cmd = this->document()->lastBlock().text();
    cmd = cmd.right(cmd.size()-3);

    if(cmd.startsWith("--- "))  //! @todo What does this do?
    {
        return;
    }

    //Append new line (for message output appearing below command)
    this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
    this->append("");

    if(!cmd.isEmpty())
    {
        //Execute command
        getHandler()->executeCommand(cmd);

        //Add command to history
        mHistory.prepend(cmd);
    }

    //Insert new command line
    this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
    this->setOutputColor("default");
    this->insertPlainText(">> ");
    this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );

    //Scroll to bottom
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}


void TerminalConsole::handleUpKeyPress()
{
    if(mHistory.isEmpty()) { return; }

    this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor );
    this->textCursor().removeSelectedText();

    ++mCurrentHistoryItem;
    if(mCurrentHistoryItem > mHistory.size()-1)
    {
        mCurrentHistoryItem = mHistory.size()-1;
    }
    setOutputColor("default");
    this->insertPlainText(">> " + mHistory.at(mCurrentHistoryItem));

    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}


void TerminalConsole::handleDownKeyPress()
{
    if(mCurrentHistoryItem == -1) { return; }

    this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor );
    this->textCursor().removeSelectedText();

    setOutputColor("default");
    --mCurrentHistoryItem;
    if(mCurrentHistoryItem == -1)
    {
        this->insertPlainText(">> ");
    }
    else
    {
        this->insertPlainText(">> " + mHistory.at(mCurrentHistoryItem));
    }

    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}


void TerminalConsole::handleTabKeyPress()
{
    ++mCurrentAutoCompleteIndex;
    if(mCurrentAutoCompleteIndex > mAutoCompleteResults.size()-1)
    {
        mCurrentAutoCompleteIndex = 0;
    }

    if(mAutoCompleteResults.isEmpty())
    {
        mAutoCompleteFilter = this->document()->lastBlock().text();
        mAutoCompleteFilter = mAutoCompleteFilter.right(mAutoCompleteFilter.size()-3);

        QStringList availableCommands = getHandler()->getCommands();
        for(int c=0; c<availableCommands.size(); ++c)
        {
            if(availableCommands[c].startsWith(mAutoCompleteFilter))
            {
                mAutoCompleteResults.append(availableCommands[c]);
            }
        }

        QStringList variableCmds = QStringList() << "disp " << "chpv " << "chpvr " << "chpvl " << "peek " << "poke " << "alias ";
        for(int c=0; c<variableCmds.size(); ++c)
        {
            if(mAutoCompleteFilter.startsWith(variableCmds[c]))
            {
                QStringList variables;
                getHandler()->getVariables(mAutoCompleteFilter.right(mAutoCompleteFilter.size()-variableCmds[c].size()),variables);
                for(int v=0; v<variables.size(); ++v)
                {
                    variables[v].prepend(variableCmds[c]);
                }
                mAutoCompleteResults.append(variables);
            }
        }

        QStringList parameterCmds = QStringList() << "chpa " << "dipa ";
        for(int c=0; c<parameterCmds.size(); ++c)
        {
            if(mAutoCompleteFilter.startsWith(parameterCmds[c]))
            {
                QStringList parameters;
                getHandler()->getParameters(mAutoCompleteFilter.right(mAutoCompleteFilter.size()-parameterCmds[c].size()),parameters);
                for(int v=0; v<parameters.size(); ++v)
                {
                    parameters[v].prepend(parameterCmds[c]);
                }
                mAutoCompleteResults.append(parameters);
            }
        }

        QStringList directoryCmds = QStringList() << "cd ";
        for(int c=0; c<directoryCmds.size(); ++c)
        {
            if(mAutoCompleteFilter.startsWith(directoryCmds[c]))
            {
                QString pwd = getHandler()->getWorkingDirectory();
                QString dir = pwd;
                QString filter = mAutoCompleteFilter.right(mAutoCompleteFilter.size()-directoryCmds[c].size());
                dir.append("/"+filter);

                QDir parentDir = QDir(dir.left(dir.lastIndexOf("/")));
                QString parentDirStr = parentDir.path();
                QString lastFilterDir = filter.right(filter.size()-filter.lastIndexOf("/")-1);
                QString leftFilterDir = "";
                if(filter.contains("/"))
                {
                    leftFilterDir = filter.left(filter.lastIndexOf("/"))+"/";
                }
                QStringList subDirs = parentDir.entryList(QStringList() << lastFilterDir+"*", QDir::Dirs | QDir::NoDotAndDotDot);

                for(int d=0; d<subDirs.size(); ++d)
                {
                    subDirs[d].prepend(leftFilterDir);
                    mAutoCompleteResults.append(directoryCmds[c]+subDirs[d]);
                }
            }
        }

        mCurrentAutoCompleteIndex = 0;
    }

    if(mAutoCompleteResults.isEmpty()) { return; }

    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    this->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    this->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
    this->textCursor().removeSelectedText();

    QString autoString = mAutoCompleteResults.at(mCurrentAutoCompleteIndex);
    this->setOutputColor("default");
    this->insertPlainText(">> "+autoString);

    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}


void TerminalConsole::handleHomeKeyPress()
{
    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    this->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    this->moveCursor(QTextCursor::NextWord, QTextCursor::MoveAnchor);
}


void TerminalConsole::cancelAutoComplete()
{
    mAutoCompleteFilter.clear();
    mAutoCompleteResults.clear();
}

void TerminalConsole::cancelRecentHistory()
{
    mCurrentHistoryItem=-1;
}



HcomHandler::HcomHandler(TerminalConsole *pConsole)
{
    mpConsole = pConsole;

    mCurrentPlotWindowName = "PlotWindow0";

    mPwd = QString(DOCUMENTSPATH);
    mPwd.chop(1);

    HcomCommand helpCmd;
    helpCmd.cmd = "help";
    helpCmd.description.append("Shows help information.");
    helpCmd.help.append("Usage: help [command]");
    helpCmd.fnc = &HcomHandler::executeHelpCommand;
    mCmdList << helpCmd;

    HcomCommand simCmd;
    simCmd.cmd = "sim";
    simCmd.description.append("Simulates current model.");
    simCmd.help.append("Usage: sim [no arguments]");
    simCmd.fnc = &HcomHandler::executeSimulateCommand;
    mCmdList << simCmd;

    HcomCommand chpvCmd;
    chpvCmd.cmd = "chpv";
    chpvCmd.description.append("Change plot variables in current plot.");
    chpvCmd.help.append("Usage: chpv [leftvar1 [leftvar2] ... [-r rightvar1 rightvar2 ... ]]");
    chpvCmd.fnc = &HcomHandler::executePlotCommand;
    mCmdList << chpvCmd;

    HcomCommand exitCmd;
    exitCmd.cmd = "exit";
    exitCmd.description.append("Exits the program.");
    exitCmd.help.append("Usage: exit [no arguments]");
    exitCmd.fnc = &HcomHandler::executeExitCommand;
    mCmdList << exitCmd;

    HcomCommand dipaCmd;
    dipaCmd.cmd = "dipa";
    dipaCmd.description.append("Display parameter value.");
    dipaCmd.help.append("Usage: dipa [parameter]");
    dipaCmd.fnc = &HcomHandler::executeDisplayParameterCommand;
    mCmdList << dipaCmd;

    HcomCommand chpaCmd;
    chpaCmd.cmd = "chpa";
    chpaCmd.description.append("Change parameter value.");
    chpaCmd.help.append("Usage: chpa [parameter value]");
    chpaCmd.fnc = &HcomHandler::executeChangeParameterCommand;
    mCmdList << chpaCmd;

    HcomCommand chssCmd;
    chssCmd.cmd = "chss";
    chssCmd.description.append("Change simulation settings.");
    chssCmd.help.append("Usage: chss [starttime timestep stoptime [samples]]");
    chssCmd.fnc = &HcomHandler::executeChangeSimulationSettingsCommand;
    mCmdList << chssCmd;

    HcomCommand execCmd;
    execCmd.cmd = "exec";
    execCmd.description.append("Executes a script file");
    execCmd.help.append("Usage: exec [filepath]");
    execCmd.fnc = &HcomHandler::executeRunScriptCommand;
    mCmdList << execCmd;

    HcomCommand wrhiCmd;
    wrhiCmd.cmd = "wrhi";
    wrhiCmd.description.append("Writes history to file.");
    wrhiCmd.help.append("Usage: wrhi [filepath]");
    wrhiCmd.fnc = &HcomHandler::executeWriteHistoryToFileCommand;
    mCmdList << wrhiCmd;

    HcomCommand printCmd;
    printCmd.cmd = "print";
    printCmd.description.append("Prints arguments on the screen.");
    printCmd.help.append("Usage: print [\"Text\" (variable)]\n");
    printCmd.help.append("Note: Not implemented yet.");
    printCmd.fnc = &HcomHandler::executePrintCommand;
    mCmdList << printCmd;

    HcomCommand chpwCmd;
    chpwCmd.cmd = "chpw";
    chpwCmd.description.append("Changes current plot window.");
    chpwCmd.help.append("Usage: chpw [number]");
    chpwCmd.fnc = &HcomHandler::executeChangePlotWindowCommand;
    mCmdList << chpwCmd;

    HcomCommand dipwCmd;
    dipwCmd.cmd = "dipw";
    dipwCmd.description.append("Displays current plot window.");
    dipwCmd.help.append(" Usage: dipw [no arguments]");
    dipwCmd.fnc = &HcomHandler::executeDisplayPlotWindowCommand;
    mCmdList << dipwCmd;

    HcomCommand chpvlCmd;
    chpvlCmd.cmd = "chpvl";
    chpvlCmd.description.append("Changes plot variables on left axis in current plot.");
    chpvlCmd.help.append(" Usage: chpvl [var1 var2 ... ]");
    chpvlCmd.fnc = &HcomHandler::executePlotLeftAxisCommand;
    mCmdList << chpvlCmd;

    HcomCommand chpvrCmd;
    chpvrCmd.cmd = "chpvr";
    chpvrCmd.description.append("Changes plot variables on right axis in current plot.");
    chpvrCmd.help.append(" Usage: chpvr [var1 var2 ... ]");
    chpvrCmd.fnc = &HcomHandler::executePlotRightAxisCommand;
    mCmdList << chpvrCmd;

    HcomCommand dispCmd;
    dispCmd.cmd = "disp";
    dispCmd.description.append("Shows a list of all variables matching specified name filter (using asterisks).");
    dispCmd.help.append("Usage: disp [filter]");
    dispCmd.fnc = &HcomHandler::executeDisplayVariablesCommand;
    mCmdList << dispCmd;

    HcomCommand peekCmd;
    peekCmd.cmd = "peek";
    peekCmd.description.append("Shows the value at a specified index in a specified data variable.");
    peekCmd.help.append("Usage: peek [variable index]");
    peekCmd.fnc = &HcomHandler::executePeekCommand;
    mCmdList << peekCmd;

    HcomCommand pokeCmd;
    pokeCmd.cmd = "poke";
    pokeCmd.description.append("Changes the value at a specified index in a specified data variable.");
    pokeCmd.help.append("Usage: poke [variable index newvalue]");
    pokeCmd.fnc = &HcomHandler::executePokeCommand;
    mCmdList << pokeCmd;

    HcomCommand aliasCmd;
    aliasCmd.cmd = "alias";
    aliasCmd.description.append("Defines an alias for a variable.");
    aliasCmd.help.append("Usage: alias [variable alias]");
    aliasCmd.fnc = &HcomHandler::executeDefineAliasCommand;
    mCmdList << aliasCmd;

    HcomCommand setCmd;
    setCmd.cmd = "set";
    setCmd.description.append("Sets Hopsan preferences.");
    setCmd.help.append(" Usage: set [preference value]\n");
    setCmd.help.append(" Available commands:\n");
    setCmd.help.append("  multicore [on/off]\n");
    setCmd.help.append("  threads [number]");
    setCmd.fnc = &HcomHandler::executeSetCommand;
    mCmdList << setCmd;

    HcomCommand saplCmd;
    saplCmd.cmd = "sapl";
    saplCmd.description.append("Saves plot file to .PLO");
    saplCmd.help.append("Usage: sapl [filepath variables]");
    saplCmd.fnc = &HcomHandler::executeSaveToPloCommand;
    mCmdList << saplCmd;

    HcomCommand loadCmd;
    loadCmd.cmd = "load";
    loadCmd.description.append("Loads a model file.");
    loadCmd.help.append("Usage: load [filepath variables]");
    loadCmd.fnc = &HcomHandler::executeLoadModelCommand;
    mCmdList << loadCmd;

    HcomCommand loadrCmd;
    loadrCmd.cmd = "loadr";
    loadrCmd.description.append("Loads most recent model file.");
    loadrCmd.help.append("Usage: loadr [no arguments]");
    loadrCmd.fnc = &HcomHandler::executeLoadRecentCommand;
    mCmdList << loadrCmd;

    HcomCommand pwdCmd;
    pwdCmd.cmd = "pwd";
    pwdCmd.description.append("Displays present working directory.");
    pwdCmd.help.append("Usage: pwd [no arguments]");
    pwdCmd.fnc = &HcomHandler::executePwdCommand;
    mCmdList << pwdCmd;

    HcomCommand cdCmd;
    cdCmd.cmd = "cd";
    cdCmd.description.append("Changes present working directory.");
    cdCmd.help.append(" Usage: cd [directory]");
    cdCmd.fnc = &HcomHandler::executeChangeDirectoryCommand;
    mCmdList << cdCmd;

    HcomCommand lsCmd;
    lsCmd.cmd = "ls";
    lsCmd.description.append("List files in current directory.");
    lsCmd.help.append("Usage: ls [no arguments]");
    lsCmd.fnc = &HcomHandler::executeListFilesCommand;
    mCmdList << lsCmd;

    HcomCommand closeCmd;
    closeCmd.cmd = "close";
    closeCmd.description.append("Closes current model.");
    closeCmd.help.append("Usage: close [no arguments]");
    closeCmd.fnc = &HcomHandler::executeCloseModelCommand;
    mCmdList << closeCmd;

    HcomCommand chtabCmd;
    chtabCmd.cmd = "chtab";
    chtabCmd.description.append("Changes current model tab.");
    chtabCmd.help.append("Usage: chtab [index]");
    chtabCmd.fnc = &HcomHandler::executeChangeTabCommand;
    mCmdList << chtabCmd;

    HcomCommand adcoCmd;
    adcoCmd.cmd = "adco";
    adcoCmd.description.append("Adds a new component to current model.");
    adcoCmd.help.append(" Usage: adco [typename name -flag value]");
    adcoCmd.fnc = &HcomHandler::executeAddComponentCommand;
    mCmdList << adcoCmd;

    HcomCommand cocoCmd;
    cocoCmd.cmd = "coco";
    cocoCmd.description.append("Connect components in current model.");
    cocoCmd.help.append("Usage: coco [comp1 port1 comp2 port2]");
    cocoCmd.fnc = &HcomHandler::executeConnectCommand;
    mCmdList << cocoCmd;

    HcomCommand crmoCmd;
    crmoCmd.cmd = "crmo";
    crmoCmd.description.append("Creates a new model.");
    crmoCmd.help.append("Usage: crmo [no arguments]");
    crmoCmd.fnc = &HcomHandler::executeCreateModelCommand;
    mCmdList << crmoCmd;

    HcomCommand fmuCmd;
    fmuCmd.cmd = "fmu";
    fmuCmd.description.append("Exports current model to Functional Mockup Unit (FMU)");
    fmuCmd.help.append("Usage: fmu [path]");
    fmuCmd.fnc = &HcomHandler::executeExportToFMUCommand;
    mCmdList << fmuCmd;
}


//! @brief Returns a list of available commands
QStringList HcomHandler::getCommands()
{
    QStringList ret;
    for(int i=0; i<mCmdList.size(); ++i)
    {
        ret.append(mCmdList.at(i).cmd);
    }
    return ret;
}


void HcomHandler::executeCommand(QString cmd)
{
    cmd = cmd.simplified();

    QString majorCmd = cmd.split(" ").first();
    QString subCmd;
    if(cmd.split(" ").size() == 1)
    {
        subCmd = QString();
    }
    else
    {
        subCmd = cmd.right(cmd.size()-majorCmd.size()-1);
    }

    int idx = -1;
    for(int i=0; i<mCmdList.size(); ++i)
    {
        if(mCmdList[i].cmd == majorCmd) { idx = i; }
    }

    ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
    SystemContainer *pCurrentSystem;
    if(pCurrentTab)
    {
        pCurrentSystem = pCurrentTab->getTopLevelSystem();
    }
    else
    {
        pCurrentSystem = 0;
    }

    if(idx<0)
    {
        if(evaluateArithmeticExpression(cmd)) { return; }
        mpConsole->printErrorMessage("Unrecognized command: " + majorCmd, "", false);
    }
    else
    {
        mCmdList[idx].runCommand(subCmd, this);
    }
}



void HcomHandler::executeExitCommand(QString cmd)
{
    gpMainWindow->close();
}


void HcomHandler::executeSimulateCommand(QString cmd)
{
    ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
    if(pCurrentTab)
    {
        pCurrentTab->simulate_blocking();
    }
}


void HcomHandler::executePlotCommand(QString cmd)
{
    changePlotVariables(cmd, -1);
}

void HcomHandler::executePlotLeftAxisCommand(QString cmd)
{
    changePlotVariables(cmd, 0);
}

void HcomHandler::executePlotRightAxisCommand(QString cmd)
{
    changePlotVariables(cmd, 1);
}

void HcomHandler::executeDisplayParameterCommand(QString cmd)
{
    QStringList parameters;
    getParameters(cmd, parameters);

    int longestParameterName=0;
    for(int p=0; p<parameters.size(); ++p)
    {
        if(parameters.at(p).size() > longestParameterName)
        {
            longestParameterName = parameters.at(p).size();
        }
    }


    for(int p=0; p<parameters.size(); ++p)
    {
        QString output = parameters[p];
        int space = longestParameterName-parameters[p].size()+3;
        for(int s=0; s<space; ++s)
        {
            output.append(" ");
        }
        output.append(getParameterValue(parameters[p]));
        mpConsole->print(output);
    }





//    cmd.remove("\"");
//    QStringList splitCmd = cmd.split(".");
//    if(splitCmd.size() == 2)
//    {
//        SystemContainer *pCurrentSystem = gpMainWindow->mpProjectTabs->getCurrentTab()->getTopLevelSystem();
//        if(!pCurrentSystem) { return; }
//        ModelObject *pModelObject = pCurrentSystem->getModelObject(splitCmd[0]);
//        if(!pModelObject) { return; }
//        QString parameterValue = pModelObject->getParameterValue(splitCmd[1]);
//        if(parameterValue.isEmpty())
//        {
//            parameterValue = "Parameter " + splitCmd[1] + " not found.";
//        }
//        mpConsole->print(parameterValue);
//    }
//    else
//    {
//        mpConsole->printErrorMessage("Wrong number of arguments.","",false);
//    }
}


void HcomHandler::executeChangeParameterCommand(QString cmd)
{
    QStringList splitCmd;
    bool withinQuotations = false;
    int start=0;
    for(int i=0; i<cmd.size(); ++i)
    {
        if(cmd[i] == '\"')
        {
            withinQuotations = !withinQuotations;
        }
        if(cmd[i] == ' ' && !withinQuotations)
        {
            splitCmd.append(cmd.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmd.append(cmd.right(cmd.size()-start));
//    for(int i=0; i<splitCmd.size(); ++i)
//    {
//        splitCmd[i].remove("\"");
//    }

    if(splitCmd.size() == 2)
    {
        ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
        if(!pCurrentTab) { return; }
        SystemContainer *pSystem = pCurrentTab->getTopLevelSystem();
        if(!pSystem) { return; }

        QStringList parameterNames;
        getParameters(splitCmd[0], parameterNames);
        QString newValue = splitCmd[1];

        for(int p=0; p<parameterNames.size(); ++p)
        {
            if(pSystem->getParameterNames().contains(parameterNames[p]))
            {
                pSystem->setParameterValue(parameterNames[p], newValue);
            }
            else
            {
                parameterNames[p].remove("\"");
                QStringList splitFirstCmd = parameterNames[p].split(".");
                if(splitFirstCmd.size() == 2)
                {
                    QList<ModelObject*> components;
                    getComponents(splitFirstCmd[0], components);
                    for(int c=0; c<components.size(); ++c)
                    {
                        QStringList parameters;
                        getParameters(splitFirstCmd[1], components[c], parameters);
                        for(int p=0; p<parameters.size(); ++p)
                        {
                            bool ok;
                            VariableType varType;
                            components[c]->setParameterValue(parameters[p], evaluateExpression(newValue, &varType, &ok));
                        }
                    }
                }
            }
        }
    }
    else
    {
        mpConsole->printErrorMessage("Wrong number of arguments.","",false);
    }
}


//Change Simulation Settings
//Usage: CHSS [starttime] [timestep] [stoptime] ([samples])
void HcomHandler::executeChangeSimulationSettingsCommand(QString cmd)
{
    cmd.remove("\"");
    QStringList splitCmd = cmd.split(" ");
    if(splitCmd.size() == 3 || splitCmd.size() == 4)
    {
        bool allOk=true;
        bool ok;
        double startT = splitCmd[0].toDouble(&ok);
        if(!ok) { allOk=false; }
        double timeStep = splitCmd[1].toDouble(&ok);
        if(!ok) { allOk=false; }
        double stopT = splitCmd[2].toDouble(&ok);
        if(!ok) { allOk=false; }

        int samples;
        if(splitCmd.size() == 4)
        {
            samples = splitCmd[3].toInt(&ok);
            if(!ok) { allOk=false; }
        }

        if(allOk)
        {
            gpMainWindow->setStartTimeInToolBar(startT);
            gpMainWindow->setTimeStepInToolBar(timeStep);
            gpMainWindow->setStopTimeInToolBar(stopT);
            if(splitCmd.size() == 4)
            {
                ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
                if(!pCurrentTab) { return; }
                SystemContainer *pCurrentSystem = pCurrentTab->getTopLevelSystem();
                if(!pCurrentSystem) { return; }
                pCurrentSystem->setNumberOfLogSamples(samples);
            }
        }
        else
        {
            mpConsole->print("Failed to apply simulation settings.");
        }
    }
    else
    {
        mpConsole->print("Wrong number of arguments.");
    }
}


//! @brief Executes the "help" command, showing help to the user
//! @param cmd Command with first word removed
void HcomHandler::executeHelpCommand(QString cmd)
{
    cmd.remove(" ");
    if(cmd.isEmpty())
    {
        mpConsole->print("-------------------------------------------------------------------------");
        mpConsole->print(" Hopsan HCOM Terminal v0.1\n");
        mpConsole->print(" Available commands:\n");
        QString commands;
        int n=0;
        for(int c=0; c<mCmdList.size(); ++c)
        {
            n=max(mCmdList[c].cmd.size(), n);
        }
        for(int c=0; c<mCmdList.size(); ++c)
        {
            commands.append(" ");
            commands.append(mCmdList[c].cmd);
            for(int i=0; i<n+3-mCmdList[c].cmd.size(); ++i)
            {
                commands.append(" ");
            }
            commands.append(mCmdList[c].description);
            commands.append("\n");
        }
        mpConsole->print(commands);
        mpConsole->print(" Type: \"help [command]\" for more information about a specific command.");
        mpConsole->print("-------------------------------------------------------------------------");
    }
    else
    {
        int idx = -1;
        for(int i=0; i<mCmdList.size(); ++i)
        {
            if(mCmdList[i].cmd == cmd) { idx = i; }
        }

        if(idx < 0)
        {
            mpConsole->print("No help available for this command.");
        }
        else
        {
            int length=max(mCmdList[idx].description.size(), mCmdList[idx].help.size())+2;
            QString delimiterLine;
            for(int i=0; i<length; ++i)
            {
                delimiterLine.append("-");
            }
            QString descLine = mCmdList[idx].description;
            descLine.prepend(" ");
            QString helpLine = mCmdList[idx].help;
            helpLine.prepend(" ");
            helpLine.replace("\n", "\n ");
            mpConsole->print(delimiterLine+"\n"+descLine+"\n"+helpLine+"\n"+delimiterLine);
        }
    }
}


//! @brief Executes the "exec" command, which excutes a script file
//! @param cmd Command with first word removed
void HcomHandler::executeRunScriptCommand(QString cmd)
{
    QStringList splitCmd = cmd.split(" ");

    if(splitCmd.isEmpty()) { return; }

    QString path = splitCmd[0];
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        mpConsole->print("Unable to read file.");
        return;
    }

    QString code;
    QTextStream t(&file);

    code = t.readAll();

    for(int i=0; i<splitCmd.size()-1; ++i)  //Replace arguments with their values
    {
        QString str = "$"+QString::number(i+1);
        code.replace(str, splitCmd[i+1]);
    }

    QStringList lines = code.split("\n");
    lines.removeAll("");
    QString gotoLabel = runScriptCommands(lines);
    while(!gotoLabel.isEmpty())
    {
        if(gotoLabel == "%%%%%EOF")
        {
            break;
        }
        for(int l=0; l<lines.size(); ++l)
        {
            if(lines[l].startsWith("&"+gotoLabel))
            {
                gotoLabel = runScriptCommands(lines.mid(l, lines.size()-l));
            }
        }
    }

    file.close();
}


//! @brief Executes the "wrhi" command, that writes history to specified file
//! @param cmd Command with fist word removed
void HcomHandler::executeWriteHistoryToFileCommand(QString cmd)
{
    if(cmd.isEmpty()) { return; }

    QString path = cmd;
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));


    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        mpConsole->print("Unable to write to file.");
        return;
    }

    QTextStream t(&file);
    for(int h=mpConsole->mHistory.size()-1; h>-1; --h)
    {
        t << mpConsole->mHistory[h] << "\n";
    }
    file.close();
}


//! @brief Executes print command
//! @todo Implement
void HcomHandler::executePrintCommand(QString cmd)
{
    mpConsole->print("Function not yet implemented.");
}


//! @brief Executes the chpw command, changing current plot window
//! @param cmd Command with first word removed
void HcomHandler::executeChangePlotWindowCommand(QString cmd)
{
    mCurrentPlotWindowName = cmd;
}


//! @brief Executes the dipw command, displaying current plot window
//! @param cmd Command with first word removed
void HcomHandler::executeDisplayPlotWindowCommand(QString cmd)
{
    mpConsole->print(mCurrentPlotWindowName);
}


//! @brief Executes the "disp" command, displaying all variables matching a filter pattern
//! @param cmd Name filter
void HcomHandler::executeDisplayVariablesCommand(QString cmd)
{
    QStringList output;
    getVariables(cmd, output);

    for(int o=0; o<output.size(); ++o)
    {
        mpConsole->print(output[o]);
    }
}


//! @brief Executes the "peek" command, allowing user to read the value at a position in a data vector
//! @param cmd Command (with the first command word already removed)
void HcomHandler::executePeekCommand(QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() != 2)
    {
        mpConsole->print("Wrong number of arguments.");
        return;
    }

    QString variable = split.first();
    bool ok;
    int id = getNumber(split.last(), &ok);
    if(!ok)
    {
        mpConsole->print("Illegal value.");
        return;
    }

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
        QString err;
        double r = pData->peekData(id, err);
        if (err.isEmpty())
        {
            mpConsole->print(QString::number(r));
        }
        else
        {
            mpConsole->print(err);
        }
    }
    else
    {
        mpConsole->print("Data variable not found");
    }
}


//! @brief Executes the "poke" command, allowing user to write a new value at a position in a data vector
//! @param cmd Command (with the first command word already removed)
void HcomHandler::executePokeCommand(QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() != 3)
    {
        mpConsole->print("Wrong number of arguments.");
        return;
    }

    QString variable = split.first();
    bool ok1, ok2;
    int id = getNumber(split[1], &ok1);
    double value = getNumber(split.last(), &ok2);
    if(!ok1 || !ok2)
    {
        mpConsole->print("Illegal value.");
        return;
    }

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
        QString err;
        double r = pData->pokeData(id, value, err);
        if (err.isEmpty())
        {
            mpConsole->print(QString::number(r));
        }
        else
        {
            mpConsole->print(err);
        }
    }
    else
    {
        mpConsole->print("Data variable not found.");
    }
    return;
}


void HcomHandler::executeDefineAliasCommand(QString cmd)
{
    if(splitWithRespectToQuotations(cmd, ' ').size() != 2)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    QString variable = splitWithRespectToQuotations(cmd, ' ')[0];
    toShortDataNames(variable);
    variable.remove("\"");
    QString alias = splitWithRespectToQuotations(cmd, ' ')[1];

    SharedLogVariableDataPtrT pVariable = getVariablePtr(variable);

    if(!pVariable || !gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->definePlotAlias(alias, pVariable->getFullVariableName()))
    {
        mpConsole->print("Failed to assign variable alias.");
    }

    gpMainWindow->mpPlotWidget->mpPlotVariableTree->updateList();

    return;
}


void HcomHandler::executeSetCommand(QString cmd)
{
    QStringList splitCmd = cmd.split(" ");
    if(splitCmd.size() != 2)
    {
        mpConsole->print("Wrong number of arguments.");
        return;
    }
    QString pref = splitCmd[0];
    QString value = splitCmd[1];

    if(pref == "multicore")
    {
        if(value != "on" && value != "off")
        {
            mpConsole->print("Unknown value.");
            return;
        }
        gConfig.setUseMultiCore(value=="on");
    }
    else if(pref == "threads")
    {
        bool ok;
        int nThreads = value.toInt(&ok);
        if(!ok)
        {
            mpConsole->print("Unknown value.");
            return;
        }
        gConfig.setNumberOfThreads(nThreads);
    }
}


void HcomHandler::executeSaveToPloCommand(QString cmd)
{
    if(!cmd.contains(" "))
    {
        mpConsole->printErrorMessage("Too few arguments.", "", false);
        return;
    }
    QString path = cmd.split(" ").first();

    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));


    cmd = cmd.right(cmd.size()-path.size()-1);

    QStringList splitCmdMajor;
    bool withinQuotations = false;
    int start=0;
    for(int i=0; i<cmd.size(); ++i)
    {
        if(cmd[i] == '\"')
        {
            withinQuotations = !withinQuotations;
        }
        if(cmd[i] == ' ' && !withinQuotations)
        {
            splitCmdMajor.append(cmd.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmdMajor.append(cmd.right(cmd.size()-start));
    QStringList allVariables;
    for(int i=0; i<splitCmdMajor.size(); ++i)
    {
        splitCmdMajor[i].remove("\"");
        QStringList variables;
        getVariables(splitCmdMajor[i], variables);
        for(int v=0; v<variables.size(); ++v)
        {
            variables[v].replace(".", "#");
            variables[v].remove("\"");

            if(variables[v].endsWith("#x"))
            {
                variables[v].chop(2);
                variables[v].append("#Position");
            }
            else if(variables[v].endsWith("#v"))
            {
                variables[v].chop(2);
                variables[v].append("#Velocity");
            }
            else if(variables[v].endsWith("#f"))
            {
                variables[v].chop(2);
                variables[v].append("#Force");
            }
            else if(variables[v].endsWith("#p"))
            {
                variables[v].chop(2);
                variables[v].append("#Pressure");
            }
            else if(variables[v].endsWith("#q"))
            {
                variables[v].chop(2);
                variables[v].append("#Flow");
            }
            else if(variables[v].endsWith("#val"))
            {
                variables[v].chop(4);
                variables[v].append("#Value");
            }
            else if(variables[v].endsWith("#Zc"))
            {
                variables[v].chop(3);
                variables[v].append("#CharImp");
            }
            else if(variables[v].endsWith("#c"))
            {
                variables[v].chop(2);
                variables[v].append("#WaveVariable");
            }
            else if(variables[v].endsWith("#me"))
            {
                variables[v].chop(3);
                variables[v].append("#EquivalentMass");
            }
            else if(variables[v].endsWith("#Q"))
            {
                variables[v].chop(2);
                variables[v].append("#HeatFlow");
            }
            else if(variables[v].endsWith("#t"))
            {
                variables[v].chop(2);
                variables[v].append("#Temperature");
            }

        }
        allVariables.append(variables);
        //splitCmdMajor[i] = getVariablePtr(splitCmdMajor[i])->getFullVariableName();
    }

    gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->exportToPlo(path, allVariables);
}


void HcomHandler::executeLoadModelCommand(QString cmd)
{
    QString path = cmd;
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    gpMainWindow->mpProjectTabs->loadModel(path);
}


void HcomHandler::executeLoadRecentCommand(QString /*cmd*/)
{
    gpMainWindow->mpProjectTabs->loadModel(gConfig.getRecentModels().first());
}

void HcomHandler::executePwdCommand(QString cmd)
{
    mpConsole->print(mPwd);
}

void HcomHandler::executeChangeDirectoryCommand(QString cmd)
{
    mPwd = QDir().cleanPath(mPwd+"/"+cmd);
    mpConsole->print(mPwd);
}

void HcomHandler::executeListFilesCommand(QString cmd)
{
    if(cmd.isEmpty())
    {
        cmd = "*";
    }
    QStringList contents = QDir(mPwd).entryList(QStringList() << cmd);
    for(int c=0; c<contents.size(); ++c)
    {
        mpConsole->print(contents[c]);
    }
}

void HcomHandler::executeCloseModelCommand(QString cmd)
{
    if(gpMainWindow->mpProjectTabs->count() > 0)
    {
        gpMainWindow->mpProjectTabs->closeProjectTab(gpMainWindow->mpProjectTabs->currentIndex());
    }
}


void HcomHandler::executeChangeTabCommand(QString cmd)
{
    gpMainWindow->mpProjectTabs->setCurrentIndex(cmd.toInt());
}


void HcomHandler::executeAddComponentCommand(QString cmd)
{
    QStringList args = cmd.split(" ");
    if(args.size() < 5)
    {
        mpConsole->printErrorMessage("Too few arguments.", "", false);
        return;
    }
    QString typeName = args[0];
    QString name = args[1];
    args.removeFirst();
    args.removeFirst();

    double xPos;
    double yPos;
    double rot;

    if(!args.isEmpty())
    {
        if(args.first() == "-a")
        {
            //Absolute
            if(args.size() != 4)
            {
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            xPos = args[1].toDouble();
            yPos = args[2].toDouble();
            rot = args[3].toDouble();
        }
        else if(args.first() == "-e")
        {
            //East of
            if(args.size() != 4)
            {
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x()+offset;
            yPos = pOther->getCenterPos().y();
            rot = args[3].toDouble();
        }
        else if(args.first() == "-w")
        {
            //West of
            if(args.size() != 4)
            {
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x()-offset;
            yPos = pOther->getCenterPos().y();
            rot = args[3].toDouble();
        }
        else if(args.first() == "-n")
        {
            //North of
            if(args.size() != 4)
            {
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x();
            yPos = pOther->getCenterPos().y()-offset;
            rot = args[3].toDouble();
        }
        else if(args.first() == "-s")
        {
            //South of
            if(args.size() != 4)
            {
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x();
            yPos = pOther->getCenterPos().y()+offset;
            rot = args[3].toDouble();
        }
    }

    QPointF pos = QPointF(xPos, yPos);
    Component *pObj = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->addModelObject(typeName, pos, rot));
    if(!pObj)
    {
        mpConsole->printErrorMessage("Failed to add new component. Incorrect typename?", "", false);
    }
    else
    {
        mpConsole->print("Added "+typeName+" to current model.");
        gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->renameModelObject(pObj->getName(), name);
    }
}


void HcomHandler::executeConnectCommand(QString cmd)
{
    QStringList args = cmd.split(" ");
    if(args.size() != 4)
    {
        mpConsole->printErrorMessage("Wrong number of arguments", "", false);
        return;
    }

    Port *pPort1 = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(args[0])->getPort(args[1]);
    Port *pPort2 = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(args[2])->getPort(args[3]);

    Connector *pConn = gpMainWindow->mpProjectTabs->getCurrentContainer()->createConnector(pPort1, pPort2);

    if (pConn != 0)
    {
        QVector<QPointF> pointVector;
        pointVector.append(pPort1->pos());
        pointVector.append(pPort2->pos());

        QStringList geometryList;
        geometryList.append("diagonal");

        pConn->setPointsAndGeometries(pointVector, geometryList);
        pConn->refreshConnectorAppearance();
    }
}


void HcomHandler::executeCreateModelCommand(QString cmd)
{
    gpMainWindow->mpProjectTabs->addNewProjectTab();
}


void HcomHandler::executeExportToFMUCommand(QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.");
    }

    gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->exportToFMU(getArgument(cmd, 0));
}


//! @brief Changes plot variables on specified axes
//! @param cmd Command containing the plot variables
//! @param axis Axis specification (0=left, 1=right, -1=both, separeted by "-r")
void HcomHandler::changePlotVariables(QString cmd, int axis)
{
    QStringList splitCmdMajor;
    bool withinQuotations = false;
    int start=0;
    for(int i=0; i<cmd.size(); ++i)
    {
        if(cmd[i] == '\"')
        {
            withinQuotations = !withinQuotations;
        }
        if(cmd[i] == ' ' && !withinQuotations)
        {
            splitCmdMajor.append(cmd.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmdMajor.append(cmd.right(cmd.size()-start));

    if(axis == -1 || axis == 0)
    {
        removePlotCurves(QwtPlot::yLeft);
    }
    if(axis == -1 || axis == 1)
    {
        removePlotCurves(QwtPlot::yRight);
    }

    int axisId;
    if(axis == -1 || axis == 0)
    {
        axisId = QwtPlot::yLeft;
    }
    else
    {
        axisId = QwtPlot::yRight;
    }
    for(int s=0; s<splitCmdMajor.size(); ++s)
    {
        if(axis == -1 && splitCmdMajor[s] == "-r")
        {
            axisId = QwtPlot::yRight;
        }
        else
        {
            QStringList variables;
            getVariables(splitCmdMajor[s], variables);
            for(int v=0; v<variables.size(); ++v)
            {
                addPlotCurve(variables[v], axisId);
            }
        }
    }
}



//! @brief Adds a plot curve to specified axis in current plot
//! @param cmd Name of variable
//! @param axis Axis to add curve to
void HcomHandler::addPlotCurve(QString cmd, int axis)
{
    cmd.remove("\"");
    //QStringList splitCmd = cmd.split(".");

    SystemContainer *pCurrentSystem = gpMainWindow->mpProjectTabs->getCurrentTab()->getTopLevelSystem();
    if(!pCurrentSystem) { return; }

    SharedLogVariableDataPtrT pData = getVariablePtr(cmd);
    if(!pData)
    {
        mpConsole->print("Variable not found.");
        return;
    }

    gpPlotHandler->plotDataToWindow(mCurrentPlotWindowName, pData, axis);
}


//! @brief Removes all curves at specified axis in current plot
//! @param axis Axis to remove from
void HcomHandler::removePlotCurves(int axis)
{
    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow(mCurrentPlotWindowName);
    if(pPlotWindow)
    {
        pPlotWindow->getCurrentPlotTab()->removeAllCurvesOnAxis(axis);
    }
}


QString HcomHandler::evaluateExpression(QString expr, VariableType *returnType, bool *evalOk)
{
    *evalOk = true;
    *returnType = Scalar;

    //expr.replace("==", "§§§§§");
    expr.replace("**", "%%%%%");

    //Remove parentheses around expression
    QString tempStr = expr.mid(1, expr.size()-2);
    if(expr.count("(") == 1 && expr.count(")") == 1 && expr.startsWith("(") && expr.endsWith(")"))
    {
        expr = tempStr;
    }
    else if(expr.count("(") > 1 && expr.count(")") > 1 && expr.startsWith("(") && expr.endsWith(")"))
    {

        if(tempStr.indexOf("(") < tempStr.indexOf(")"))
        {
            expr = tempStr;
        }
    }

    //Numerical value, return it
    bool ok;
    expr.toDouble(&ok);
    if(ok)
    {
        return expr;
    }

    //Pre-defined variable, return its value
    if(mLocalVars.contains(expr))
    {
        return QString::number(mLocalVars.find(expr).value());
    }

    //Parameter name, return its value
    if(getParameterValue(expr) != "NaN")
    {
        return getParameterValue(expr);
    }

    //Data variable, return it
    QStringList variables;
    getVariables(expr,variables);
    if(!variables.isEmpty())
    {
        *returnType = DataVector;
        return variables.first();
    }

    //Evaluate expression using SymHop
    SymHop::Expression symHopExpr = SymHop::Expression(expr);

    //Multiplication between data vector and scalar
    LogDataHandler *pLogData = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler();
    *returnType = DataVector;
    if(symHopExpr.getFactors().size() == 2)
    {
        SymHop::Expression f0 = symHopExpr.getFactors()[0];
        SymHop::Expression f1 = symHopExpr.getFactors()[1];
        if(!getVariablePtr(f0.toString()).isNull() && f1.isNumericalSymbol())
            return pLogData->mulVariableWithScalar(getVariablePtr(f0.toString()), f1.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(f1.toString()).isNull() && f0.isNumericalSymbol())
            return pLogData->mulVariableWithScalar(getVariablePtr(f1.toString()), f0.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(f0.toString()).isNull() && !getVariablePtr(f1.toString()).isNull())
            return pLogData->multVariables(getVariablePtr(f0.toString()), getVariablePtr(f1.toString())).data()->getFullVariableName();
    }
    if(symHopExpr.isPower())
    {
        SymHop::Expression b = *symHopExpr.getBase();
        SymHop::Expression p = *symHopExpr.getPower();
        if(!getVariablePtr(b.toString()).isNull() && p.toDouble() == 2.0)
            return pLogData->multVariables(getVariablePtr(b.toString()), getVariablePtr(b.toString())).data()->getFullVariableName();
    }
    if(symHopExpr.getFactors().size() == 1 && symHopExpr.getDivisors().size() == 1)
    {
        SymHop::Expression f = symHopExpr.getFactors()[0];
        SymHop::Expression d = symHopExpr.getDivisors()[0];
        if(!getVariablePtr(f.toString()).isNull() && d.isNumericalSymbol())
            return pLogData->divVariableWithScalar(getVariablePtr(f.toString()), d.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(f.toString()).isNull() && !getVariablePtr(d.toString()).isNull())
            return pLogData->divVariables(getVariablePtr(f.toString()), getVariablePtr(d.toString())).data()->getFullVariableName();
    }
    if(symHopExpr.getTerms().size() == 2)
    {
        SymHop::Expression t0 = symHopExpr.getTerms()[0];
        SymHop::Expression t1 = symHopExpr.getTerms()[1];
        if(!getVariablePtr(t0.toString()).isNull() && t1.isNumericalSymbol())
            return pLogData->addVariableWithScalar(getVariablePtr(t0.toString()), t1.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(t1.toString()).isNull() && t0.isNumericalSymbol())
            return pLogData->addVariableWithScalar(getVariablePtr(t1.toString()), t0.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(t0.toString()).isNull() && !getVariablePtr(t1.toString()).isNull())
            return pLogData->addVariables(getVariablePtr(t0.toString()), getVariablePtr(t1.toString())).data()->getFullVariableName();

        if(t1.isNegative())
        {
            t1.changeSign();
            if(!getVariablePtr(t0.toString()).isNull() && t1.isNumericalSymbol())
                return pLogData->subVariableWithScalar(getVariablePtr(t0.toString()), t1.toDouble()).data()->getFullVariableName();
            else if(!getVariablePtr(t1.toString()).isNull() && t0.isNumericalSymbol())
                return pLogData->subVariableWithScalar(getVariablePtr(t1.toString()), t0.toDouble()).data()->getFullVariableName();
            else if(!getVariablePtr(t0.toString()).isNull() && !getVariablePtr(t1.toString()).isNull())
                return pLogData->subVariables(getVariablePtr(t0.toString()), getVariablePtr(t1.toString())).data()->getFullVariableName();
        }
    }

    *returnType = Scalar;
    return QString::number(symHopExpr.evaluate(mLocalVars));
}


void HcomHandler::splitAtFirst(QString str, QString c, QString &left, QString &right)
{
    int idx=0;
    int parBal=0;
    for(; idx<str.size(); ++idx)
    {
        if(str[idx] == '(') { ++parBal; }
        if(str[idx] == ')') { --parBal; }
        if(str.mid(idx, c.size()) == c && parBal == 0)
        {
            break;
        }
    }

    left = str.left(idx);
    right = str.right(str.size()-idx-c.size());
    return;
}


bool HcomHandler::containsOutsideParentheses(QString str, QString c)
{
    int idx=0;
    int parBal=0;
    for(; idx<str.size(); ++idx)
    {
        if(str[idx] == '(') { ++parBal; }
        if(str[idx] == ')') { --parBal; }
        if(str.mid(idx, c.size()) == c && parBal == 0)
        {
            return true;
        }
    }
    return false;
}


QString HcomHandler::runScriptCommands(QStringList lines)
{
    qDebug() << "Number of commands to run: " << lines.size();

    for(int l=0; l<lines.size(); ++l)
    {
        if(lines[l].startsWith("#") || lines[l].startsWith("&")) { continue; }  //Ignore comments and labels

        if(lines[l].startsWith("stop"))
        {
            return "%%%%%EOF";
        }
        else if(lines[l].startsWith("goto"))
        {
            QString argument = lines[l].section(" ",1);
            return argument;
        }
        else if(lines[l].startsWith("while"))        //Handle while loops
        {
            QString argument = lines[l].section("(",1).remove(")");
            QStringList loop;
            int nLoops=1;
            while(nLoops > 0)
            {
                ++l;
                if(l>lines.size()-1)
                {
                    mpConsole->print("Missing REPEAT in while loop.");
                    return QString();
                }

                if(lines[l].startsWith("while")) { ++nLoops; }
                if(lines[l].startsWith("repeat")) { --nLoops; }

                loop.append(lines[l]);
            }
            loop.removeLast();

            bool evalOk;
            VariableType type;
            while(evaluateExpression(argument, &type, &evalOk).toInt() > 0)
            {
                if(!evalOk)
                {
                    mpConsole->print("Evaluation of loop argument failed.");
                    break;
                }
                QString gotoLabel = runScriptCommands(loop);
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
        }
        else if(lines[l].startsWith("if"))        //Handle if statements
        {
            QString argument = lines[l].section("(",1).remove(")");
            qDebug() << "Argument: " << argument;
            QStringList ifCode;
            QStringList elseCode;
            bool inElse=false;
            while(true)
            {
                ++l;
                if(l>lines.size()-1)
                {
                    mpConsole->print("Missing ENDIF in if-statement.");
                    return QString();
                }
                if(lines[l].startsWith("endif"))
                {
                    break;
                }
                if(lines[l].startsWith("else"))
                {
                    inElse=true;
                }
                else if(!inElse)
                {
                    ifCode.append(lines[l]);
                }
                else
                {
                    elseCode.append(lines[l]);
                }
            }

            bool evalOk;
            VariableType type;
            if(evaluateExpression(argument, &type, &evalOk).toInt() > 0)
            {
                if(!evalOk)
                {
                    mpConsole->print("Evaluation of if-statement argument failed.");
                    return QString();
                }
                QString gotoLabel = runScriptCommands(ifCode);
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
            else
            {
                if(!evalOk)
                {
                    mpConsole->print("Evaluation of if-statement argument failed.");
                    return QString();
                }
                QString gotoLabel = runScriptCommands(elseCode);
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
        }
        else if(lines[l].startsWith("foreach"))        //Handle foreach loops
        {
            QString var = lines[l].section(" ",1,1);
            QString filter = lines[l].section(" ",2,2);
            QStringList vars;
            getVariables(filter, vars);
            QStringList loop;
            while(!lines[l].startsWith("endforeach"))
            {
                ++l;
                loop.append(lines[l]);
            }
            loop.removeLast();

            for(int v=0; v<vars.size(); ++v)
            {
                //Append quotations around spaces
                QStringList splitVar = vars[v].split(".");
                vars[v].clear();
                for(int s=0; s<splitVar.size(); ++s)
                {
                    if(splitVar[s].contains(" "))
                    {
                        splitVar[s].append("\"");
                        splitVar[s].prepend("\"");
                    }
                    vars[v].append(splitVar[s]);
                    vars[v].append(".");
                }
                vars[v].chop(1);

                //Execute command
                QStringList tempCmds;
                for(int l=0; l<loop.size(); ++l)
                {
                    QString tempCmd = loop[l];
                    tempCmd.replace("$"+var, vars[v]);
                    tempCmds.append(tempCmd);
                }
                QString gotoLabel = runScriptCommands(tempCmds);
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
        }
        else
        {
            this->executeCommand(lines[l]);
        }
    }
    return QString();
}


//! @brief Help function that returns a list of components depending on input (with support for asterisks)
//! @param str String to look for
//! @param components Reference to list of found components
void HcomHandler::getComponents(QString str, QList<ModelObject*> &components)
{
    QString left = str.split("*").first();
    QString right = str.split("*").last();

    ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
    if(!pCurrentTab) { return; }
    SystemContainer *pCurrentSystem = pCurrentTab->getTopLevelSystem();
    if(!pCurrentSystem) { return; }
    for(int n=0; n<pCurrentSystem->getModelObjectNames().size(); ++n)
    {
        if(pCurrentSystem->getModelObjectNames().at(n).startsWith(left) && pCurrentSystem->getModelObjectNames().at(n).endsWith(right))
        {
            components.append(pCurrentSystem->getModelObject(pCurrentSystem->getModelObjectNames().at(n)));
        }
    }
}


//! @brief Help function that returns a list of parameters according to input (with support for asterisks)
//! @param str String to look for
//! @param pComponent Pointer to component to look in
//! @param parameterse Reference to list of found parameters
void HcomHandler::getParameters(QString str, ModelObject* pComponent, QStringList &parameters)
{
    QString left = str.split("*").first();
    QString right = str.split("*").last();


    for(int n=0; n<pComponent->getParameterNames().size(); ++n)
    {
        if(pComponent->getParameterNames().at(n).startsWith(left) && pComponent->getParameterNames().at(n).endsWith(right))
        {
            parameters.append(pComponent->getParameterNames().at(n));
        }
    }
}


void HcomHandler::getParameters(QString str, QStringList &parameters)
{
    if(gpMainWindow->mpProjectTabs->count() == 0) { return; }

    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();

    QStringList componentNames = pSystem->getModelObjectNames();

    QStringList allParameters;

    //Add quotation marks around component name if it contains spaces
    for(int n=0; n<componentNames.size(); ++n)
    {
        QStringList parameterNames = pSystem->getModelObject(componentNames[n])->getParameterNames();

        if(componentNames[n].contains(" "))
        {
            componentNames[n].prepend("\"");
            componentNames[n].append("\"");
        }

        for(int p=0; p<parameterNames.size(); ++p)
        {
            allParameters.append(componentNames[n]+"."+parameterNames[p]);
        }
    }

    QStringList systemParameters = pSystem->getParameterNames();
    for(int s=0; s<systemParameters.size(); ++s)
    {
        if(systemParameters[s].contains(" "))
        {
            systemParameters[s].prepend("\"");
            systemParameters[s].append("\"");
        }
        allParameters.append(systemParameters[s]);
    }

    QStringList splitStr = str.split("*");
    for(int p=0; p<allParameters.size(); ++p)
    {
        bool ok=true;
        QString name = allParameters[p];
        for(int s=0; s<splitStr.size(); ++s)
        {
            if(s==0)
            {
                if(!name.startsWith(splitStr[s]))
                {
                    ok=false;
                    break;
                }
                name.remove(0, splitStr[s].size());
            }
            else if(s==splitStr.size()-1)
            {
                if(!name.endsWith(splitStr[s]))
                {
                    ok=false;
                    break;
                }
            }
            else
            {
                if(!name.contains(splitStr[s]))
                {
                    ok=false;
                    break;
                }
                name.remove(0, name.indexOf(splitStr[s])+splitStr[s].size());
            }
        }
        if(ok)
        {
            parameters.append(allParameters[p]);
        }
    }
}



QString HcomHandler::getParameterValue(QString parameter)
{
    parameter.remove("\"");
    QString compName = parameter.split(".").first();
    QString parName = parameter.split(".").last();

    if(gpMainWindow->mpProjectTabs->count() == 0)
    {
        return "NaN";
    }

    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    ModelObject *pComp = pSystem->getModelObject(compName);
    if(pComp && pComp->getParameterNames().contains(parName))
    {
        return pComp->getParameterValue(parName);
    }
    else if(pSystem->getParameterNames().contains(parameter))
    {
        return pSystem->getParameterValue(parameter);
    }
    return "NaN";
}

//! @brief Help function that returns a list of variables according to input (with support for asterisks)
//! @param str String to look for
//! @param variables Reference to list of found variables
void HcomHandler::getVariables(QString str, QStringList &variables)
{
    if(gpMainWindow->mpProjectTabs->count() == 0) { return; }

    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    QStringList names = pSystem->getLogDataHandler()->getPlotDataNames();
    names.append(pSystem->getAliasNames());

    //Add quotation marks around component name if it contains spaces
    for(int n=0; n<names.size(); ++n)
    {
        if(names[n].split(".").first().contains(" "))
        {
            names[n].prepend("\"");
            names[n].insert(names[n].indexOf("."),"\"");
        }
    }

    //Translate long data names to short equivalents
    for(int n=0; n<names.size(); ++n)
    {
        toShortDataNames(names[n]);
    }

    QStringList splitStr = str.split("*");
    for(int n=0; n<names.size(); ++n)
    {
        bool ok=true;
        QString name = names[n];
        for(int s=0; s<splitStr.size(); ++s)
        {
            if(s==0)
            {
                if(!name.startsWith(splitStr[s]))
                {
                    ok=false;
                    break;
                }
                name.remove(0, splitStr[s].size());
            }
            else if(s==splitStr.size()-1)
            {
                if(!name.endsWith(splitStr[s]))
                {
                    ok=false;
                    break;
                }
            }
            else
            {
                if(!name.contains(splitStr[s]))
                {
                    ok=false;
                    break;
                }
                name.remove(0, name.indexOf(splitStr[s])+splitStr[s].size());
            }
        }
        if(ok)
        {
            variables.append(names[n]);
        }
    }
}


QString HcomHandler::getWorkingDirectory()
{
    return mPwd;
}


//! @brief Checks if a command is an arithmetic expression and evaluates it if possible
//! @param cmd Command to evaluate
//! @returns True if it is a correct exrpession, otherwise false
bool HcomHandler::evaluateArithmeticExpression(QString cmd)
{
    //cmd.replace("==", "§§§§§");
    cmd.replace("**", "%%%%%");

/*    if(cmd.count("=") > 1)
    {
        mpConsole->print("Multiple assignments not allowed.");
        return false;
    }
    else */

//    cmd.replace(">=", "!!!gtoe!!!");
//    cmd.replace("<=", "!!!stoe!!!");

//    cmd.replace("!!!gtoe!!!", ">=");
//    cmd.replace("!!!stoe!!!", "<=");
    if(cmd.endsWith("*")) { return false; }

    SymHop::Expression expr = SymHop::Expression(cmd);

    //if(cmd.count("=") == 1)     //Assignment
    if(expr.isAssignment())
    {
        QString left = expr.getLeft()->toString();

        bool leftIsOk = left[0].isLetter();
        for(int i=1; i<left.size(); ++i)
        {
            if(!left.at(i).isLetterOrNumber())
            {
                leftIsOk = false;
            }
        }

        LogDataHandler *pLogData = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler();
        QStringList plotDataNames = pLogData->getPlotDataNames();
        if(!leftIsOk && !plotDataNames.contains(left))
        if(!leftIsOk && (gpMainWindow->mpProjectTabs->count() == 0 || !getVariablePtr(left)))
        {
            mpConsole->print("Illegal variable name.");
            return false;
        }

        VariableType type;
        bool evalOk;
        QString value = evaluateExpression(expr.getRight()->toString(), &type, &evalOk);

        //QString right = cmd.split("=").at(1);
        //right.remove(" ");

        //bool evalOk;
        //VariableType type;
        //QString value=evaluateExpression(right,&type,&evalOk);

        if(evalOk && type==Scalar)
        {
            mLocalVars.insert(left, value.toDouble());
            mpConsole->print("Assigning "+left+" with "+value);
            return true;
        }
        else if(evalOk && type==DataVector)
        {
            SharedLogVariableDataPtrT pLeftData = getVariablePtr(left);
            SharedLogVariableDataPtrT pValueData = getVariablePtr(value);
            if(pLeftData != 0) { left = pLeftData->getFullVariableName(); }
            if(pValueData != 0) { value = pValueData->getFullVariableName(); }

            gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->assignVariable(left, value);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
//        cmd.replace("!!!gtoe!!!", ">=");
//        cmd.replace("!!!stoe!!!", "<=");

        //! @todo Should we allow pure expessions without assignment?
        bool evalOk;
        VariableType type;
        QString value=evaluateExpression(cmd, &type, &evalOk);
        if(evalOk && type==Scalar)
        {
            mpConsole->print(value);
            return true;
        }
        else if(evalOk && type==DataVector)
        {
            mpConsole->print(value);
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}


//! @brief Returns a pointer to a data variable for given full data name
//! @param fullName Full concatinated name of the variable
//! @returns Pointer to the data variable
SharedLogVariableDataPtrT HcomHandler::getVariablePtr(QString fullName)
{
    fullName.replace(".","#");
    int generation = -1;

    if(fullName.count("#") == 1 || fullName.count("#") == 3)
    {
        generation = fullName.split("#").last().toInt();
        fullName.chop(fullName.split("#").last().size()+1);
    }

    if(fullName.endsWith("#x"))
    {
        fullName.chop(2);
        fullName.append("#Position");
    }
    else if(fullName.endsWith("#v"))
    {
        fullName.chop(2);
        fullName.append("#Velocity");
    }
    else if(fullName.endsWith("#f"))
    {
        fullName.chop(2);
        fullName.append("#Force");
    }
    else if(fullName.endsWith("#p"))
    {
        fullName.chop(2);
        fullName.append("#Pressure");
    }
    else if(fullName.endsWith("#q"))
    {
        fullName.chop(2);
        fullName.append("#Flow");
    }
    else if(fullName.endsWith("#val"))
    {
        fullName.chop(4);
        fullName.append("#Value");
    }
    else if(fullName.endsWith("#Zc"))
    {
        fullName.chop(3);
        fullName.append("#CharImp");
    }
    else if(fullName.endsWith("#c"))
    {
        fullName.chop(2);
        fullName.append("#WaveVariable");
    }
    else if(fullName.endsWith("#me"))
    {
        fullName.chop(3);
        fullName.append("#EquivalentMass");
    }
    else if(fullName.endsWith("#Q"))
    {
        fullName.chop(2);
        fullName.append("#HeatFlow");
    }
    else if(fullName.endsWith("#t"))
    {
        fullName.chop(2);
        fullName.append("#Temperature");
    }

//    SharedLogVariableDataPtrT pRetVal = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->getPlotDataByAlias(fullName,generation);
//    if(!pRetVal)
//    {
        SharedLogVariableDataPtrT pRetVal = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->getPlotData(fullName,generation);
//    }
    return pRetVal;
}


//! @brief Parses a string into a number
//! @param str String to parse, should be a number of a variable name
//! @param ok Pointer to boolean that tells if parsing was successful
double HcomHandler::getNumber(QString str, bool *ok)
{
    *ok = true;
    if(str.toDouble())
    {
        return str.toDouble();
    }
    else if(mLocalVars.contains(str))
    {
        return mLocalVars.find(str).value();
    }
    *ok = false;
    return 0;
}


void HcomHandler::toShortDataNames(QString &variable)
{
    if(variable.endsWith(".Position"))
    {
        variable.chop(9);
        variable.append(".x");
    }
    else if(variable.endsWith(".Velocity"))
    {
        variable.chop(9);
        variable.append(".v");
    }
    else if(variable.endsWith(".Force"))
    {
        variable.chop(6);
        variable.append(".f");
    }
    else if(variable.endsWith(".Pressure"))
    {
        variable.chop(9);
        variable.append(".p");
    }
    else if(variable.endsWith(".Flow"))
    {
        variable.chop(5);
        variable.append(".q");
    }
    else if(variable.endsWith(".Value"))
    {
        variable.chop(6);
        variable.append(".val");
    }
    else if(variable.endsWith(".CharImp"))
    {
        variable.chop(8);
        variable.append(".Zc");
    }
    else if(variable.endsWith(".WaveVariable"))
    {
        variable.chop(13);
        variable.append(".c");
    }
    else if(variable.endsWith(".EquivalentMass"))
    {
        variable.chop(15);
        variable.append(".me");
    }
    else if(variable.endsWith(".HeatFlow"))
    {
        variable.chop(9);
        variable.append(".Q");
    }
    else if(variable.endsWith(".Temperature"))
    {
        variable.chop(12);
        variable.append(".t");
    }
}


QString HcomHandler::getDirectory(QString cmd)
{
    if(QDir().exists(QDir().cleanPath(mPwd+"/"+cmd)))
    {
        return QDir().cleanPath(mPwd+"/"+cmd);
    }
    else if(QDir().exists(cmd))
    {
        return cmd;
    }
    else
    {
        return "";
    }
}

QStringList HcomHandler::getArguments(QString cmd)
{
    QStringList splitCmd;
    bool withinQuotations = false;
    int start=0;
    for(int i=0; i<cmd.size(); ++i)
    {
        if(cmd[i] == '\"')
        {
            withinQuotations = !withinQuotations;
        }
        if(cmd[i] == ' ' && !withinQuotations)
        {
            splitCmd.append(cmd.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmd.append(cmd.right(cmd.size()-start));
    splitCmd.removeFirst();

    return splitCmd;
}


int HcomHandler::getNumberOfArguments(QString cmd)
{
    return getArguments(cmd).size();
}

QString HcomHandler::getArgument(QString cmd, int idx)
{
    return getArguments(cmd).at(idx);
}
