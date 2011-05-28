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

#include "PythonQt.h"
#include "PythonQt_QtAll.h"
#include "gui/PythonQtScriptingConsole.h"
#include "PythonQtConversion.h"

#include "../PyWrapperClasses.h"


//! Create a dock for the Python console

//! Constructor
PyDockWidget::PyDockWidget(MainWindow *pMainWindow, QWidget * parent)
    : QDockWidget(tr("Python Console"), parent)
{
        PythonQt::init(PythonQt::RedirectStdOut);
        PythonQt_QtAll::init();

        PythonQt::self()->registerCPPClass("MainWindow", "","", PythonQtCreateObject<PyHopsanClassWrapper>);
        PythonQt::self()->registerCPPClass("GUIObject", "","", PythonQtCreateObject<PyGUIObjectClassWrapper>);
        PythonQt::self()->registerCPPClass("GUIPort", "","", PythonQtCreateObject<PyGUIPortClassWrapper>);

        PythonQtObjectPtr  mainContext = PythonQt::self()->getMainModule();
        mainContext.addObject("hopsan", pMainWindow);

//        pyTestClass *test = new pyTestClass();
//        mainContext.addObject("test", test);

        mpPyConsole = new PythonQtScriptingConsole(NULL, mainContext);
        mpPyConsole->consoleMessage("There is an object called hopsan that allow you to interact with Hopsan.");
        mpPyConsole->appendCommandPrompt();

        mpScriptFileLineEdit = new QLineEdit();
        //mpScriptFileLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        //mpStartTimeLineEdit->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpStartTimeLineEdit));

        QPushButton *pPyCustomButton = new QPushButton();
        pPyCustomButton->setText("Run .py-file");
        pPyCustomButton->connect(pPyCustomButton,SIGNAL(clicked()), this, SLOT(runPyScript()));

        QHBoxLayout *pScriptFileLayout = new QHBoxLayout();
        pScriptFileLayout->addWidget(mpScriptFileLineEdit);
        pScriptFileLayout->addWidget(pPyCustomButton);

        QVBoxLayout *pPyLayout = new QVBoxLayout();
        pPyLayout->addWidget(mpPyConsole);
        pPyLayout->setContentsMargins(4,4,4,4);
        pPyLayout->addLayout(pScriptFileLayout);

        PyWidget *pPyWidget = new PyWidget();
        pPyWidget->setLayout(pPyLayout);

        setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        setWidget(pPyWidget);//->setWidget(mpPythonConsole);

}


void PyDockWidget::saveSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement lastscript = appendDomElement(rDomElement, "lastscript");
    lastscript.setAttribute("file", mpScriptFileLineEdit->text());
}


void PyDockWidget::loadSettingsFromDomElement(QDomElement &rDomElement)
{
    QDomElement lastscript = rDomElement.firstChildElement("lastscript");
    QString filename = lastscript.attribute("file");
    if(!(filename.isEmpty()))
        mpScriptFileLineEdit->setText(filename);
}


void PyDockWidget::runPyScript()
{
    PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
    QString command = QString("execfile('").append(mpScriptFileLineEdit->text()).append("')");
    mainContext.evalScript(command);
    mpPyConsole->appendCommandPrompt();
}


void PyDockWidget::runPyScript(QString path)
{
    if(path.isEmpty()) return;
    PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
    QString command = QString("execfile('").append(path).append("')");
    mainContext.evalScript(command);
    mpPyConsole->appendCommandPrompt();
}


PyWidget::PyWidget(QWidget *parent)
    : QWidget(parent)
{
    //Nothing to do...
}


//! @brief Reimplementation of QWidget::sizeHint(), used to reduce the size of the plot widget when docked
QSize PyWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 1; //pixels
    return size;
}
