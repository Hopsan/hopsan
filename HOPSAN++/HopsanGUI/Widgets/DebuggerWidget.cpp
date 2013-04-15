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

DebuggerWidget::DebuggerWidget(SystemContainer *pSystem, QWidget *parent) :
    QDialog(parent)
{
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
    mpStepIndicatorLabel = new QLabel(mpButtonsWidget);
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    mpStepIndicatorLabel->setFont(font);
    mpHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    mpAbortButton = new QPushButton(mpButtonsWidget);
    mpInitializeButton = new QPushButton(mpButtonsWidget);
    mpGotoButton = new QPushButton(mpButtonsWidget);
    mpForwardButton = new QPushButton(mpButtonsWidget);

    mpHorizontalLayout = new QHBoxLayout(mpButtonsWidget);
    mpHorizontalLayout->addWidget(mpCurrentStepLabel);
    mpHorizontalLayout->addWidget(mpStepIndicatorLabel);
    mpHorizontalLayout->addItem(mpHorizontalSpacer);
    mpHorizontalLayout->addWidget(mpAbortButton);
    mpHorizontalLayout->addWidget(mpInitializeButton);
    mpHorizontalLayout->addWidget(mpGotoButton);
    mpHorizontalLayout->addWidget(mpForwardButton);

    mpVerticalLayout->addWidget(mpButtonsWidget);

    connect(mpAbortButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(mpComponentsList, SIGNAL(currentTextChanged(QString)), this, SLOT(updatePortsList(QString)));
    connect(mpPortsList, SIGNAL(currentTextChanged(QString)), this, SLOT(updateVariablesList(QString)));
    connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addVariable()));
    connect(mpRemoveButton, SIGNAL(clicked()), this, SLOT(removeVariable()));


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
    mpCurrentStepLabel->setText(tr("Current step:"));
    mpStepIndicatorLabel->setText(tr("27"));
    mpAbortButton->setText(tr("Abort"));
    mpGotoButton->setText(tr("Go to step "));
    mpForwardButton->setText(tr("Step forward"));
    mpInitializeButton->setText(tr("Initialize"));
}


void DebuggerWidget::setInitData()
{
    mpComponentsList->addItems(mpSystem->getModelObjectNames());
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
