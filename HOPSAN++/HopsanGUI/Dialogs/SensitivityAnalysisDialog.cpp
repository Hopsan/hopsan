/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 rediibuting any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SensitivityAnalysisDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the sensitivity analysis dialog
//!
//$Id$

#include "Configuration.h"
#include "GUIPort.h"
#include "PlotWindow.h"
#include "Dialogs/SensitivityAnalysisDialog.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/ProjectTabWidget.h"

#ifndef WIN32
#include <unistd.h> //Needed for sysctl
#endif


//! @brief Constructor
SensitivityAnalysisDialog::SensitivityAnalysisDialog(MainWindow *parent)
    : QDialog(parent)
{
        //Set the name and size of the main window
    this->resize(640,640);
    this->setWindowTitle("Sensitivity Analysis");
    this->setPalette(gConfig.getPalette());

    //Parameters list
    mpParametersLabel = new QLabel("Choose uncertain parameters, and specify their standard deviation.");
    QFont boldFont = mpParametersLabel->font();
    boldFont.setBold(true);
    //mpParametersLabel->setFont(boldFont);
    mpParametersList = new QTreeWidget(this);
    mpParameterNameLabel = new QLabel("Parameter Name");
    mpParameterAverageLabel = new QLabel("Mean Value");
    mpParameterSigmaLabel = new QLabel("Standard Deviation");
    mpParameterNameLabel->setFont(boldFont);
    mpParameterAverageLabel->setFont(boldFont);
    mpParameterSigmaLabel->setFont(boldFont);
    mpParametersLayout = new QGridLayout(this);
    mpParametersLayout->addWidget(mpParametersLabel,        0, 0, 1, 3);
    mpParametersLayout->addWidget(mpParametersList,         1, 0, 1, 3);
    mpParametersLayout->addWidget(mpParameterNameLabel,     2, 0, 1, 1);
    mpParametersLayout->addWidget(mpParameterAverageLabel,      2, 1, 1, 1);
    mpParametersLayout->addWidget(mpParameterSigmaLabel,      2, 2, 1, 1);
    mpParametersGroupBox = new QGroupBox(this);
    mpParametersGroupBox->setLayout(mpParametersLayout);

    //Output variables list
    mpOutputLabel = new QLabel("Choose output variables:");
    mpOutputList = new QTreeWidget(this);
    mpOutputNameLabel = new QLabel("Variable Name");
    mpOutputNameLabel->setFont(boldFont);
    mpOutputLayout = new QGridLayout(this);
    mpOutputLayout->addWidget(mpOutputLabel,        0, 0, 1, 3);
    mpOutputLayout->addWidget(mpOutputList,         1, 0, 1, 3);
    mpOutputLayout->addWidget(mpOutputNameLabel,     2, 0, 1, 1);
    mpOutputGroupBox = new QGroupBox(this);
    mpOutputGroupBox->setLayout(mpOutputLayout);

    //Output variables
    mpStepsLabel = new QLabel("Number of simulation steps: ");
    mpStepsSpinBox = new QSpinBox(this);
    mpStepsSpinBox->setValue(100);
    mpStepsSpinBox->setMinimum(1);
    mpStepsSpinBox->setMaximum(1000000);
    mpStepsSpinBox->setSingleStep(1);
    mpStepsLayout = new QHBoxLayout(this);
    mpStepsLayout->addWidget(mpStepsLabel);
    mpStepsLayout->addWidget(mpStepsSpinBox);
    mpStepsWidget = new QWidget(this);
    mpStepsWidget->setLayout(mpStepsLayout);

    //Buttons
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    mpRunButton = new QPushButton(tr("&Start Analysis"), this);
    mpRunButton->setDefault(true);
    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpRunButton, QDialogButtonBox::ActionRole);

    //Toolbar
    mpHelpAction = new QAction("Show Context Help", this);
    mpHelpAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Help.png"));
    mpToolBar = new QToolBar(this);
    mpToolBar->addAction(mpHelpAction);


    //Main layout
    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpParametersGroupBox,   0, 0, 1, 2);
    mpLayout->addWidget(mpOutputGroupBox,       1, 0, 1, 2);
    mpLayout->addWidget(mpStepsWidget,          2, 0, 1, 2);
    mpLayout->addWidget(mpButtonBox,            3, 1, 1, 1);
    mpLayout->addWidget(mpToolBar,              3, 0, 1, 1);
    setLayout(mpLayout);

    //Connections
    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpRunButton,                    SIGNAL(clicked()),      this,                   SLOT(run()));
    connect(mpHelpAction,                   SIGNAL(triggered()),    gpMainWindow,           SLOT(openContextHelp()));
}


//! @brief Reimplementation of open() slot, used to initialize the dialog
void SensitivityAnalysisDialog::open()
{
    mpParametersList->clear();
    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    QStringList componentNames = pSystem->getModelObjectNames();
    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpParametersList->insertTopLevelItem(0, pComponentItem);
        QStringList parameterNames = pSystem->getModelObject(componentNames.at(c))->getParameterNames();
        for(int p=0; p<parameterNames.size(); ++p)
        {
            QTreeWidgetItem *pParameterItem = new QTreeWidgetItem(QStringList() << parameterNames.at(p));
            pParameterItem->setCheckState(0, Qt::Unchecked);
            pComponentItem->insertChild(0, pParameterItem);
        }
    }
    connect(mpParametersList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(updateChosenParameters(QTreeWidgetItem*,int)), Qt::UniqueConnection);

    mpOutputList->clear();
    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpOutputList->insertTopLevelItem(0, pComponentItem);
        QList<Port*> ports = pSystem->getModelObject(componentNames.at(c))->getPortListPtrs();
        for(int p=0; p<ports.size(); ++p)
        {
            QTreeWidgetItem *pPortItem = new QTreeWidgetItem(QStringList() << ports.at(p)->getPortName());
            QVector<QString> portNames, portUnits;
            pSystem->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(componentNames.at(c), ports.at(p)->getPortName(), portNames, portUnits);
            for(int v=0; v<portNames.size(); ++v)
            {
                QTreeWidgetItem *pVariableItem = new QTreeWidgetItem(QStringList() << portNames.at(v));
                pVariableItem->setCheckState(0, Qt::Unchecked);
                pPortItem->insertChild(0, pVariableItem);
            }
            pComponentItem->insertChild(0, pPortItem);
        }
    }
    connect(mpOutputList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(updateChosenVariables(QTreeWidgetItem*,int)), Qt::UniqueConnection);

    mOutputVariables.clear();
    mSelectedParameters.clear();
    mSelectedComponents.clear();

    QDialog::show();
}


void SensitivityAnalysisDialog::updateChosenParameters(QTreeWidgetItem* item, int /*i*/)
{
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedComponents.append(item->parent()->text(0));
        mSelectedParameters.append(item->text(0));
        QLabel *pLabel = new QLabel(item->parent()->text(0) + ", " + item->text(0) + ": ");
        //pLabel->setAlignment(Qt::AlignCenter);
        QString averageValue = gpMainWindow->mpProjectTabs->getCurrentContainer()->getModelObject(item->parent()->text(0))->getParameterValue(item->text(0));
        QLineEdit *pAverageLineEdit = new QLineEdit(averageValue, this);
        QLineEdit *pSigmaLineEdit = new QLineEdit("0.0", this);
        pSigmaLineEdit->setValidator(new QDoubleValidator());
        mpParameterLabels.append(pLabel);
        mpParameterAverageLineEdits.append(pAverageLineEdit);
        mpParameterSigmaLineEdits.append(pSigmaLineEdit);

        int row = mpParametersLayout->rowCount();
        mpParametersLayout->addWidget(pLabel, row, 0);
        mpParametersLayout->addWidget(pAverageLineEdit, row, 1);
        mpParametersLayout->addWidget(pSigmaLineEdit, row, 2);
    }
    else
    {
        int i=0;
        for(; i<mSelectedParameters.size(); ++i)
        {
            if(mSelectedComponents.at(i) == item->parent()->text(0) &&
               mSelectedParameters.at(i) == item->text(0))
            {
                break;
            }
        }
        mpParametersLayout->removeWidget(mpParameterLabels.at(i));
        mpParametersLayout->removeWidget(mpParameterAverageLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterSigmaLineEdits.at(i));
        QLabel *pParameterLabel = mpParameterLabels.at(i);
        QLineEdit *pParameterAverageLineEdit = mpParameterAverageLineEdits.at(i);
        QLineEdit *pParameterSigmaLineEdit = mpParameterSigmaLineEdits.at(i);
        mpParameterLabels.removeAt(i);
        mpParameterAverageLineEdits.removeAt(i);
        mpParameterSigmaLineEdits.removeAt(i);
        mSelectedParameters.removeAt(i);
        mSelectedComponents.removeAt(i);
        delete(pParameterLabel);
        delete(pParameterAverageLineEdit);
        delete(pParameterSigmaLineEdit);
    }
}


void SensitivityAnalysisDialog::updateChosenVariables(QTreeWidgetItem* item, int /*i*/)
{
    QStringList variable;
    variable << item->parent()->parent()->text(0) << item->parent()->text(0) << item->text(0);

    if(item->checkState(0) == Qt::Checked)
    {
        mOutputVariables.append(variable);
        QLabel *pLabel = new QLabel(variable.at(0) + ", " + variable.at(1) + ", " + variable.at(2));
        mpOutputLabels.append(pLabel);
        int row = mpOutputLayout->rowCount();
        mpOutputLayout->addWidget(pLabel, row, 0);
    }
    else
    {
        int i=0;
        for(; i<mOutputVariables.size(); ++i)
        {
            if(mOutputVariables.at(i) == variable)
            {
                break;
            }
        }
        mpOutputLayout->removeWidget(mpOutputLabels.at(i));
        QLabel *pOutputLabels = mpOutputLabels.at(i);
        mpOutputLabels.removeAt(i);
        mOutputVariables.removeAll(variable);
        delete(pOutputLabels);
    }
}


void SensitivityAnalysisDialog::run()
{
    ProjectTabWidget *pTabs = gpMainWindow->mpProjectTabs;
    int nThreads = gConfig.getNumberOfThreads();
    if(nThreads == 0)
    {
#ifdef WIN32
        std::string temp = getenv("NUMBER_OF_PROCESSORS");
        nThreads = atoi(temp.c_str());
#else
        nThreads = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    }
    if(nThreads == 0)   //Extra check, just to be sure
    {
        nThreads = 1;
    }

    int nSteps = mpStepsSpinBox->value();
    int nParameteres = mSelectedParameters.size();

    if(gConfig.getUseMulticore())
    {
        //Close all other containers
        for(int i=0; i<pTabs->count(); ++i)
        {
            if(pTabs->getContainer(i) != pTabs->getCurrentContainer())
            {
                pTabs->closeProjectTab(i);
                --i;
            }
        }

        //Load more tabs with same model
        for(int i=1; i<nThreads; ++i)
        {
            pTabs->loadModel(pTabs->getCurrentContainer()->getModelFileInfo().absoluteFilePath(), true);
        }
    }
    else
    {
        nThreads = 1;
    }

    int nTabs = pTabs->count();

    if(gConfig.getUseMulticore())
    {
        bool noChange=false;
        for(int i=0; i<nSteps/nThreads; ++i)
        {
            for(int t=0; t<nTabs; ++t)
            {
                for(int p=0; p<nParameteres; ++p)
                {
                    double randPar = normalDistribution(mpParameterAverageLineEdits.at(p)->text().toDouble(), mpParameterSigmaLineEdits.at(p)->text().toDouble());
                    pTabs->getContainer(t)->getModelObject(mSelectedComponents.at(p))->setParameterValue(mSelectedParameters.at(p), QString().setNum(randPar));
                }
            }
            pTabs->simulateAllOpenModels_blocking(noChange);
            noChange=true;
        }
    }
    else
    {
        for(int i=0; i<nSteps; ++i)
        {
            for(int p=0; p<nParameteres; ++p)
            {
                double randPar = normalDistribution(mpParameterAverageLineEdits.at(p)->text().toDouble(), mpParameterSigmaLineEdits.at(p)->text().toDouble());
                pTabs->getCurrentContainer()->getModelObject(mSelectedComponents.at(p))->setParameterValue(mSelectedParameters.at(p), QString().setNum(randPar));
            }
            pTabs->getCurrentTab()->simulate_blocking();
        }
    }

    if(gConfig.getUseMulticore())
    {
        for(int v=0; v<mOutputVariables.size(); ++v)
        {
            pTabs->setCurrentIndex(0);

            QString component = mOutputVariables.at(v).at(0);
            QString port = mOutputVariables.at(v).at(1);
            QString variable = mOutputVariables.at(v).at(2);

            QString fullName = makeConcatName(component,port,variable);

            PlotWindow *pPlotWindow = pTabs->getContainer(0)->getModelObject(component)->getPort(port)->plot(variable, QString(), QColor("Blue"));
            pPlotWindow->hideCurveInfo();
            pPlotWindow->setLegendsVisible(false);

            int nGenerations = pTabs->getContainer(0)->getPlotDataPtr()->getLatestGeneration()+1;
            for(int g=nGenerations-nSteps/nThreads; g<nGenerations-1; ++g)
            {
                pTabs->getContainer(0)->getPlotDataPtr()->plotToWindow(fullName, g, QwtPlot::yLeft, pPlotWindow, QColor("Blue"));
                //gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->addPlotCurve(g, component, port, variable, QString(), QwtPlot::yLeft, QString(), QColor("Blue"));
            }

            //! @todo Why is there two loop bellow why not begin at tab 0 and jsut have one, why nGen-1 on the first one
            for(int t=1; t<nTabs; ++t)
            {
                pTabs->setCurrentIndex(t);
                for(int g=nGenerations-nSteps/nThreads; g<nGenerations; ++g)
                {
                    pTabs->getContainer(t)->getPlotDataPtr()->plotToWindow(fullName, g, QwtPlot::yLeft, pPlotWindow, QColor("Blue"));
                    //gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->addPlotCurve(g, component, port, variable, QString(), QwtPlot::yLeft, QString(), QColor("Blue"));
                }
            }
        }
    }
    else
    {
        for(int v=0; v<mOutputVariables.size(); ++v)
        {
            QString component = mOutputVariables.at(v).at(0);
            QString port = mOutputVariables.at(v).at(1);
            QString variable = mOutputVariables.at(v).at(2);
            PlotWindow *pPlotWindow = pTabs->getCurrentContainer()->getModelObject(component)->getPort(port)->plot(variable, QString(), QColor("Blue"));
            pPlotWindow->hideCurveInfo();
            pPlotWindow->setLegendsVisible(false);

            QString fullName = makeConcatName(component,port,variable);
            int nGenerations = pTabs->getContainer(0)->getPlotDataPtr()->getLatestGeneration()+1;
            for(int g=nGenerations - nSteps; g<nGenerations; ++g)
            {
                pTabs->getCurrentContainer()->getPlotDataPtr()->plotToWindow(fullName, g, QwtPlot::yLeft, pPlotWindow, QColor("Blue"));
                //gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->addPlotCurve(g, component, port, variable, QString(), QwtPlot::yLeft, QString(), QColor("Blue"));
            }
        }
    }
}
