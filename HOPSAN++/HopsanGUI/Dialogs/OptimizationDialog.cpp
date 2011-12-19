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
//! @file   OptimizationDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-10-24
//!
//! @brief Contains a class for the optimization dialog
//!
//$Id$

#include <QDebug>
#include <limits>
#include "Configuration.h"
#include "GUIPort.h"
#include "Dialogs/OptimizationDialog.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PyDockWidget.h"

class ProjectTabWidget;


//! @brief Constructor
OptimizationDialog::OptimizationDialog(MainWindow *parent)
    : QDialog(parent)
{
        //Set the name and size of the main window
    this->resize(640,480);
    this->setWindowTitle("Optimization");
    this->setPalette(gConfig.getPalette());

    //Settings tab
    mpSettingsLabel = new QLabel("Please choose general settings for optimization algorithm.");
    QFont boldFont = mpSettingsLabel->font();
    boldFont.setBold(true);
    mpSettingsLabel->setFont(boldFont);

    mpIterationsLabel = new QLabel("Number of iterations:");
    mpIterationsSpinBox = new QSpinBox(this);
    mpIterationsSpinBox->setRange(0, std::numeric_limits<int>::max());
    mpIterationsSpinBox->setValue(100);

    mpSearchPointsLabel = new QLabel("Number of search points:" );
    mpSearchPointsSpinBox = new QSpinBox(this);
    mpSearchPointsSpinBox->setRange(1, std::numeric_limits<int>::max());
    mpSearchPointsSpinBox->setValue(8);

    mpAlphaLabel = new QLabel("Reflection coefficient: ");
    mpAlphaLineEdit = new QLineEdit("1.3", this);
    mpAlphaLineEdit->setValidator(new QDoubleValidator());

    mpBetaLabel = new QLabel("Randomization factor: ");
    mpBetaLineEdit = new QLineEdit("0.3", this);
    mpBetaLineEdit->setValidator(new QDoubleValidator());

    mpGammaLabel = new QLabel("Forgetting factor: ");
    mpGammaLineEdit = new QLineEdit("0.3", this);
    mpGammaLineEdit->setValidator(new QDoubleValidator());

    mpEpsilonFLabel = new QLabel("Tolerance for function convergence: ");
    mpEpsilonFLineEdit = new QLineEdit("0.00001", this);
    mpEpsilonFLineEdit->setValidator(new QDoubleValidator());

    mpEpsilonXLabel = new QLabel("Tolerance for parameter convergence: ");
    mpEpsilonXLineEdit = new QLineEdit("0.0001", this);
    mpEpsilonXLineEdit->setValidator(new QDoubleValidator());

    mpMultiThreadedCheckBox = new QCheckBox("Multi-threaded optimization", this);
    mpMultiThreadedCheckBox->setChecked(gConfig.getUseMulticore());

    mpThreadsLabel = new QLabel("Number of threads: ");
    mpThreadsSpinBox = new QSpinBox(this);
    mpThreadsSpinBox->setValue(gConfig.getNumberOfThreads());
    mpThreadsSpinBox->setRange(1,1000000000);
    mpThreadsSpinBox->setSingleStep(1);
    mpThreadsSpinBox->setSingleStep(1);
    mpThreadsLabel->setEnabled(mpMultiThreadedCheckBox->isChecked());
    mpThreadsSpinBox->setEnabled(mpMultiThreadedCheckBox->isChecked());
    connect(mpMultiThreadedCheckBox, SIGNAL(toggled(bool)), mpThreadsLabel, SLOT(setEnabled(bool)));
    connect(mpMultiThreadedCheckBox, SIGNAL(toggled(bool)), mpThreadsSpinBox, SLOT(setEnabled(bool)));

    mpPlottingCheckBox = new QCheckBox("Plot each iteration", this);
    mpPlottingCheckBox->setChecked(true);

    mpExport2CSVBox= new QCheckBox("Export trace data to CSV file", this);
    mpExport2CSVBox->setChecked(false);

    mpSettingsLayout = new QGridLayout(this);
    mpSettingsLayout->addWidget(mpSettingsLabel,        0, 0);
    mpSettingsLayout->addWidget(mpIterationsLabel,      1, 0);
    mpSettingsLayout->addWidget(mpIterationsSpinBox,    1, 1);
    mpSettingsLayout->addWidget(mpSearchPointsLabel,    2, 0);
    mpSettingsLayout->addWidget(mpSearchPointsSpinBox,  2, 1);
    mpSettingsLayout->addWidget(mpAlphaLabel,           3, 0);
    mpSettingsLayout->addWidget(mpAlphaLineEdit,         3, 1);
    mpSettingsLayout->addWidget(mpBetaLabel,            4, 0);
    mpSettingsLayout->addWidget(mpBetaLineEdit,          4, 1);
    mpSettingsLayout->addWidget(mpGammaLabel,           5, 0);
    mpSettingsLayout->addWidget(mpGammaLineEdit,         5, 1);
    mpSettingsLayout->addWidget(mpEpsilonFLabel,        6, 0);
    mpSettingsLayout->addWidget(mpEpsilonFLineEdit,      6, 1);
    mpSettingsLayout->addWidget(mpEpsilonXLabel,        7, 0);
    mpSettingsLayout->addWidget(mpEpsilonXLineEdit,      7, 1);
    mpSettingsLayout->addWidget(mpMultiThreadedCheckBox, 8, 0);
    mpSettingsLayout->addWidget(mpThreadsLabel,          9, 0);
    mpSettingsLayout->addWidget(mpThreadsSpinBox,        9, 1);
    mpSettingsLayout->addWidget(mpPlottingCheckBox,      10, 0, 1, 2);
    mpSettingsLayout->addWidget(mpExport2CSVBox,         11, 0, 1, 2);
    mpSettingsLayout->addWidget(new QWidget(this),       12, 0, 1, 2);    //Dummy widget for stretching the layout
    mpSettingsLayout->setRowStretch(0, 0);
    mpSettingsLayout->setRowStretch(1, 0);
    mpSettingsLayout->setRowStretch(2, 0);
    mpSettingsLayout->setRowStretch(3, 0);
    mpSettingsLayout->setRowStretch(4, 0);
    mpSettingsLayout->setRowStretch(4, 0);
    mpSettingsLayout->setRowStretch(6, 0);
    mpSettingsLayout->setRowStretch(7, 0);
    mpSettingsLayout->setRowStretch(8, 0);
    mpSettingsLayout->setRowStretch(9, 1);
    mpSettingsLayout->setRowStretch(10, 1);
    mpSettingsWidget = new QWidget(this);
    mpSettingsWidget->setLayout(mpSettingsLayout);

    //Parameter tab
    mpParametersLabel = new QLabel("Choose optimization parameters, and specify their minimum and maximum values.");
    mpParametersLabel->setFont(boldFont);
    mpParametersLogCheckBox = new QCheckBox("Use logarithmic parameter scaling", this);
    mpParametersLogCheckBox->setChecked(false);
    mpParametersList = new QTreeWidget(this);
    mpParameterMinLabel = new QLabel("Min Value");
    mpParameterMinLabel->setAlignment(Qt::AlignCenter);
    mpParameterNameLabel = new QLabel("Parameter Name");
    mpParameterNameLabel->setAlignment(Qt::AlignCenter);
    mpParameterMaxLabel = new QLabel("Max Value");
    mpParameterMaxLabel->setAlignment(Qt::AlignCenter);
    mpParameterMinLabel->setFont(boldFont);
    mpParameterNameLabel->setFont(boldFont);
    mpParameterMaxLabel->setFont(boldFont);
    mpParametersLayout = new QGridLayout(this);
    mpParametersLayout->addWidget(mpParametersLabel,        0, 0, 1, 4);
    mpParametersLayout->addWidget(mpParametersLogCheckBox,  1, 0, 1, 4);
    mpParametersLayout->addWidget(mpParametersList,         2, 0, 1, 4);
    mpParametersLayout->addWidget(mpParameterMinLabel,      3, 0, 1, 1);
    mpParametersLayout->addWidget(mpParameterNameLabel,     3, 1, 1, 1);
    mpParametersLayout->addWidget(mpParameterMaxLabel,      3, 2, 1, 1);
    mpParametersWidget = new QWidget(this);
    mpParametersWidget->setLayout(mpParametersLayout);



    //Objective function tab
    mpObjectiveLabel = new QLabel("Create an objective function by first choosing variables in the list and then choosing a function below.");
    mpObjectiveLabel->setFont(boldFont);
    mpVariablesList = new QTreeWidget(this);
    mpMinMaxComboBox = new QComboBox(this);
    mpMinMaxComboBox->addItems(QStringList() << "Minimize" << "Maximize");
    mpFunctionsComboBox = new QComboBox(this);
    mpAddFunctionButton = new QPushButton("Add Function");
    mpWeightLabel = new QLabel("Weight");
    mpNormLabel = new QLabel("Norm. Factor");
    mpExpLabel = new QLabel("Exp. Factor");
    mpDescriptionLabel = new QLabel("Description");
    mpDataLabel = new QLabel("Data");
    mpWeightLabel->setFont(boldFont);
    mpNormLabel->setFont(boldFont);
    mpExpLabel->setFont(boldFont);
    mpDescriptionLabel->setFont(boldFont);
    mpDataLabel->setFont(boldFont);
    mpObjectiveLayout = new QGridLayout(this);
    mpObjectiveLayout->addWidget(mpObjectiveLabel,          0, 0, 1, 7);
    mpObjectiveLayout->addWidget(mpVariablesList,           1, 0, 1, 7);
    mpObjectiveLayout->addWidget(mpMinMaxComboBox,          2, 0, 1, 1);
    mpObjectiveLayout->addWidget(mpFunctionsComboBox,       2, 1, 1, 4);
    mpObjectiveLayout->addWidget(mpAddFunctionButton,       2, 5, 1, 2);
    mpObjectiveLayout->addWidget(mpWeightLabel,             3, 0, 1, 1);
    mpObjectiveLayout->addWidget(mpNormLabel,               3, 1, 1, 1);
    mpObjectiveLayout->addWidget(mpExpLabel,                3, 2, 1, 1);
    mpObjectiveLayout->addWidget(mpDescriptionLabel,        3, 3, 1, 2);
    mpObjectiveLayout->addWidget(mpDataLabel,               3, 5, 1, 2);
    mpObjectiveLayout->addWidget(new QWidget(this),         4, 0, 1, 7);
    mpObjectiveLayout->setRowStretch(0, 0);
    mpObjectiveLayout->setRowStretch(1, 0);
    mpObjectiveLayout->setRowStretch(2, 0);
    mpObjectiveLayout->setRowStretch(3, 0);
    mpObjectiveLayout->setRowStretch(4, 1);
    mpObjectiveLayout->setColumnStretch(0, 0);
    mpObjectiveLayout->setColumnStretch(1, 0);
    mpObjectiveLayout->setColumnStretch(2, 0);
    mpObjectiveLayout->setColumnStretch(3, 0);
    mpObjectiveLayout->setColumnStretch(4, 1);
    mpObjectiveLayout->setColumnStretch(5, 0);
    mpObjectiveLayout->setColumnStretch(6, 0);
    mpObjectiveWidget = new QWidget(this);
    mpObjectiveWidget->setLayout(mpObjectiveLayout);

    //Output box tab
    mpOutputBox = new QTextEdit(this);
    QFont monoFont = mpOutputBox->font();
    monoFont.setFamily("Courier");
    mpOutputBox->setFont(monoFont);
    mpOutputBox->setMinimumWidth(450);
    mpOutputLayout = new QGridLayout(this);
    mpOutputLayout->addWidget(mpOutputBox);
    mpOutputWidget = new QWidget(this);
    mpOutputWidget->setLayout(mpOutputLayout);

    //Tab widget
    mpTabWidget = new QTabWidget(this);
    mpTabWidget->addTab(mpSettingsWidget, "Settings");
    mpTabWidget->addTab(mpParametersWidget, "Parameters");
    mpTabWidget->addTab(mpObjectiveWidget, "Objective Function");
    mpTabWidget->addTab(mpOutputWidget, "Output Code");

    //Buttons
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    mpGenerateButton = new QPushButton(tr("&Generate Script"), this);
    mpGenerateButton->setDefault(true);
    mpRunButton = new QPushButton(tr("&Run Optimization"), this);
    mpRunButton->setDefault(true);
    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpGenerateButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpRunButton, QDialogButtonBox::ActionRole);

    //Main layout
    QGridLayout *pLayout = new QGridLayout;
    pLayout->addWidget(mpTabWidget, 1, 1);
    //pLayout->addWidget(mpOutputBox, 1, 3, 1, 1);
    pLayout->addWidget(mpButtonBox, 2, 1);
    setLayout(pLayout);

    //Connections
    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpGenerateButton,               SIGNAL(clicked()),      this,                   SLOT(updateOutputBox()));
    connect(mpRunButton,                    SIGNAL(clicked()),      this,                   SLOT(run()));
    connect(mpAddFunctionButton,            SIGNAL(clicked()),      this,                   SLOT(addFunction()));
}


void OptimizationDialog::loadConfiguration()
{
    GUISystem *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();

    OptimizationSettings optSettings = pSystem->getOptimizationSettings();

    mpIterationsSpinBox->setValue(optSettings.mNiter);
    mpSearchPointsSpinBox->setValue(optSettings.mNsearchp);
    mpAlphaLineEdit->setText(QString().setNum(optSettings.mRefcoeff));
    mpBetaLineEdit->setText(QString().setNum(optSettings.mRandfac));
    mpGammaLineEdit->setText(QString().setNum(optSettings.mForgfac));
    mpEpsilonFLineEdit->setText(QString().setNum(optSettings.mFunctol));
    mpEpsilonXLineEdit->setText(QString().setNum(optSettings.mPartol));
    mpPlottingCheckBox->setChecked(optSettings.mPlot);
    mpExport2CSVBox->setChecked(optSettings.mSavecsv);
    mpParametersLogCheckBox->setChecked(optSettings.mlogPar);

    //Parameters
    for(int i=0; i<optSettings.mParamters.size(); ++i)
    {
        findParameterTreeItem(optSettings.mParamters.at(i).mComponentName, optSettings.mParamters.at(i).mParameterName)->setCheckState(0, Qt::Checked);
    }
    //Objectives
    for(int i=0; i<optSettings.mObjectives.size(); ++i)
    {
        //! @todo Find a good way of setting the objective functions

        int idx = mpFunctionsComboBox->findText(optSettings.mObjectives.at(i).mFunctionName);
        if(idx > -1) //found!
        {//Lägg till variabel i XML -> compname, portname, varname, ska vara i mSelectedVariables
            mpFunctionsComboBox->setCurrentIndex(idx);
            addObjectiveFunction(idx, optSettings.mObjectives.at(i).mWeight, optSettings.mObjectives.at(i).mNorm, optSettings.mObjectives.at(i).mExp, optSettings.mObjectives.at(i).mVariableInfo, optSettings.mObjectives.at(i).mData);
        }
    }
}


void OptimizationDialog::saveConfiguration()
{
    OptimizationSettings optSettings;

    //Settings
    optSettings.mNiter = mpIterationsSpinBox->value();
    optSettings.mNsearchp = mpSearchPointsSpinBox->value();
    optSettings.mRefcoeff = mpAlphaLineEdit->text().toDouble();
    optSettings.mRandfac = mpBetaLineEdit->text().toDouble();
    optSettings.mForgfac = mpGammaLineEdit->text().toDouble();
    optSettings.mFunctol = mpEpsilonFLineEdit->text().toDouble();
    optSettings.mPartol = mpEpsilonXLineEdit->text().toDouble();
    optSettings.mPlot = mpPlottingCheckBox->isChecked();
    optSettings.mSavecsv = mpExport2CSVBox->isChecked();
    optSettings.mlogPar = mpParametersLogCheckBox->isChecked();

    //Parameters
    for(int i=0; i < mpParameterLabels.size(); ++i)
    {
        qDebug() << mSelectedParameters.at(i) << "   " << mSelectedComponents.at(i) << "   " << mpParameterMinLineEdits.at(i)->text();
        OptParameter parameter;
        parameter.mComponentName = mSelectedComponents.at(i);
        parameter.mParameterName = mSelectedParameters.at(i);
        parameter.mMax = mpParameterMaxLineEdits.at(i)->text().toDouble();
        parameter.mMin = mpParameterMinLineEdits.at(i)->text().toDouble();
        optSettings.mParamters.append(parameter);
    }
    //Objective functions
    for(int i=0; i < mWeightLineEditPtrs.size(); ++i)
    {
        Objectives objective;
        objective.mFunctionName = mFunctionName.at(i);
        objective.mWeight = mWeightLineEditPtrs.at(i)->text().toDouble();
        objective.mNorm   = mNormLineEditPtrs.at(i)->text().toDouble();
        objective.mExp    = mExpLineEditPtrs.at(i)->text().toDouble();

        QStringList variableInfo;
        for(int j=0; j < mFunctionComponents.at(i).size(); ++j)
        {
            variableInfo << mFunctionComponents.at(i).at(j);
            variableInfo << mFunctionPorts.at(i).at(j);
            variableInfo << mFunctionVariables.at(i).at(j);
        }
        objective.mVariableInfo.append(variableInfo);

        for(int j=0; j < mDataLineEditPtrs.at(i).size(); ++j)
        {
            objective.mData.append(mDataLineEditPtrs.at(i).at(j)->text());
        }
        optSettings.mObjectives.append(objective);
    }



    GUISystem *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    pSystem->setOptimizationSettings(optSettings);
}


//! @brief Reimplementation of open() slot, used to initialize the dialog
void OptimizationDialog::open()
{
    //Load the objective functions
    if(!loadObjectiveFunctions())
        return;

    mpFunctionsComboBox->clear();
    mpFunctionsComboBox->addItems(mObjectiveFunctionDescriptions);

    //Clear all parameters
    for(int c=0; c<mpParametersList->topLevelItemCount(); ++c)      //Uncheck all parameters (will "remove" them)
    {
        for(int p=0; p<mpParametersList->topLevelItem(c)->childCount(); ++p)
        {
            if(mpParametersList->topLevelItem(c)->child(p)->checkState(0) == Qt::Checked)
            {
                mpParametersList->topLevelItem(c)->child(p)->setCheckState(0, Qt::Unchecked);
            }
        }
    }
    mpParametersList->clear();

    //Populate parameters list
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

    //Clear all objective functions
    mpVariablesList->clear();
    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpVariablesList->insertTopLevelItem(0, pComponentItem);
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
    connect(mpVariablesList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(updateChosenVariables(QTreeWidgetItem*,int)), Qt::UniqueConnection);

    mSelectedVariables.clear();
    mSelectedFunctionsMinMax.clear();
    mSelectedFunctions.clear();
    mSelectedParameters.clear();
    mSelectedComponents.clear();

    for(int i=0; i<mWeightLineEditPtrs.size(); ++i)
    {
        mpObjectiveLayout->removeWidget(mWeightLineEditPtrs.at(i));
        mpObjectiveLayout->removeWidget(mNormLineEditPtrs.at(i));
        mpObjectiveLayout->removeWidget(mExpLineEditPtrs.at(i));
        mpObjectiveLayout->removeWidget(mFunctionLabelPtrs.at(i));
        mpObjectiveLayout->removeWidget(mRemoveFunctionButtonPtrs.at(i));
        mpObjectiveLayout->removeWidget(mDataWidgetPtrs.at(i));

        for(int j=0; j<mDataLineEditPtrs.at(i).size(); ++j)
        {
            delete(mDataLineEditPtrs.at(i).at(j));
        }
        delete(mWeightLineEditPtrs.at(i));
        delete(mNormLineEditPtrs.at(i));
        delete(mExpLineEditPtrs.at(i));
        delete(mFunctionLabelPtrs.at(i));
        delete(mRemoveFunctionButtonPtrs.at(i));
        delete(mDataWidgetPtrs.at(i));
    }

    mWeightLineEditPtrs.clear();
    mNormLineEditPtrs.clear();
    mExpLineEditPtrs.clear();
    mFunctionLabelPtrs.clear();
    mFunctionName.clear();
    mRemoveFunctionButtonPtrs.clear();
    mDataLineEditPtrs.clear();
    mDataWidgetPtrs.clear();
    mFunctionComponents.clear();
    mFunctionPorts.clear();
    mFunctionVariables.clear();

    for(int i=0; i<mpParameterLabels.size(); ++i)
    {
        mpParametersLayout->removeWidget(mpParameterLabels.at(i));
        mpParametersLayout->removeWidget(mpParameterMinLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterMaxLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterRemoveButtons.at(i));

        delete(mpParameterLabels.at(i));
        delete(mpParameterMinLineEdits.at(i));
        delete(mpParameterMaxLineEdits.at(i));
        delete(mpParameterRemoveButtons.at(i));
    }

    mpParameterLabels.clear();
    mpParameterMinLineEdits.clear();
    mpParameterMaxLineEdits.clear();
    mpParameterRemoveButtons.clear();

    mScript.clear();
    mpOutputBox->clear();
    mpRunButton->setDisabled(true);

    loadConfiguration();

    QDialog::show();
}


void OptimizationDialog::reject()
{
    saveConfiguration();

    QDialog::reject();
}


//! @brief Generates the Python script based upon selections made in the dialog
void OptimizationDialog::generateScriptFile()
{
    bool multicore = mpMultiThreadedCheckBox->isChecked();
    int nThreads = mpThreadsSpinBox->value();

    if(mSelectedParameters.isEmpty())
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("No parameters specified for optimization.");
        return;
    }

    if(mSelectedFunctions.isEmpty())
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("No objective functions specified for optimization.");
        return;
    }

    QString iterationsString = QString().setNum(mpIterationsSpinBox->value());
    QString alphaString = QString().setNum(mpAlphaLineEdit->text().toDouble());
    QString betaString = QString().setNum(mpBetaLineEdit->text().toDouble());
    QString gammaString = QString().setNum(mpGammaLineEdit->text().toDouble());
    QString nParString = QString().setNum(mpParameterLabels.size());
    mScript.clear();

    QTextStream scriptStream(&mScript);

    scriptStream << "# coding=utf-8\n";
    scriptStream << "################################################################################\n";
    scriptStream << "## This Python script is automatic generated by Hopsan NG                     ##\n";
    scriptStream << "##                                                                            ##\n";

    //Maybe there should not be a copyright on the generated script file??

    scriptStream << "## Copyright (c) " << QDate::currentDate().year() << "                                                         ##\n";
    scriptStream << "##    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Bjorn Eriksson,       ##\n";
    scriptStream << "##    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack                 ##\n";
    scriptStream << "##                                                                            ##\n";
    scriptStream << "## This file is provided \"as is\", with no guarantee or warranty for the     ##\n";
    scriptStream << "## functionality or reliability of the contents. All contents in this file is ##\n";
    scriptStream << "## the original work of the copyright holders at the Division of Fluid and    ##\n";
    scriptStream << "## Mechatronic Systems (Flumes) at Linkoping University. Modifying, using or  ##\n";
    scriptStream << "## redistributing any part of this file is prohibited without explicit        ##\n";
    scriptStream << "## permission from the copyright holders.                                     ##\n";
    scriptStream << "################################################################################\n";
    scriptStream << "##########################################################\n";
    scriptStream << "## Complex Optimization Script for Hopsan               ##\n";
    scriptStream << "##                                                      ##\n";
    scriptStream << "## Export routine written by Robert Braun, October 2011 ##\n";
    scriptStream << "##########################################################\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Import required packages #####\n";
    scriptStream << "\n";
    scriptStream << "import sys\n";
    scriptStream << "import time\n";
    scriptStream << "\n";
    scriptStream << "startTime = time.time()\n";
    scriptStream << "\n";
    scriptStream << "sys.path.append('" << gExecPath+QString(SCRIPTPATH) <<"')\n";
    scriptStream << "from HopsanOptimization import *\n";
    scriptStream << "from OptimizationObjectiveFunctions import *\n";
    scriptStream << "import random\n";
    scriptStream << "from time import sleep\n";
    scriptStream << "from types import FloatType\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Simulation settings #####\n";
    scriptStream << "\n";
    scriptStream << "iterations=" << iterationsString << "\n";
    scriptStream << "hopsan.turnOffProgressBar()\n";
    scriptStream << "alpha=" << alphaString << "\n";
    scriptStream << "beta=" << betaString << "\n";
    scriptStream << "gamma=" << gammaString << "\n";
    scriptStream << "tolFunc=" << QString().setNum(mpEpsilonFLineEdit->text().toDouble()) << "\n";
    scriptStream << "tolX=" << QString().setNum(mpEpsilonXLineEdit->text().toDouble()) << "\n";
    if(multicore)
    {
        scriptStream << "nThreads=" << QString().setNum(nThreads) << "\n";
        scriptStream << "\n";
        scriptStream << "\n";
        scriptStream << "\n";
        scriptStream << "##### Load More Models #####\n";
        scriptStream << "\n";
        scriptStream << "modelPath = \"../Models/Example Models/Position Servo.hmf\"\n";
        scriptStream << "hopsan.closeAllModels()\n";
        scriptStream << "for i in range(nThreads):\n";
        scriptStream << "  hopsan.loadModel(modelPath)\n";
        scriptStream << "hopsan.useMultiCore()\n";
    }
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Optimization parameters #####\n";
    scriptStream << "\n";
    scriptStream << "parameters = [[0.0";
    for(int i=1; i<mpParameterLabels.size(); ++i)
        scriptStream << ", 0.0";
    scriptStream << "]";
    for(int i=1; i<mpSearchPointsSpinBox->value(); ++i)
    {
        scriptStream << ",\n           [0.0";
        for(int i=1; i<mpParameterLabels.size(); ++i)
            scriptStream << ", 0.0";
        scriptStream << "]";
    }
    scriptStream << "]\n";
    scriptStream << "componentNames = [\""+mSelectedComponents.at(0)+"\"";
    for(int i=1; i<mSelectedComponents.size(); ++i)
        scriptStream << ", \"" << mSelectedComponents.at(i) << "\"";
    scriptStream << "]   #Names of components where parameters are located\n";
    scriptStream << "parameterNames = [\""+mSelectedParameters.at(0)+"\"";
    for(int i=1; i<mSelectedParameters.size(); ++i)
        scriptStream << ", \"" << mSelectedParameters.at(i) << "\"";
    scriptStream << "]           #Names of parameters to optimize\n";
    scriptStream << "minValues = ["+QString().setNum(mpParameterMinLineEdits.at(0)->text().toDouble());
    for(int i=1; i<mSelectedParameters.size(); ++i)
        scriptStream << ", "+QString().setNum(mpParameterMinLineEdits.at(i)->text().toDouble());
    scriptStream << "]                    #Minimum value for each parameter\n";
    scriptStream << "maxValues = ["+QString().setNum(mpParameterMaxLineEdits.at(0)->text().toDouble());
    for(int i=1; i<mSelectedParameters.size(); ++i)
        scriptStream << ", "+QString().setNum(mpParameterMaxLineEdits.at(i)->text().toDouble());
    scriptStream << "]                    #Maximum value for each parameter\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Trace log #####\n";
    scriptStream << "\n";
    scriptStream << "trace = {}\n";
    scriptStream << "trace['parameters']=[[0]*len(parameters[0])]*iterations;\ntrace['fitness']=[0 for i in range(iterations)];\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Objective function #####\n";
    scriptStream << "\n";
    scriptStream << "obj = [0.0";
    for(int i=1; i<mpSearchPointsSpinBox->value(); ++i)
        scriptStream << ", 0.0";
    scriptStream << "]\n";
    scriptStream << "def getObjective():\n";
    for(int i=0; i<mWeightLineEditPtrs.size(); ++i)
        scriptStream << "  w"+QString().setNum(i)+"="+QString().setNum(mWeightLineEditPtrs.at(i)->text().toDouble())+"\n";
    for(int i=0; i<mNormLineEditPtrs.size(); ++i)
        scriptStream << "  n"+QString().setNum(i)+"="+QString().setNum(mNormLineEditPtrs.at(i)->text().toDouble())+"\n";
    for(int i=0; i<mExpLineEditPtrs.size(); ++i)
        scriptStream << "  g"+QString().setNum(i)+"="+QString().setNum(mExpLineEditPtrs.at(i)->text().toDouble())+"\n";

    scriptStream << "  time=hopsan.component(\""+mFunctionComponents.first().first()+"\").port(\""+mFunctionPorts.first().first()+"\").getTimeVector()\n";
    QMap<QString, QString> addedVariables;
    for(int i=0; i<mFunctionVariables.size(); ++i)
    {
        for(int j=0; j<mFunctionVariables.at(i).size(); ++j)
        {
            QString variable = "\""+mFunctionComponents.at(i).at(j)+"\").port(\""+mFunctionPorts.at(i).at(j)+"\").getDataVector(\""+mFunctionVariables.at(i).at(j)+"\"";
            QString variableId = "data"+QString().setNum(i)+QString().setNum(j);
            if(addedVariables.contains(variable))
            {
                scriptStream << "  "+variableId+"="+addedVariables.find(variable).value()+"\n";
            }
            else
            {
                addedVariables.insert(variable, variableId);
                scriptStream << "  "+variableId+"=hopsan.component("+variable+")\n";
            }
        }
    }
    scriptStream << "  return ";
    for(int i=0; i<mFunctionVariables.size(); ++i)
        scriptStream << generateFunctionCode(i);
    scriptStream << "\n\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Starting points #####\n";
    scriptStream << "\n";
    scriptStream << "for i in range(len(parameters)):\n";
    scriptStream << "  for j in range(len(parameterNames)):\n";
    scriptStream << "    parameters[i][j] = minValues[j]+(maxValues[j]-minValues[j])*random.random()\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Execute optimization #####\n";
    scriptStream << "\n";
    if(multicore)
    {
        scriptStream << "for i in range(len(parameters)):\n";
        scriptStream << "  for t in range(nThreads):\n";
        scriptStream << "    hopsan.gotoTab(t)\n";
        scriptStream << "    for j in range(len(parameterNames)):\n";
        scriptStream << "      hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[i][j])\n";
        scriptStream << "  hopsan.simulateAllOpenModels(False)\n";
        scriptStream << "  for t in range(nThreads):\n";
        scriptStream << "    obj[i] = getObjective()\n";
    }
    else
    {
          scriptStream << "for i in range(len(parameters)):\n";
          scriptStream << "  for j in range(len(parameterNames)):\n";
          scriptStream << "    hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[i][j])\n";
          scriptStream << "  hopsan.simulate()\n";
          scriptStream << "  obj[i] = getObjective()\n\n";

    }
    if(mpPlottingCheckBox->isChecked())
    {
        scriptStream << "#Run one simulation first and open plot window, if user wants to see plots in real-time\n";
        QStringList plottedVariables;
        for(int i=0; i<mFunctionVariables.size(); ++i)
        {
            for(int j=0; j<mFunctionVariables.at(i).size(); ++j)
            {
                QString plotCommand = "hopsan.plot(\""+mFunctionComponents.at(i).at(j)+"\",\""+mFunctionPorts.at(i).at(j)+"\",\""+mFunctionVariables.at(i).at(j)+"\")\n";
                if(!plottedVariables.contains(plotCommand))
                {
                    scriptStream << plotCommand;
                    plottedVariables.append(plotCommand);
                }
            }
        }
        scriptStream << "\n";
    }
    if(multicore)
    {
        scriptStream << "worstIds = indexOfMaxN(obj, nThreads)\n";
        scriptStream << "previousWorstIds = [-1 -1]\n";
        scriptStream << "previousObj = obj\n";
    }
    else
    {
        scriptStream << "worstId = indexOfMax(obj)\n";
        scriptStream << "previousWorstId = -1\n";
    }
    scriptStream << "kf=1-(alpha/2)**(gamma/(2*"+nParString+"))\n";
    scriptStream << "\n";
    scriptStream << "for k in range(iterations):\n";
    if(multicore)
    {
        //! @todo Implement logarithmic scale support for multithreaded simulations!
        scriptStream << "  for t in range(nThreads):\n";
        scriptStream << "    hopsan.gotoTab(t)\n";
        scriptStream << "    for j in range(len(parameterNames)):\n";
        scriptStream << "      hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[worstIds[t]][j])\n";
        scriptStream << "  hopsan.simulateAllOpenModels(True)\n";
        scriptStream << "  for t in range(nThreads):\n";
        scriptStream << "    hopsan.gotoTab(t)\n";
        scriptStream << "    obj[worstIds[t]] = getObjective()\n";
        scriptStream << "  objspread=max(obj)-min(obj)\n";
        scriptStream << "  for i in range(len(parameters)):\n";
        scriptStream << "    obj[i] = obj[i] + objspread*kf		#Apply forgetting factor\n";
        scriptStream << "\n";
        scriptStream << "  contracted = False				#Contract points if they were reflected last step and are worse now\n";
        scriptStream << "  for w in range(len(worstIds)):\n";
        scriptStream << "    if obj[worstIds[w]] > previousObj[worstIds[w]]:\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLogSpace(minValues)\n";
            scriptStream << "    toLogSpace(maxValues)\n";
            scriptStream << "    toLogSpace2(parameters)\n";
        }
        scriptStream << "      contract(parameters, worstIds[w], previousWorstIds,minValues,maxValues)\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLinearSpace(minValues)\n";
            scriptStream << "    toLinearSpace(maxValues)\n";
            scriptStream << "    toLinearSpace2(parameters)\n";
        }
        scriptStream << "      contracted = True\n";
        scriptStream << "\n";
        scriptStream << "  if not contracted:\n";
        scriptStream << "    worstIds = indexOfMaxN(obj, nThreads)\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLogSpace(minValues)\n";
            scriptStream << "    toLogSpace(maxValues)\n";
            scriptStream << "    toLogSpace2(parameters)\n";
        }
        scriptStream << "    reflectWorstN(parameters,worstIds,previousWorstIds,alpha,minValues,maxValues,beta)\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLinearSpace(minValues)\n";
            scriptStream << "    toLinearSpace(maxValues)\n";
            scriptStream << "    toLinearSpace2(parameters)\n";
        }
        scriptStream << "\n";
        scriptStream << "  previousWorstIds = worstIds[:]\n";
        scriptStream << "  previousObj = obj[:]\n";
    }
    else
    {
        scriptStream << "  for j in range(len(parameterNames)):\n";
        scriptStream << "    hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[worstId][j])\n";
        scriptStream << "  hopsan.simulate()\n";
        scriptStream << "  obj[worstId] = getObjective()\n";
        scriptStream << "  objspread=max(obj)-min(obj)\n";
        scriptStream << "  for i in range(len(parameters)):\n";
        scriptStream << "    obj[i] = obj[i] + objspread*kf\n";
        scriptStream << "  worstId = indexOfMax(obj)\n";
        scriptStream << "  if worstId == previousWorstId:\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLogSpace(minValues)\n";
            scriptStream << "    toLogSpace(maxValues)\n";
            scriptStream << "    toLogSpace2(parameters)\n";
        }
        scriptStream << "    reflectWorst(parameters,worstId,0.5,minValues,maxValues,beta)  #Same as previous, move halfway to centroid\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLinearSpace(minValues)\n";
            scriptStream << "    toLinearSpace(maxValues)\n";
            scriptStream << "    toLinearSpace2(parameters)\n";
        }
        scriptStream << "  else:\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLogSpace(minValues)\n";
            scriptStream << "    toLogSpace(maxValues)\n";
            scriptStream << "    toLogSpace2(parameters)\n";
        }
        scriptStream << "    reflectWorst(parameters,worstId,alpha,minValues,maxValues,beta)      #Reflect worst through centroid of the remaining points\n";
        if(mpParametersLogCheckBox->isChecked())
        {
            scriptStream << "    toLinearSpace(minValues)\n";
            scriptStream << "    toLinearSpace(maxValues)\n";
            scriptStream << "    toLinearSpace2(parameters)\n";
        }
        scriptStream << "  previousWorstId=worstId\n";
        scriptStream << "  trace['parameters'][k]=parameters[worstId]\n  trace['fitness'][k]=obj[worstId]\n";
    }
    scriptStream << "  if min(obj) == 0:\n";
    scriptStream << "    if abs(max(obj)-min(obj)) <= tolFunc:\n";
    scriptStream << "      elapsedTime = (time.time() - startTime)\n";
    scriptStream << "      print 'Converged in function values after {} iterations in {} seconds. Worst objective function value = {!r}.'.format(k, elapsedTime, max(obj))\n";
    scriptStream << "      break\n";
    scriptStream << "  elif abs(max(obj)-min(obj))/abs(min(obj)) <= tolFunc:\n";
    scriptStream << "    elapsedTime = (time.time() - startTime)\n";
    scriptStream << "    print 'Converged in function values after {} iterations in {} seconds. Worst objective function value = {!r}.'.format(k, elapsedTime, max(obj))\n";
    scriptStream << "    break\n";
    scriptStream << "  xConverged=True\n";
    scriptStream << "  for i in range(len(parameterNames)):\n";
    scriptStream << "    if abs((maxPar(parameters, i)-minPar(parameters,i))/(maxValues[i]-minValues[i])) > tolX:\n";
    scriptStream << "      xConverged=False;\n";
    scriptStream << "  if xConverged:\n";
    scriptStream << "    elapsedTime = (time.time() - startTime)\n";
    scriptStream << "    print 'Converged in parameter values after {} iterations in {} seconds. Worst objective function value = {!r}.'.format(k, elapsedTime, max(obj))\n";
    scriptStream << "    break\n";
    scriptStream << "\n";
    if(!mpPlottingCheckBox->isChecked())
    {
        scriptStream << "#Plot when simulation is finished\n";
        for(int i=0; i<mFunctionVariables.size(); ++i)
            for(int j=0; j<mFunctionVariables.at(i).size(); ++j)
                scriptStream << "hopsan.plot(\""+mFunctionComponents.at(i).at(j)+"\",\""+mFunctionPorts.at(i).at(j)+"\",\""+mFunctionVariables.at(i).at(j)+"\")\n";
        scriptStream << "\n";
    }
    if(mpExport2CSVBox->isChecked())
    {
        scriptStream << "\n";
        scriptStream << "\n";
        scriptStream << "\n";
        scriptStream << "##### Save trace log to file #####\n";
        scriptStream << "\n";
        scriptStream << "import csv\n\n";
        scriptStream << "fl = open('";
        scriptStream << generateFileName().replace("Script", "Result").replace(".py",".csv");
        scriptStream << "', 'wb')\n";
        scriptStream << "writer = csv.writer(fl,dialect='excel',delimiter=';')\n";
        scriptStream << "writer.writerow([";
        for(int i=0; i<mSelectedComponents.size(); ++i)
            scriptStream << "'" << mSelectedComponents.at(i) << "::" << mSelectedParameters.at(i) << "',";
        scriptStream << "'ObjectiveFunction'])\n";
        scriptStream << "data=[[0]*(len(trace['parameters'][0])+len([trace['fitness'][0]])) for i in range(iterations)]\n";
        scriptStream << "parI=range(len(trace['parameters'][0]))\n";
        scriptStream << "objI=range(len(trace['parameters'][0]),len(trace['parameters'][0])+len([trace['fitness'][0]]))\n";
        scriptStream << "for i in range(len(trace['parameters'])):\n";
        scriptStream << "  for j in range(len(trace['parameters'][0])):\n";
        scriptStream << "    data[i][j]=trace['parameters'][i][j]\n";
        scriptStream << "    data[i][j+1]=trace['fitness'][i]\n";
        scriptStream << "for values in data:\n";
        scriptStream << "    writer.writerow(values)\n";
        scriptStream << "fl.close()\n";
    }
}


//! @brief Adds a new parameter to the list of selected parameter, and displays it in dialog
//! @param item Tree widget item which represents parameter
void OptimizationDialog::updateChosenParameters(QTreeWidgetItem* item, int /*i*/)
{
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedComponents.append(item->parent()->text(0));
        mSelectedParameters.append(item->text(0));
        QLabel *pLabel = new QLabel(trUtf8(" <  ") + item->parent()->text(0) + ", " + item->text(0) + trUtf8("  < "));
        pLabel->setAlignment(Qt::AlignCenter);

        GUISystem *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
        OptimizationSettings optSettings = pSystem->getOptimizationSettings();
        QString min, max;
        for(int i=0; i<optSettings.mParamters.size(); ++i)
        {
            if(item->parent()->text(0) == optSettings.mParamters.at(i).mComponentName)
            {
                if(item->text(0) == optSettings.mParamters.at(i).mParameterName)
                {
                    min.setNum(optSettings.mParamters.at(i).mMin);
                    max.setNum(optSettings.mParamters.at(i).mMax);
                }
            }
        }
        if(min == "")
        {
            min = "0.0";
        }
        if(max == "")
        {
            max = "1.0";
        }


        QLineEdit *pMinLineEdit = new QLineEdit(min, this);
        pMinLineEdit->setValidator(new QDoubleValidator());
        QLineEdit *pMaxLineEdit = new QLineEdit(max, this);
        pMaxLineEdit->setValidator(new QDoubleValidator());
        QToolButton *pRemoveButton = new QToolButton(this);
        pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
        pRemoveButton->setToolTip("Remove Parameter");
        connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeParameter()));

        mpParameterLabels.append(pLabel);
        mpParameterMinLineEdits.append(pMinLineEdit);
        mpParameterMaxLineEdits.append(pMaxLineEdit);
        mpParameterRemoveButtons.append(pRemoveButton);

        int row = mpParametersLayout->rowCount();
        mpParametersLayout->addWidget(pMinLineEdit, row, 0);
        mpParametersLayout->addWidget(pLabel, row, 1);
        mpParametersLayout->addWidget(pMaxLineEdit, row, 2);
        mpParametersLayout->addWidget(pRemoveButton, row, 3);
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
        mpParametersLayout->removeWidget(mpParameterMinLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterMaxLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterRemoveButtons.at(i));

        delete(mpParameterLabels.at(i));
        delete(mpParameterMinLineEdits.at(i));
        delete(mpParameterMaxLineEdits.at(i));
        delete(mpParameterRemoveButtons.at(i));

        mpParameterLabels.removeAt(i);
        mpParameterMinLineEdits.removeAt(i);
        mpParameterMaxLineEdits.removeAt(i);
        mpParameterRemoveButtons.removeAt(i);

        mSelectedParameters.removeAt(i);
        mSelectedComponents.removeAt(i);
    }
}


QTreeWidgetItem* OptimizationDialog::findParameterTreeItem(QString componentName, QString parameterName)
{
    QTreeWidgetItem* foundItem=0;

    for(int c=0; c<mpParametersList->topLevelItemCount(); ++c)      //Uncheck the parameter in the list before removing it
    {
        if(mpParametersList->topLevelItem(c)->text(0) == componentName)
        {
            bool doBreak = false;
            for(int p=0; p<mpParametersList->topLevelItem(c)->childCount(); ++p)
            {
                if(mpParametersList->topLevelItem(c)->child(p)->text(0) == parameterName)
                {
                    foundItem = mpParametersList->topLevelItem(c)->child(p);
                    doBreak = true;
                    break;
                }
            }
            if(doBreak)
                return foundItem;
        }
    }
}


//! @brief Removes an objevtive function from the selected functions
void OptimizationDialog::removeParameter()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mpParameterRemoveButtons.indexOf(button);

    QTreeWidgetItem *selectedItem = findParameterTreeItem(mSelectedComponents.at(i), mSelectedParameters.at(i));
    if(selectedItem)
    {
        selectedItem->setCheckState(0, Qt::Unchecked);
    }
    else
    {

//    for(int c=0; c<mpParametersList->topLevelItemCount(); ++c)      //Uncheck the parameter in the list before removing it
//    {
//        if(mpParametersList->topLevelItem(c)->text(0) == mSelectedComponents.at(i))
//        {
//            bool doBreak = false;
//            for(int p=0; p<mpParametersList->topLevelItem(c)->childCount(); ++p)
//            {
//                if(mpParametersList->topLevelItem(c)->child(p)->text(0) == mSelectedParameters.at(i))
//                {
//                    mpParametersList->topLevelItem(c)->child(p)->setCheckState(0, Qt::Unchecked);       //Will trigger actual remove in updateChosenVariables()
//                    doBreak = true;
//                    break;
//                }
//            }
//            if(doBreak) return;
//        }
//    }

    //Parameter is not in list (should not really happen), so remove it here instead
    mpParametersLayout->removeWidget(mpParameterLabels.at(i));
    mpParametersLayout->removeWidget(mpParameterMinLineEdits.at(i));
    mpParametersLayout->removeWidget(mpParameterMaxLineEdits.at(i));
    mpParametersLayout->removeWidget(mpParameterRemoveButtons.at(i));

    delete(mpParameterLabels.at(i));
    delete(mpParameterMinLineEdits.at(i));
    delete(mpParameterMaxLineEdits.at(i));
    delete(mpParameterRemoveButtons.at(i));

    mpParameterLabels.removeAt(i);
    mpParameterMinLineEdits.removeAt(i);
    mpParameterMaxLineEdits.removeAt(i);
    mpParameterRemoveButtons.removeAt(i);

    mSelectedParameters.removeAt(i);
    mSelectedComponents.removeAt(i);
    }
}


//! @brief Adds a new variable to the list of selected variables
//! @param item Tree widget item which represents variable
void OptimizationDialog::updateChosenVariables(QTreeWidgetItem *item, int /*i*/)
{
    QStringList variable;
    variable << item->parent()->parent()->text(0) << item->parent()->text(0) << item->text(0);
    mSelectedVariables.removeAll(variable);
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedVariables.append(variable);
    }
}


//! @brief Adds a new objective function from combo box and selected variables
void OptimizationDialog::addFunction()
{
    int idx = mpFunctionsComboBox->currentIndex();
    addObjectiveFunction(idx, 1.0, 1.0, 2.0, mSelectedVariables, QStringList());
}


//! @brief Adds a new objective function
void OptimizationDialog::addObjectiveFunction(int idx, double weight, double norm, double exp, QList<QStringList> selectedVariables, QStringList objData)
{
    if(!verifyNumberOfVariables(idx, selectedVariables.size()))
        return;

    QStringList data = mObjectiveFunctionDataLists.at(idx);

    mSelectedFunctionsMinMax.append(mpMinMaxComboBox->currentText());
    mSelectedFunctions.append(idx);
    mFunctionComponents.append(QStringList());
    mFunctionPorts.append(QStringList());
    mFunctionVariables.append(QStringList());
    for(int i=0; i<selectedVariables.size(); ++i)
    {
        mFunctionComponents.last().append(selectedVariables.at(i).at(0));
        mFunctionPorts.last().append(selectedVariables.at(i).at(1));
        mFunctionVariables.last().append(selectedVariables.at(i).at(2));
    }

    QLineEdit *pWeightLineEdit = new QLineEdit(QString().setNum(weight), this);
    pWeightLineEdit->setValidator(new QDoubleValidator());
    QLineEdit *pNormLineEdit = new QLineEdit(QString().setNum(norm), this);
    pNormLineEdit->setValidator(new QDoubleValidator());
    QLineEdit *pExpLineEdit = new QLineEdit(QString().setNum(exp), this);
    pExpLineEdit->setValidator(new QDoubleValidator());

    QString variablesText = mFunctionComponents.last().first()+", "+mFunctionPorts.last().first()+", "+mFunctionVariables.last().first();
    for(int i=1; i<mFunctionVariables.last().size(); ++i)
    {
        variablesText.append(" and " + mFunctionComponents.last().at(i)+", "+mFunctionPorts.last().at(i)+", "+mFunctionVariables.last().at(i));
    }
    QLabel *pFunctionLabel = new QLabel(mpMinMaxComboBox->currentText() + " " + mObjectiveFunctionDescriptions.at(idx)+" for "+variablesText, this);
    mFunctionName.append(mObjectiveFunctionDescriptions.at(idx));
    pFunctionLabel->setWordWrap(true);
    QWidget *pDataWidget = new QWidget(this);
    QGridLayout *pDataGrid = new QGridLayout(this);
    pDataWidget->setLayout(pDataGrid);
    QList<QLineEdit*> dummyList;
    for(int i=0; i<data.size(); ++i)
    {
        QString thisData;
        if(objData.size()<=i)
            thisData = "1.0";
        else
            thisData = objData.at(i);

        QLabel *pDataLabel = new QLabel(data.at(i), this);
        QLineEdit *pDataLineEdit = new QLineEdit(thisData, this);
        pDataLineEdit->setValidator(new QDoubleValidator());
        pDataGrid->addWidget(pDataLabel, i, 0);
        pDataGrid->addWidget(pDataLineEdit, i, 1);
        dummyList.append(pDataLineEdit);
    }
    mDataLineEditPtrs.append(dummyList);
    QToolButton *pRemoveButton = new QToolButton(this);
    pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
    pRemoveButton->setToolTip("Remove Function");
    mWeightLineEditPtrs.append(pWeightLineEdit);
    mNormLineEditPtrs.append(pNormLineEdit);
    mExpLineEditPtrs.append(pExpLineEdit);
    mFunctionLabelPtrs.append(pFunctionLabel);
    mDataWidgetPtrs.append(pDataWidget);
    mRemoveFunctionButtonPtrs.append(pRemoveButton);

    int row = mpObjectiveLayout->rowCount()-1;
    mpObjectiveLayout->addWidget(pWeightLineEdit, row,   0, 1, 1);
    mpObjectiveLayout->addWidget(pNormLineEdit, row,     1, 1, 1);
    mpObjectiveLayout->addWidget(pExpLineEdit, row,      2, 1, 1);
    mpObjectiveLayout->addWidget(pFunctionLabel, row,   3, 1, 2);
    mpObjectiveLayout->addWidget(pDataWidget, row,      5, 1, 1);
    mpObjectiveLayout->addWidget(pRemoveButton, row,    6, 1, 1);
    mpObjectiveLayout->setRowStretch(row, 0);
    mpObjectiveLayout->setRowStretch(row+1, 1);
    mpObjectiveLayout->setColumnStretch(0, 0);
    mpObjectiveLayout->setColumnStretch(1, 0);
    mpObjectiveLayout->setColumnStretch(2, 0);
    mpObjectiveLayout->setColumnStretch(3, 1);
    mpObjectiveLayout->setColumnStretch(4, 0);
    mpObjectiveLayout->setColumnStretch(5, 0);
    mpObjectiveLayout->setColumnStretch(6, 0);

    connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeFunction()));
}


//! @brief Removes an objevtive function from the selected functions
void OptimizationDialog::removeFunction()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mRemoveFunctionButtonPtrs.indexOf(button);

    mpObjectiveLayout->removeWidget(mWeightLineEditPtrs.at(i));
    mpObjectiveLayout->removeWidget(mNormLineEditPtrs.at(i));
    mpObjectiveLayout->removeWidget(mExpLineEditPtrs.at(i));
    mpObjectiveLayout->removeWidget(mFunctionLabelPtrs.at(i));
    mpObjectiveLayout->removeWidget(mRemoveFunctionButtonPtrs.at(i));
    mpObjectiveLayout->removeWidget(mDataWidgetPtrs.at(i));

    for(int j=0; j<mDataLineEditPtrs.at(i).size(); ++j)
    {
        delete(mDataLineEditPtrs.at(i).at(j));
    }
    delete(mWeightLineEditPtrs.at(i));
    delete(mNormLineEditPtrs.at(i));
    delete(mExpLineEditPtrs.at(i));
    delete(mFunctionLabelPtrs.at(i));
    delete(mRemoveFunctionButtonPtrs.at(i));
    delete(mDataWidgetPtrs.at(i));

    mWeightLineEditPtrs.removeAt(i);
    mNormLineEditPtrs.removeAt(i);
    mExpLineEditPtrs.removeAt(i);
    mFunctionLabelPtrs.removeAt(i);
    mFunctionName.removeAt(i);
    mRemoveFunctionButtonPtrs.removeAt(i);
    mDataLineEditPtrs.removeAt(i);
    mDataWidgetPtrs.removeAt(i);
    mSelectedFunctions.removeAt(i);
    mFunctionComponents.removeAt(i);
    mFunctionPorts.removeAt(i);
    mFunctionVariables.removeAt(i);
}


//! @brief Generates the script code and shows it in the output box
void OptimizationDialog::updateOutputBox()
{
    //! @todo Implement

    generateScriptFile();
    mpOutputBox->clear();
    mpOutputBox->insertPlainText(mScript);
    mpRunButton->setDisabled(mScript.isEmpty());

    saveConfiguration();
}


//! @brief Generates file name for the script
QString OptimizationDialog::generateFileName()
{
    QString dateString = QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate);
    qDebug() << "dateString = " << dateString.toUtf8();
    dateString.replace(":", "_");
    dateString.replace(".", "_");
    dateString.replace(" ", "_");
    return "OptimizationScript_"+dateString.toUtf8()+".py";
}


//! @brief Saves the generated script code to file and executes the script
void OptimizationDialog::run()
{
    saveConfiguration();

    QString dateString = QDateTime::currentDateTime().toString();
    qDebug() << "dateString = " << dateString.toUtf8();
    dateString.replace(":", "_");
    dateString.replace(".", "_");
    dateString.replace(" ", "_");
    QString pyPath = gExecPath+QString(SCRIPTPATH)+generateFileName();
    QFile pyFile(pyPath);
    if (!pyFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + pyPath);
        return;
    }
    QTextStream pyStream(&pyFile);
    //pyStream << mScript;
    pyStream << mpOutputBox->toPlainText().toAscii();
    pyFile.close();

    QTime simTimer;
    simTimer.start();
    gpMainWindow->mpPyDockWidget->runPyScript(pyPath);
    QString timeString = QString().setNum(simTimer.elapsed());
    gpMainWindow->mpMessageWidget->printGUIInfoMessage("Optimization finished after " + timeString + " ms");
    close();
}


//! @brief Checks if number of selected variables is correct. Gives error messages if they are too many or too low.
//! @param i Selected objective function
bool OptimizationDialog::verifyNumberOfVariables(int idx, int nSelVar)
{
    int nVar = mObjectiveFunctionNumberOfVariables.at(idx);

    if(nSelVar > nVar)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Too many variables selected for this function.");
        return false;
    }
    else if(nSelVar < nVar)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Too few variables selected for this function.");
        return false;
    }
    return true;
}


bool OptimizationDialog::loadObjectiveFunctions()
{
    mObjectiveFunctionDescriptions.clear();
    mObjectiveFunctionCalls.clear();
    mObjectiveFunctionNumberOfVariables.clear();
    mObjectiveFunctionUsesTimeVector.clear();
    mObjectiveFunctionDataLists.clear();

    //Read from OptimizationObjectiveFunctions.xml
    QFile file(gExecPath+QString(SCRIPTPATH) + "OptimizationObjectiveFunctions.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 "Unable to read objective functions file. Please make sure that it is located in the Scripts directory.\n");
        return false;
    }
    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 gpMainWindow->tr("HopsanObjectiveFunctions: Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        file.close();
        return false;
    }
    else
    {
        QDomElement functionsRoot = domDocument.documentElement();
        if (functionsRoot.tagName() != "hopsanobjectivefunctions")
        {
            QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                     "The file is not an Hopsan objective function file. Incorrect hmf root tag name: "
                                     + functionsRoot.tagName() + " != hopsanobjectivefunctions");
            file.close();
            return false;
        }
        else
        {
                //Load default user settings
            QDomElement functionElement = functionsRoot.firstChildElement("objectivefunction");
            while(!functionElement.isNull())
            {
                mObjectiveFunctionDescriptions << functionElement.attribute("description");
                mObjectiveFunctionCalls << functionElement.attribute("call");
                mObjectiveFunctionNumberOfVariables << functionElement.attribute("numberofvariables").toInt();
                mObjectiveFunctionUsesTimeVector << (functionElement.attribute("needstimevector") == "true");
                QStringList parameters;
                QDomElement parameterElement = functionElement.firstChildElement("parameter");
                while(!parameterElement.isNull())
                {
                    parameters << parameterElement.text();
                    parameterElement = parameterElement.nextSiblingElement("parameter");
                }
                mObjectiveFunctionDataLists.append(parameters);
                functionElement = functionElement.nextSiblingElement("objectivefunction");
            }
        }

    file.close();
    return true;

}



//    mObjectiveFunctionDescriptions = QStringList() << "Highest value" << "Lowest value" << "Overshoot over value" << "First time to value" << "Difference from value at time" << "Average absolute difference between variables";
//    mObjectiveFunctionCalls = QStringList() << "maxValue" << "minValue" << "overShoot" << "firstTimeAt" << "diffFromValueAtTime" << "averageAbsoluteDifference";
//    mObjectiveFunctionNumberOfVariables = QList<int>() << 1 << 1 << 1 << 1 << 1 << 2;
//    mObjectiveFunctionUsesTimeVector = QList<bool>() << false << false << false << true << true << false;

//    mObjectiveFunctionDataLists.append(QStringList());
//    mObjectiveFunctionDataLists.append(QStringList());
//    mObjectiveFunctionDataLists.append(QStringList() << "x");
//    mObjectiveFunctionDataLists.append(QStringList() << "x");
//    mObjectiveFunctionDataLists.append(QStringList() << "x" << "t");
//    mObjectiveFunctionDataLists.append(QStringList());
}


//! @brief Returns the Python calling code to one of the selected functions
//! @param i Index of selected function
QString OptimizationDialog::generateFunctionCode(int i)
{
    QString retval;
    if(mSelectedFunctionsMinMax.at(i) == "Minimize")
    {
        retval.append("+");
    }
    else
    {
        retval.append("-");
    }

    int fnc = mSelectedFunctions.at(i);

    retval.append("w"+QString().setNum(i)+"*("+mObjectiveFunctionCalls.at(fnc)+"(data"+QString().setNum(i)+"0");
    for(int v=1; v<mObjectiveFunctionNumberOfVariables.at(fnc); ++v)
        retval.append(", data"+QString().setNum(i)+QString().setNum(v));
    if(mObjectiveFunctionUsesTimeVector.at(fnc))
        retval.append(", time");
    for(int d=0; d<mObjectiveFunctionDataLists.at(fnc).size(); ++d)
    {
        double num = mDataLineEditPtrs.at(i).at(d)->text().toDouble();
        retval.append(", "+QString().setNum(num));
    }
    retval.append(")/n"+QString().setNum(i)+")**g"+QString().setNum(i));

    return retval;
}
