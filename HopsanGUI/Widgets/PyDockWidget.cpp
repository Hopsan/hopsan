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
//! @file   PyDockWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-09-21
//!
//! @brief Contains a derived QDockWidget class that contain a Python console
//!
//$Id$

#include "PyDockWidget.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>

#ifdef USEPYTHONQT

#include "global.h"
#include "PythonQt.h"
#include "PythonQt_QtAll.h"
#include "gui/PythonQtScriptingConsole.h"
#include "PythonQtConversion.h"
#include "DesktopHandler.h"

#include "PyWrapperClasses.h"

#include "GUIObjects/GUIModelObject.h"
#include "GUIPort.h"
#include "Configuration.h"
#include "LogVariable.h"

void PythonTerminalWidget::saveSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement lastscript = appendDomElement(rDomElement, "lastscript");
    lastscript.setAttribute("file", mpScriptFileLineEdit->text());
}


QString PythonTerminalWidget::getLastOutput()
{
    QString text = mpPyConsole->toPlainText();
    QStringList lines = text.split("\n");
    if(lines.size() > 3)
    {
        return lines[lines.size()-3];
    }

    return QString();
}


void PythonTerminalWidget::runPyScript()
{
    PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
    QString command = QString("execfile('").append(mpScriptFileLineEdit->text()).append("')");
    mainContext.evalScript(command);
    mpPyConsole->appendCommandPrompt();
}


void PythonTerminalWidget::loadPyScript()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Script File"),
                                                         gpConfig->getScriptDir(),
                                                         tr("Python Scripts (*.py)"));
    if(!modelFileName.isEmpty())
    {
        mpScriptFileLineEdit->setText(modelFileName);
        QFileInfo fileInfo = QFileInfo(modelFileName);
        gpConfig->setScriptDir(fileInfo.absolutePath());
    }
}


void PythonTerminalWidget::runPyScript(QString path)
{
    if(path.isEmpty()) return;
    PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
    QString command = QString("execfile('").append(path).append("')");
    mainContext.evalScript(command);
    mpPyConsole->appendCommandPrompt();
}


void PythonTerminalWidget::runMultipleCommands(QString command, int n)
{
    PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
    for(int i=0; i<n; ++i)
    {
        mainContext.evalScript(command);
        mpPyConsole->appendCommandPrompt();
        qApp->processEvents();
    }
}

void PythonTerminalWidget::printMessage(const GUIMessage &rMessage)
{
    if(!mDoPrintHopsanMessage) return;

    bool mShowInfoMessages = true;
    bool mShowWarningMessages = true;
    bool mShowErrorMessages = true;
    bool mShowDebugMessages = false;
    // Only show some message types
    if( (rMessage.mType == Info    && mShowInfoMessages)     ||
        (rMessage.mType == Warning && mShowWarningMessages)  ||
        (rMessage.mType == Error   && mShowErrorMessages)    ||
        (rMessage.mType == Debug   && mShowDebugMessages)    ||
        (rMessage.mType == UndefinedMessageType)             ||
        (rMessage.mType == Fatal) )
    {
        QString output = rMessage.mMessage;
        if(!rMessage.mTimestamp.isEmpty())
        {
            output.prepend("["+rMessage.mTimestamp+"] ");
        }

//        // Message is tagged, and group by tag setting is active
//        if(mGroupByTag && !rMessage.mTag.isEmpty() && (rMessage.mTag == mLastTag) )
//        {
//            ++mSubsequentTags;
//            this->undo();
//            output.append(QString("    (%1 similar)").arg(mSubsequentTags));
//        }
//        // Message is not tagged, or group by tag setting is not active
//        else
//        {
//            mSubsequentTags = 1;
//            mLastTag =rMessage.mTag;
//        }

//        mpPyConsole->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
//        mpPyConsole->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
        mpPyConsole->consoleMessage(output);
//        mpPyConsole->->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
    }
}


QString PythonTerminalWidget::runCommand(QString command)
{
    //PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
    //QVariant output = mainContext.evalScript(command);
    //mpPyConsole->appendCommandPrompt();

    mpPyConsole->append("py> "+command);
    mpPyConsole->executeLine(false);

    qApp->processEvents();


    //QVariant test = mainContext.getVariable("j11");
    //qDebug() << test;
    //qDebug() << test.toString();


    return getLastOutput();
}

PythonTerminalWidget::PythonTerminalWidget(QWidget *parent)
    : QWidget(parent)
{
    PythonQt::init(PythonQt::RedirectStdOut);
    PythonQt_QtAll::init();

    PythonQt::self()->registerClass(&ModelObject::staticMetaObject, NULL, PythonQtCreateObject<PyModelObjectClassWrapper>);
    PythonQt::self()->registerClass(&Port::staticMetaObject, NULL, PythonQtCreateObject<PyPortClassWrapper>);
    PythonQt::self()->registerClass(&VectorVariable::staticMetaObject, NULL, PythonQtCreateObject<PyVectorVariableClassWrapper>);

    GUIMessageHandler *pPythonMessageHandler = new GUIMessageHandler(this);

    PythonHopsanInterface *pPythonHopsanInterface = new PythonHopsanInterface(pPythonMessageHandler);
    PythonQtObjectPtr  mainContext = PythonQt::self()->getMainModule();
    mainContext.addObject("hopsan", pPythonHopsanInterface);

    mpPyConsole = new PythonQtScriptingConsole(NULL, mainContext);
    mpPyConsole->consoleMessage("There is an object called hopsan that allow you to interact with Hopsan.");
    mpPyConsole->appendCommandPrompt();

    mDoPrintHopsanMessage = true;
    pPythonMessageHandler->startPublish();
    connect(pPythonMessageHandler, SIGNAL(newAnyMessage(GUIMessage)), this, SLOT(printMessage(GUIMessage)));

    mpScriptFileLineEdit = new QLineEdit();

    mpLoadScriptButton = new QToolButton(this);
    mpLoadScriptButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Open.png"));
    mpLoadScriptButton->setToolTip("Load Script File");
    connect(mpLoadScriptButton, SIGNAL(clicked()), this, SLOT(loadPyScript()));

    QPushButton *pPyCustomButton = new QPushButton(this);
    pPyCustomButton->setText("Run .py-file");
    pPyCustomButton->connect(pPyCustomButton,SIGNAL(clicked()), this, SLOT(runPyScript()));

    QHBoxLayout *pScriptFileLayout = new QHBoxLayout();
    pScriptFileLayout->addWidget(mpScriptFileLineEdit);
    pScriptFileLayout->addWidget(mpLoadScriptButton);
    pScriptFileLayout->addWidget(pPyCustomButton);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(mpPyConsole);
    pLayout->setContentsMargins(4,4,4,4);
    pLayout->addLayout(pScriptFileLayout);

    mpScriptFileLineEdit->setText(gpConfig->getLastPyScriptFile());

    // Add script path to Python path
    QString scriptPath = QString(gpDesktopHandler->getScriptsPath());
    scriptPath.replace("\\", "/");
    scriptPath.replace("//", "/");
    runCommand("import sys");
    runCommand("sys.path.append(\""+scriptPath+"\")");

    mpPyConsole->clear();
}


////! @brief Reimplementation of QWidget::sizeHint(), used to reduce the size of the python widget when docked
//QSize PyWidget::sizeHint() const
//{
//    QSize size = QWidget::sizeHint();
//    //Set very small height. A minimum apperantly stops at resonable size.
//    size.rheight() = 1; //pixels
//    return size;
//}

#endif
