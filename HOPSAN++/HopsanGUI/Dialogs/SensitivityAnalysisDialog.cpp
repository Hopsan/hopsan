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

    //Main layout
    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpParametersGroupBox,   0, 1);
    mpLayout->addWidget(mpOutputGroupBox,       1, 1);
    mpLayout->addWidget(mpStepsWidget,          2, 1);
    mpLayout->addWidget(mpButtonBox,            3, 1);
    setLayout(mpLayout);

    //Connections
    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpRunButton,                    SIGNAL(clicked()),      this,                   SLOT(run()));
}


//! @brief Reimplementation of open() slot, used to initialize the dialog
void SensitivityAnalysisDialog::open()
{
    mpParametersList->clear();
    GUISystem *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    QStringList componentNames = pSystem->getGUIModelObjectNames();
    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpParametersList->insertTopLevelItem(0, pComponentItem);
        QStringList parameterNames = pSystem->getGUIModelObject(componentNames.at(c))->getParameterNames();
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
        QList<GUIPort*> ports = pSystem->getGUIModelObject(componentNames.at(c))->getPortListPtrs();
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
        QString averageValue = gpMainWindow->mpProjectTabs->getCurrentContainer()->getGUIModelObject(item->parent()->text(0))->getParameterValue(item->text(0));
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
    int nSteps = mpStepsSpinBox->value();
    int nParameteres = mSelectedParameters.size();

    if(gConfig.getUseMulticore())
    {
        for(int i=1; i<nThreads; ++i)
        {
            pTabs->loadModel(pTabs->getCurrentContainer()->getModelFileInfo().absoluteFilePath(), true);
        }
    }

    int nTabs = pTabs->count();

    bool noChange=false;
    for(int i=1; i<nSteps/nThreads; ++i)
    {
        for(int t=0; t<nTabs; ++t)
        {
            for(int p=0; p<nParameteres; ++p)
            {
                double randPar = normalDistribution(mpParameterAverageLineEdits.at(p)->text().toDouble(), mpParameterSigmaLineEdits.at(p)->text().toDouble());
                pTabs->getContainer(t)->getGUIModelObject(mSelectedComponents.at(p))->setParameterValue(mSelectedParameters.at(p), QString().setNum(randPar));
            }
        }
        pTabs->simulateAllOpenModelsWithoutSplit(noChange);
        noChange=true;
    }

    for(int v=0; v<mOutputVariables.size(); ++v)
    {
        pTabs->setCurrentIndex(0);

        QString component = mOutputVariables.at(v).at(0);
        QString port = mOutputVariables.at(v).at(1);
        QString variable = mOutputVariables.at(v).at(2);
        pTabs->getContainer(0)->getGUIModelObject(component)->getPort(port)->plot(variable);
        gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->hideCurveInfo();
        gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->setLegendsVisible(false);

        for(int g=1; g<pTabs->getContainer(0)->getNumberOfPlotGenerations(); ++g)
        {
            gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->addPlotCurve(g, component, port, variable);
        }

        for(int t=1; t<nTabs; ++t)
        {
            pTabs->setCurrentIndex(t);
            for(int g=0; g<pTabs->getContainer(t)->getNumberOfPlotGenerations(); ++g)
            {
                gpMainWindow->mpPlotWidget->mpPlotVariableTree->getLastPlotWindow()->addPlotCurve(g, component, port, variable);
            }
        }


    }
}

