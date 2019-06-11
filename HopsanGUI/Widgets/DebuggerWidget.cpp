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
//! @file   DebuggerWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a class for the debugger widget
//!

#include "DebuggerWidget.h"

//Hopsan includes
#include "global.h"
#include "ComponentSystem.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "GUIPort.h"
#include "ModelHandler.h"
#include "Widgets/ModelWidget.h"
#include "GUIObjects/GUISystem.h"



DebuggerWidget::DebuggerWidget(ModelWidget *pModel, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Debug.svg"));

    mpModel = pModel;

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
    mpPortsList->setSelectionMode(QListWidget::MultiSelection);
    mpVariablesList = new QListWidget(mpVariablesTab);
    mpVariablesList->setSelectionMode(QListWidget::MultiSelection);
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

    //this->setAttribute(Qt::WA_DeleteOnClose);

    connect(mpModel, SIGNAL(destroyed()), this, SLOT(close()));

    retranslateUi();
    setInitData();
}


void DebuggerWidget::retranslateUi()
{
    setWindowTitle(tr("Hopsan Debugger") + " (" + mpModel->getTopLevelSystemContainer()->getModelFileInfo().fileName().remove(".hmf").remove(".xml")+")");
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
    mCurrentTime = gpModelHandler->getCurrentModel()->getStartTime().toDouble();

    mpComponentsList->addItems(mpModel->getTopLevelSystemContainer()->getModelObjectNames());
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
    mOutputFile.setFileName(gpDesktopHandler->getDocumentsPath()+"HopsanDebuggerOutput_"+dateString+".csv");
}


void DebuggerWidget::updatePortsList(QString component)
{
    mpPortsList->clear();
    mpVariablesList->clear();
    QList<Port*> ports = mpModel->getTopLevelSystemContainer()->getModelObject(component)->getPortListPtrs();
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
    NodeInfo info(mpModel->getTopLevelSystemContainer()->getModelObject(component)->getPort(port)->getNodeType());
    QStringList variables = info.variableLabels;
    mpVariablesList->addItems(variables);
    // Note we don't want to sort here, we want them to appear in the correct order
}


void DebuggerWidget::addVariable()
{
    if(mpComponentsList->currentItem() == 0 || mpPortsList->currentItem() == 0) return;
    QString component = mpComponentsList->currentItem()->text();
    QString port = mpPortsList->currentItem()->text();
    QList<QListWidgetItem*> items = mpVariablesList->selectedItems();

    //List with full variable names to populate
    QStringList variables;


    if(!component.isEmpty() && !port.isEmpty() && items.isEmpty())
    {
        //Case 1: No variable selected. Add all variables from selected ports.
        QList<QListWidgetItem*> ports = mpPortsList->selectedItems();

        for(int i=0; i<ports.size(); ++i)
        {
            port = ports[i]->text();
            NodeInfo info(mpModel->getTopLevelSystemContainer()->getModelObject(component)->getPort(port)->getNodeType());
            for(const QString &data : info.variableLabels)
            {
                variables.push_back(component+"#"+port+"#"+data);
            }
        }
    }
    else if(!component.isEmpty() && !port.isEmpty() && !items.isEmpty())
    {
        //Case 2: Items selected. Add selected items.
        for(int i=0; i<items.size(); ++i)
        {
            variables.push_back(component+"#"+port+"#"+items[i]->text());
        }
    }

    //Add all selected variables to table
    for(const QString &variable : variables)
    {
        if(mVariables.contains(variable)) continue;

        mVariables.append(variable);
        mpChoosenVariablesList->addItem(variable);


        mpTraceTable->insertColumn(0);

        QTableWidgetItem *pItem = new QTableWidgetItem(variable);
        mpTraceTable->setHorizontalHeaderItem(0, pItem);
    }
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
    int nSteps = int((stopT-startT)/mpModel->getTopLevelSystemContainer()->getTimeStep());
    if(mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->initialize(startT,stopT, nSteps+1))
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
    mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->simulate(getCurrentTime(), targetTime, -1, true);
    if(doCollectData)
    {
        collectPlotData();
    }
    logLastData();
    updateTimeDisplay();
}


void DebuggerWidget::collectPlotData(bool overWriteGeneration)
{
    mpModel->collectPlotData(overWriteGeneration);
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
        QString component = var.split("#").at(0);
        QString port = var.split("#").at(1);
        QString data = var.split("#").at(2);

        double value;
        mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->getLastNodeData(component, port, data, value);
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
    return mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->getCurrentTime();
}

int DebuggerWidget::getCurrentStep() const
{
    //return mpSystem->getLogDataHandler()->getAllVariablesAtNewestGeneration().first()->getDataSize()-1;
    return int(mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->getCurrentTime() / getTimeStep());
}

double DebuggerWidget::getTimeStep() const
{
    return mpModel->getTopLevelSystemContainer()->getTimeStep();
}

double DebuggerWidget::getStartTime() const
{
    return gpModelHandler->getCurrentModel()->getStartTime().toDouble();
}

double DebuggerWidget::getStopTime() const
{
    return gpModelHandler->getCurrentModel()->getStopTime().toDouble();
}

