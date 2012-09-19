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
#include "PlotWindow.h"
#include <QDateTime>


HcomWidget::HcomWidget(MainWindow *pParent)
    : QTextEdit(pParent)
{
    this->setReadOnly(false);
    this->setFont(QFont(this->font().family(), 8));
    this->setMouseTracking(true);
    this->installEventFilter(this);
    this->append("Hopsan HCOM Terminal v0.1\nWrite HELP for a list of commands.\n");
    this->append(">> ");

    mCurrentHistoryItem=-1;

    mCmdList << "SIM" << "EXIT" << "PLOT" << "DIPA" << "CHPA" << "CHSS" << "HELP" << "EXEC" << "WRHI" << "PRINT";

    mCurrentPlotWindow = 0;
}


//! @brief Reimplementation of QWidget::sizeHint(), used to reduce the size of the message widget when docked
QSize HcomWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 1; //pixels
    return size;
}


void HcomWidget::loadConfig()
{
}


void HcomWidget::mouseMoveEvent(QMouseEvent *event)
{
    this->setFrameShape(QFrame::NoFrame);

    //qDebug() << "Mouse move event!";

    QTextEdit::mouseMoveEvent(event);
}


bool HcomWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        //qDebug() << "Ate an event!";

        this->setFrameShape(QFrame::NoFrame);

        return true;
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}


bool HcomWidget::isOnLastLine()
{
    int row = this->textCursor().blockNumber();
    int col = this->textCursor().columnNumber();
    int rows = this->document()->blockCount();

    return (row == rows-1 && col > 2);

    //return (para > promptParagraph) || ( (para == promptParagraph) && (index >= promptLength) );
}


void HcomWidget::keyPressEvent(QKeyEvent *event)
{
    if(isOnLastLine())
    {
        int col = this->textCursor().columnNumber();
        if(event->key() == Qt::Key_Backspace && col == 3)
        {
            return;     //Don't allow backspace if on first allowed character
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
        qDebug() << "Nope!";
    }
}


void HcomWidget::handleEnterKeyPress()
{
    QString cmd = this->document()->lastBlock().text();
    cmd = cmd.right(cmd.size()-3);
    executeCommand(cmd);

    mHistory.prepend(cmd);

    this->append(">> ");
}


void HcomWidget::handleUpKeyPress()
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
    this->insertPlainText(">> " + mHistory.at(mCurrentHistoryItem));

    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}


void HcomWidget::handleDownKeyPress()
{
    if(mCurrentHistoryItem == -1) { return; }

    this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor );
    this->textCursor().removeSelectedText();

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


void HcomWidget::handleTabKeyPress()
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

        for(int i=0; i<mCmdList.size(); ++i)
        {
            if(mCmdList.at(i).startsWith(mAutoCompleteFilter))
            {
                mAutoCompleteResults.append(mCmdList.at(i));
            }
        }
        mCurrentAutoCompleteIndex = 0;
    }

    if(mAutoCompleteResults.isEmpty()) { return; }

    this->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
    this->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor );
    this->textCursor().removeSelectedText();

    this->insertPlainText(">> "+mAutoCompleteResults.at(mCurrentAutoCompleteIndex));

    this->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}


void HcomWidget::cancelAutoComplete()
{
    mAutoCompleteFilter.clear();
    mAutoCompleteResults.clear();
}

void HcomWidget::cancelRecentHistory()
{
    mCurrentHistoryItem=-1;
}


void HcomWidget::executeCommand(QString cmd)
{
    QString majorCmd = cmd.split(" ").first().toUpper();
    QString subCmd;
    if(cmd.split(" ").size() == 1)
    {
        subCmd = QString();
    }
    else
    {
        subCmd = cmd.right(cmd.size()-majorCmd.size()-1);
    }

    int idx = mCmdList.indexOf(majorCmd);

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

    switch (idx)
    {
    case 0:
        if(pCurrentTab)
        {
            pCurrentTab->simulate_blocking();
        }
        break;
    case 1:
        gpMainWindow->close();
        break;
    case 2:
        executePlotCommand(subCmd);
        break;
    case 3:
        executeDisplayParameterCommand(subCmd);
        break;
    case 4:
        executeChangeParameterCommand(subCmd);
        break;
    case 5:
        executeChangeSimulationSettingsCommand(subCmd);
        break;
    case 6:
        executeHelpCommand(subCmd);
        break;
    case 7:
        executeRunScriptCommand(subCmd);
        break;
    case 8:
        executeWriteHistoryToFileCommand(subCmd);
        break;
    case 9:
        executePrintCommand(subCmd);
        break;
    default:
        this->append("Unrecognized command.");
    }
}


//Uses last plot generation unless there is one specified
void HcomWidget::executePlotCommand(QString cmd)
{
    cmd.remove("\"");
    QStringList splitCmd = cmd.split(".");
    if(splitCmd.size() == 3 || splitCmd.size() == 4)
    {

        SystemContainer *pCurrentSystem = gpMainWindow->mpProjectTabs->getCurrentTab()->getTopLevelSystem();
        if(!pCurrentSystem) { return; }
        ModelObject *pModelObject = pCurrentSystem->getModelObject(splitCmd[0]);
        if(!pModelObject) { return; }
        Port *pPort = pModelObject->getPort(splitCmd[1]);
        if(!pPort) { return; }
        int generation = pCurrentSystem->getPlotDataPtr()->size()-1;
        if(splitCmd.size() == 4)
        {
            generation = splitCmd[3].toInt();
            if(!pCurrentSystem->getPlotDataPtr()->componentHasPlotGeneration(generation, splitCmd[0]))
            {
                this->append("Variable not found.");
                return;
            }
        }

        if(!pPort->plot(splitCmd[2]))
        {
            pPort->plot(getShortVariableName(splitCmd[2]));
            gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->getPlotTabWidget()->getCurrentTab()->getCurves().last()->setGeneration(generation);
        }
    }
    else
    {
        this->append("Wrong number of arguments.");
    }
}


void HcomWidget::executeDisplayParameterCommand(QString cmd)
{
    cmd.remove("\"");
    QStringList splitCmd = cmd.split(".");
    if(splitCmd.size() == 2)
    {
        SystemContainer *pCurrentSystem = gpMainWindow->mpProjectTabs->getCurrentTab()->getTopLevelSystem();
        if(!pCurrentSystem) { return; }
        ModelObject *pModelObject = pCurrentSystem->getModelObject(splitCmd[0]);
        if(!pModelObject) { return; }
        QString parameterValue = pModelObject->getParameterValue(splitCmd[1]);
        if(parameterValue.isEmpty())
        {
            parameterValue = "Parameter " + splitCmd[1] + " not found.";
        }
        this->append(parameterValue);
    }
    else
    {
        this->append("Wrong number of arguments.");
    }
}


void HcomWidget::executeChangeParameterCommand(QString cmd)
{
    cmd.remove("\"");
    QStringList splitCmd = cmd.split(" ");
    if(splitCmd.size() == 2)
    {
        QStringList splitFirstCmd = splitCmd[0].split(".");
        if(splitFirstCmd.size() == 2)
        {

            ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
            if(!pCurrentTab) { return; }
            SystemContainer *pCurrentSystem = pCurrentTab->getTopLevelSystem();
            if(!pCurrentSystem) { return; }
            ModelObject *pModelObject = pCurrentSystem->getModelObject(splitFirstCmd[0]);
            if(!pModelObject) { return; }
            if(!pModelObject->setParameterValue(splitFirstCmd[1], splitCmd[1]))
            {
                this->append("Parameter " + splitFirstCmd[1] + " not found.");
            }
        }
    }
    else
    {
        this->append("Wrong number of arguments.");
    }
}


//Change Simulation Settings
//Usage: CHSS [starttime] [timestep] [stoptime] ([samples])
void HcomWidget::executeChangeSimulationSettingsCommand(QString cmd)
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
            this->append("Failed to apply simulation settings.");
        }
    }
    else
    {
        this->append("Wrong number of arguments.");
    }
}


void HcomWidget::executeHelpCommand(QString cmd)
{
    cmd.remove(" ");
    if(cmd.isEmpty())
    {
        append("\nHopsan HCOM Terminal v0.1\n");
        append("Available commands:");
        QString commands;
        for(int c=0; c<mCmdList.size(); ++c)
        {
            commands.append(mCmdList[c]);
            commands.append(" ");
        }
        append(commands+"\n");
    }
    else
    {
        append("\nHelp for " + cmd+"\n");
    }
}


void HcomWidget::executeRunScriptCommand(QString cmd)
{
    QStringList splitCmd = cmd.split(" ");

    if(splitCmd.isEmpty()) { return; }

    QFile file(splitCmd[0]);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        append("Unable to read file.");
        return;
    }

    QString code;
    QTextStream t(&file);
    while(!t.atEnd())
    {
        code = t.readLine();

        for(int i=0; i<splitCmd.size()-1; ++i)  //Replace arguments with their values
        {
            QString str = "$"+QString::number(i+1);
            code.replace(str, splitCmd[i+1]);
        }

        if(!code.startsWith("#"))
        {
            this->executeCommand(code);
        }
    }
    file.close();
}


void HcomWidget::executeWriteHistoryToFileCommand(QString cmd)
{
    if(cmd.isEmpty()) { return; }

    QFile file(cmd);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        append("Unable to write to file.");
        return;
    }

    QTextStream t(&file);
    for(int h=mHistory.size()-1; h>-1; --h)
    {
        t << mHistory[h] << "\n";
    }
    file.close();
}


void HcomWidget::executePrintCommand(QString cmd)
{
    //! @todo Implement

    append("Function not yet implemented.");
}


QString HcomWidget::getShortVariableName(QString var)
{
    QMap<QString, QString> nameMap;
    nameMap.insert("x", "Position");
    nameMap.insert("v", "Velocity");
    nameMap.insert("f", "Force");

    if(nameMap.contains(var))
    {
        return nameMap.find(var).value();
    }
    else
    {
        return QString();
    }
}
