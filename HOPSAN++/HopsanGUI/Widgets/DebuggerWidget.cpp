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
//! @file   DebuggerWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a class for the debugger widget
//!

//Qt includes
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

//Hopsan includes
#include "ComponentSystem.h"
#include "CoreAccess.h"
#include "DebuggerWidget.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "MainWindow.h"
#include "Widgets/ModelWidget.h"



DebuggerWidget::DebuggerWidget(SystemContainer *pSystem, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowIcon(QIcon(QString(ICONPATH)+"Hopsan-Debug.png"));

    mpSystem = pSystem;

    this->resize(800, 600);
    QVBoxLayout *pVerticalLayout = new QVBoxLayout(this);


    //Trace tab
    mpTraceTab = new QWidget();
    mpTraceTabLayout = new QVBoxLayout(mpTraceTab);
    mpTraceTable = new QTableWidget(0,0,mpTraceTab);
    mpTraceTabLayout->addWidget(mpTraceTable);


    //Variables tab
    mpVariablesTab = new QWidget();
    mpComponentsList = new QListWidget(mpVariablesTab);
    mpPortsList = new QListWidget(mpVariablesTab);
    mpVariablesList = new QListWidget(mpVariablesTab);
    mpRemoveButton = new QPushButton(mpVariablesTab);
    mpAddButton = new QPushButton(mpVariablesTab);
    mpChoosenVariablesList = new QListWidget(mpVariablesTab);

    mpVariablesTabLayout = new QGridLayout(mpVariablesTab);
    mpVariablesTabLayout->addWidget(mpComponentsList,       1, 0, 1, 2);
    mpVariablesTabLayout->addWidget(mpPortsList,            1, 2, 1, 2);
    mpVariablesTabLayout->addWidget(mpVariablesList,        1, 4, 1, 2);
    mpVariablesTabLayout->addWidget(mpAddButton,            3, 0, 1, 3);
    mpVariablesTabLayout->addWidget(mpRemoveButton,         3, 3, 1, 3);
    mpVariablesTabLayout->addWidget(mpChoosenVariablesList, 4, 0, 1, 6);


    //Tab widget
    mpTabWidget = new QTabWidget(this);
    mpTabWidget->addTab(mpTraceTab, QString());
    mpTabWidget->addTab(mpVariablesTab, QString());
    mpTabWidget->setCurrentIndex(0);
    pVerticalLayout->addWidget(mpTabWidget);


    //Buttons widget
    QWidget *pButtonsWidget = new QWidget(this);
    mpCurrentStepLabel = new QLabel(pButtonsWidget);
    mTimeIndicatorLabel = new QLabel(pButtonsWidget);
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    mTimeIndicatorLabel->setFont(font);
    mpAbortButton = new QPushButton(pButtonsWidget);
    mpInitializeButton = new QPushButton(pButtonsWidget);
    mpGotoTimeSpinBox = new QDoubleSpinBox(pButtonsWidget);
    mpGotoButton = new QPushButton(pButtonsWidget);
    mpForwardButton = new QPushButton(pButtonsWidget);
    mpNumStepsSpinBox = new QSpinBox(pButtonsWidget);
    mpMultiForwardButton = new QPushButton(pButtonsWidget);

    QGridLayout *pButtonLayout = new QGridLayout(pButtonsWidget);
    pButtonLayout->addWidget(mpCurrentStepLabel,0,0);
    pButtonLayout->addWidget(mTimeIndicatorLabel,0,1);
    //pButtonLayout->addItem(pHorizontalSpacer,0,2,2,1);
    pButtonLayout->addWidget(mpAbortButton,0,3);
    pButtonLayout->addWidget(mpInitializeButton,0,4);
    pButtonLayout->addWidget(mpGotoButton,0,5);
    pButtonLayout->addWidget(mpGotoTimeSpinBox,1,5);
    pButtonLayout->addWidget(mpForwardButton,0,6);
    pButtonLayout->addWidget(mpNumStepsSpinBox,1,7);
    pButtonLayout->addWidget(mpMultiForwardButton,0,7);
    pButtonLayout->setColumnMinimumWidth(2,40);
    pButtonLayout->setColumnStretch(2,1);

    pVerticalLayout->addWidget(pButtonsWidget);

    connect(mpAbortButton,        SIGNAL(clicked()),                      this, SLOT(accept()));
    connect(mpComponentsList,     SIGNAL(currentTextChanged(QString)),    this, SLOT(updatePortsList(QString)));
    connect(mpPortsList,          SIGNAL(currentTextChanged(QString)),    this, SLOT(updateVariablesList(QString)));
    connect(mpAddButton,          SIGNAL(clicked()),                      this, SLOT(addVariable()));
    connect(mpRemoveButton,       SIGNAL(clicked()),                      this, SLOT(removeVariable()));
    connect(mpInitializeButton,   SIGNAL(clicked()),                      this, SLOT(runInitialization()));
    connect(mpForwardButton,      SIGNAL(clicked()),                      this, SLOT(stepForward()));
    connect(mpMultiForwardButton, SIGNAL(clicked()),                      this, SLOT(nStepsForward()));
    connect(mpGotoButton,         SIGNAL(clicked()),                      this, SLOT(simulateTo()));

    retranslateUi();
    setInitData();
}


void DebuggerWidget::retranslateUi()
{
    setWindowTitle(tr("Hopsan Debugger"));
    mpTabWidget->setTabText(mpTabWidget->indexOf(mpTraceTab), tr("Trace variables"));
    mpRemoveButton->setText(tr("Remove"));
    mpAddButton->setText(tr("Add"));
    mpTabWidget->setTabText(mpTabWidget->indexOf(mpVariablesTab), tr("Select variables"));
    mpCurrentStepLabel->setText(tr("Current time:"));
    mTimeIndicatorLabel->setText(tr("0"));
    mpAbortButton->setText(tr("Abort"));
    mpGotoButton->setText(tr("Go to time"));
    mpForwardButton->setText(tr("step forward"));
    mpMultiForwardButton->setText(tr("n steps fwd."));
    mpGotoTimeSpinBox->setToolTip(tr("Target time"));
    mpNumStepsSpinBox->setToolTip(tr("Num steps"));
    mpInitializeButton->setText(tr("Initialize"));
}


void DebuggerWidget::setInitData()
{
    mCurrentTime = gpMainWindow->getStartTimeFromToolBar();

    mpComponentsList->addItems(mpSystem->getModelObjectNames());
    mpComponentsList->sortItems();
    mpGotoButton->setDisabled(true);
    mpForwardButton->setDisabled(true);
    mpMultiForwardButton->setDisabled(true);

    mpNumStepsSpinBox->setMinimum(0);
    mpNumStepsSpinBox->setMaximum(INT_MAX);
    mpNumStepsSpinBox->setValue(10);

    mpGotoTimeSpinBox->setMinimum(getStartTime()+getTimeStep());
    mpGotoTimeSpinBox->setValue(getStartTime()+getTimeStep());
    mpGotoTimeSpinBox->setMaximum(getStopTime());
    mpGotoTimeSpinBox->setDecimals( int(log10(1.0/getTimeStep())));
    mpGotoTimeSpinBox->setSingleStep(getTimeStep());

    QString dateString = QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate);
    qDebug() << "dateString = " << dateString.toUtf8();
    dateString.replace(":", "_");
    dateString.replace(".", "_");
    dateString.replace(" ", "_");
    dateString.replace("/", "_");
    dateString.replace("\\", "_");
    mOutputFile.setFileName(gDesktopHandler.getDocumentsPath()+"HopsanDebuggerOutput_"+dateString+".csv");
}


void DebuggerWidget::updatePortsList(QString component)
{
    mpPortsList->clear();
    mpVariablesList->clear();
    QList<Port*> ports = mpSystem->getModelObject(component)->getPortListPtrs();
    Q_FOREACH(const Port *port, ports)
    {
        mpPortsList->addItem(port->getName());
    }
    mpPortsList->sortItems();
}


void DebuggerWidget::updateVariablesList(QString port)
{
    if(port.isEmpty()) return;
    mpVariablesList->clear();
    QString component = mpComponentsList->currentItem()->text();
    NodeInfo info(mpSystem->getModelObject(component)->getPort(port)->getNodeType());
    QStringList variables = info.variableLabels;
    mpVariablesList->addItems(variables);
    // Note we dont want to sort here, we want them to appear in the correct order
}


void DebuggerWidget::addVariable()
{
    if(mpPortsList->count() == 0 || mpVariablesList->count() == 0) return;

    if(mpComponentsList->currentItem() == 0 || mpPortsList->currentItem() == 0 || mpVariablesList->currentItem() == 0) return;
    QString component = mpComponentsList->currentItem()->text();
    QString port = mpPortsList->currentItem()->text();
    QString data = mpVariablesList->currentItem()->text();
    if(component.isEmpty() || port.isEmpty() || data.isEmpty()) return;

    QString fullName = component+"::"+port+"::"+data;

    if(mVariables.contains(fullName)) return;

    mVariables.append(fullName);
    mpChoosenVariablesList->addItem(fullName);


    mpTraceTable->insertColumn(0);

    QTableWidgetItem *pItem = new QTableWidgetItem(fullName);
    mpTraceTable->setHorizontalHeaderItem(0, pItem);
}


void DebuggerWidget::removeVariable()
{
    if(mpChoosenVariablesList->currentItem() == 0) return;

    mVariables.removeOne(mpChoosenVariablesList->currentItem()->text());

    for(int c=0; c<mpTraceTable->columnCount(); ++c)
    {
        if(mpTraceTable->horizontalHeaderItem(c)->text() == mpChoosenVariablesList->currentItem()->text())
        {
            mpTraceTable->removeColumn(c);
            break;
        }
    }

    mpChoosenVariablesList->clear();
    mpChoosenVariablesList->addItems(mVariables);
}

void DebuggerWidget::runInitialization()
{
    double startT = getStartTime();
    double stopT = getStopTime();
    int nSteps = int((stopT-startT)/mpSystem->getTimeStep());
    if(mpSystem->getCoreSystemAccessPtr()->initialize(startT,stopT, nSteps+1))
    {
        mpGotoButton->setEnabled(true);
        mpForwardButton->setEnabled(true);
        mpMultiForwardButton->setEnabled(true);
    }
    collectPlotData(false);
    logLastData();
    updateTimeDisplay();
}

void DebuggerWidget::stepForward()
{
    simulateTo(getCurrentTime()+getTimeStep());
}

void DebuggerWidget::nStepsForward()
{
    for(int i=0; i<mpNumStepsSpinBox->value(); ++i)
    {
        simulateTo(getCurrentTime()+getTimeStep(), false);
    }
    collectPlotData();
    //simulateTo(getCurrentTime()+getTimeStep()*double(mpNumStepsSpinBox->value()));
}

void DebuggerWidget::simulateTo()
{
    //double targetTime = QInputDialog::getDouble(this, "Hopsan Debugger", "Choose target time", getCurrentTime()+getTimeStep(), getCurrentTime()+getTimeStep(), getStopTime(), int(log10(1.0/getTimeStep())));
    simulateTo(mpGotoTimeSpinBox->value());
}

void DebuggerWidget::simulateTo(double targetTime, bool doCollectData)
{
    mpSystem->getCoreSystemAccessPtr()->simulate(getCurrentTime(), targetTime, -1, true);
    if(doCollectData)
    {
        collectPlotData();
    }
    logLastData();
    updateTimeDisplay();
}


void DebuggerWidget::collectPlotData(bool overWriteGeneration)
{
    mpSystem->collectPlotData(overWriteGeneration);
}


void DebuggerWidget::logLastData()
{
    QString outputLine;

    outputLine.append(QString::number(getCurrentTime())+",");

    mpTraceTable->insertRow(mpTraceTable->rowCount());
    QTableWidgetItem *pItem = new QTableWidgetItem(QString::number(getCurrentStep())+": "+QString::number(getCurrentTime()));
    mpTraceTable->setVerticalHeaderItem(mpTraceTable->rowCount()-1, pItem);
    Q_FOREACH(const QString &var, mVariables)
    {
        QString component = var.split("::").at(0);
        QString port = var.split("::").at(1);
        QString data = var.split("::").at(2);

        double value;
        mpSystem->getCoreSystemAccessPtr()->getLastNodeData(component, port, data, value);
        outputLine.append(QString::number(value)+",");
        QTableWidgetItem *pDataItem = new QTableWidgetItem(QString::number(value));
        for(int c=0; c<mpTraceTable->columnCount(); ++c)
        {
            if(mpTraceTable->horizontalHeaderItem(c)->text() == var)
            {
                mpTraceTable->setItem(mpTraceTable->rowCount()-1, c, pDataItem);
            }
        }
    }
    mpTraceTable->scrollToBottom();
    //mpTraceTable->verticalScrollBar()->setSliderPosition (mpTraceTable->verticalScrollBar()->maximum());

    outputLine.chop(1);
    outputLine.append("\n");
    mOutputFile.open(QFile::WriteOnly | QFile::Text | QFile::Append);
    mOutputFile.write(outputLine .toUtf8());
    mOutputFile.close();
}

void DebuggerWidget::updateTimeDisplay()
{
    mTimeIndicatorLabel->setText(QString::number(getCurrentTime()));
    mpGotoTimeSpinBox->setMinimum(getCurrentTime()+getTimeStep());
    mpGotoTimeSpinBox->setValue(getCurrentTime()+getTimeStep());
}

double DebuggerWidget::getCurrentTime() const
{
    return mpSystem->getCoreSystemAccessPtr()->getCurrentTime();
}

int DebuggerWidget::getCurrentStep() const
{
    //return mpSystem->getLogDataHandler()->getAllVariablesAtNewestGeneration().first()->getDataSize()-1;
    return int(mpSystem->getCoreSystemAccessPtr()->getCurrentTime() / getTimeStep());
}

double DebuggerWidget::getTimeStep() const
{
    return mpSystem->getTimeStep();
}

double DebuggerWidget::getStartTime() const
{
    return gpMainWindow->getStartTimeFromToolBar();
}

double DebuggerWidget::getStopTime() const
{
    return gpMainWindow->getFinishTimeFromToolBar();
}

