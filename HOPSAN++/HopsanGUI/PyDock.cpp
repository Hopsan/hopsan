//!
//! @file   PyDock.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-09-21
//!
//! @brief Contains a derived QDockWidget class that contain a Python console
//!
//$Id$

#include "PyDock.h"

#include "PythonQt.h"
#include "PythonQt_QtAll.h"
#include "gui/PythonQtScriptingConsole.h"
#include "PythonQtConversion.h"

#include "PyWrapperClasses.h"


//! Create a dock for the Python console

//! Constructor
PyDock::PyDock(MainWindow *pMainWindow, QWidget * parent)
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

        mpPyConsole  = new PythonQtScriptingConsole(NULL, mainContext);
        mpPyConsole->consoleMessage("There is an object called hopsan that allow you to interact with Hopsan NG.");
        mpPyConsole->appendCommandPrompt();

        mpScriptFileLineEdit = new QLineEdit("../../../pyscripts/pyBE.py");
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
        pPyLayout->addLayout(pScriptFileLayout);

        QWidget *pPyWidget = new QWidget();
        pPyWidget->setLayout(pPyLayout);

        setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        setWidget(pPyWidget);//->setWidget(mpPythonConsole);

}


void PyDock::runPyScript()
{
    PythonQtObjectPtr  mainContext = PythonQt::self()->getMainModule();
    QString command = QString("execfile('").append(mpScriptFileLineEdit->text()).append("')");
    mainContext.evalScript(command);
    mpPyConsole->appendCommandPrompt();
}
