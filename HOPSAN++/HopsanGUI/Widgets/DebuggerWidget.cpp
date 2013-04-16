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

#include "DebuggerWidget.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "CoreAccess.h"
#include "MainWindow.h"
#include "DesktopHandler.h"
#include "Widgets/ProjectTabWidget.h"

#include "ComponentSystem.h"


DebuggerWidget::DebuggerWidget(SystemContainer *pSystem, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowIcon(QIcon(QString(ICONPATH)+"Hopsan-Debug.png"));

    mpSystem = pSystem;

    this->resize(800, 600);
    mpVerticalLayout = new QVBoxLayout(this);


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
    mpVerticalLayout->addWidget(mpTabWidget);


    //Buttons widget
    mpButtonsWidget = new QWidget(this);
    mpCurrentStepLabel = new QLabel(mpButtonsWidget);
    mTimeIndicatorLabel = new QLabel(mpButtonsWidget);
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    mTimeIndicatorLabel->setFont(font);
    mpHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    mpAbortButton = new QPushButton(mpButtonsWidget);
    mpInitializeButton = new QPushButton(mpButtonsWidget);
    mpGotoButton = new QPushButton(mpButtonsWidget);
    mpForwardButton = new QPushButton(mpButtonsWidget);

    mpHorizontalLayout = new QHBoxLayout(mpButtonsWidget);
    mpHorizontalLayout->addWidget(mpCurrentStepLabel);
    mpHorizontalLayout->addWidget(mTimeIndicatorLabel);
    mpHorizontalLayout->addItem(mpHorizontalSpacer);
    mpHorizontalLayout->addWidget(mpAbortButton);
    mpHorizontalLayout->addWidget(mpInitializeButton);
    mpHorizontalLayout->addWidget(mpGotoButton);
    mpHorizontalLayout->addWidget(mpForwardButton);

    mpVerticalLayout->addWidget(mpButtonsWidget);

    connect(mpAbortButton,      SIGNAL(clicked()),                      this, SLOT(accept()));
    connect(mpComponentsList,   SIGNAL(currentTextChanged(QString)),    this, SLOT(updatePortsList(QString)));
    connect(mpPortsList,        SIGNAL(currentTextChanged(QString)),    this, SLOT(updateVariablesList(QString)));
    connect(mpAddButton,        SIGNAL(clicked()),                      this, SLOT(addVariable()));
    connect(mpRemoveButton,     SIGNAL(clicked()),                      this, SLOT(removeVariable()));
    connect(mpInitializeButton, SIGNAL(clicked()),                      this, SLOT(runInitialization()));
    connect(mpForwardButton,    SIGNAL(clicked()),                      this, SLOT(stepForward()));
    connect(mpGotoButton,       SIGNAL(clicked()),                      this, SLOT(simulateTo()));

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
    mpForwardButton->setText(tr("Step forward"));
    mpInitializeButton->setText(tr("Initialize"));
}


void DebuggerWidget::setInitData()
{
    mCurrentTime = gpMainWindow->getStartTimeFromToolBar();

    mpComponentsList->addItems(mpSystem->getModelObjectNames());
    mpGotoButton->setDisabled(true);
    mpForwardButton->setDisabled(true);

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
}


void DebuggerWidget::updateVariablesList(QString port)
{
    if(port.isEmpty()) return;
    mpVariablesList->clear();
    QString component = mpComponentsList->currentItem()->text();
    NodeInfo info(mpSystem->getModelObject(component)->getPort(port)->getNodeType());
    QStringList variables = info.variableLabels;
    mpVariablesList->addItems(variables);
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
    }
    collectLastData(false);
    updateTimeDisplay();
}

void DebuggerWidget::stepForward()
{
    simulateTo(getCurrentTime()+getTimeStep());
}

void DebuggerWidget::simulateTo()
{
    double targetTime = QInputDialog::getDouble(this, "Hopsan Debugger", "Choose target time", getCurrentTime()+getTimeStep(), getCurrentTime()+getTimeStep(), getStopTime());
    simulateTo(targetTime);
}

void DebuggerWidget::simulateTo(double targetTime)
{
    mpSystem->getCoreSystemAccessPtr()->simulate(getCurrentTime(), targetTime, -1, true);
    collectLastData();
    updateTimeDisplay();
}

void DebuggerWidget::collectLastData(bool overWriteGeneration)
{
    mpSystem->collectPlotData(overWriteGeneration);

    QString outputLine;

    outputLine.append(QString::number(getCurrentTime())+",");

    mpTraceTable->insertRow(mpTraceTable->rowCount());
    QTableWidgetItem *pItem = new QTableWidgetItem(QString::number(getCurrentTime()));
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
}

double DebuggerWidget::getCurrentTime() const
{
    return mpSystem->getCoreSystemAccessPtr()->getCurrentTime();
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

