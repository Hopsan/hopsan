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
#include "DesktopHandler.h"
#include "GUIPort.h"
#include "Dialogs/OptimizationDialog.h"
#include "HcomHandler.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"
#include "Utilities/HighlightingUtilities.h"
#include "Widgets/HcomWidget.h"
#include "ModelHandler.h"
#include "Widgets/PyDockWidget.h"

class CentralTabWidget;


//! @brief Constructor
OptimizationDialog::OptimizationDialog(MainWindow *parent)
    : QWizard(parent)
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

    mpAlgorithmLabel = new QLabel("Optimiation algorithm:");
    mpAlgorithmBox = new QComboBox(this);
    mpAlgorithmBox->addItems(QStringList() << "Complex" << "Particle Swarm");
    connect(mpAlgorithmBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setAlgorithm(int)));

    mpIterationsLabel = new QLabel("Number of iterations:");
    mpIterationsSpinBox = new QSpinBox(this);
    mpIterationsSpinBox->setRange(0, std::numeric_limits<int>::max());
    mpIterationsSpinBox->setValue(100);

    mpSearchPointsLabel = new QLabel("Number of search points:" );
    mpSearchPointsSpinBox = new QSpinBox(this);
    mpSearchPointsSpinBox->setRange(1, std::numeric_limits<int>::max());
    mpSearchPointsSpinBox->setValue(8);

    mpParticlesLabel = new QLabel("Number of particles:" );
    mpParticlesSpinBox = new QSpinBox(this);
    mpParticlesSpinBox->setRange(1, std::numeric_limits<int>::max());
    mpParticlesSpinBox->setValue(20);

    mpAlphaLabel = new QLabel("Reflection coefficient: ");
    mpAlphaLineEdit = new QLineEdit("1.3", this);
    mpAlphaLineEdit->setValidator(new QDoubleValidator());

    mpBetaLabel = new QLabel("Randomization factor: ");
    mpBetaLineEdit = new QLineEdit("0.3", this);
    mpBetaLineEdit->setValidator(new QDoubleValidator());

    mpGammaLabel = new QLabel("Forgetting factor: ");
    mpGammaLineEdit = new QLineEdit("0.3", this);
    mpGammaLineEdit->setValidator(new QDoubleValidator());

    mpOmegaLabel = new QLabel("Inertia Weight: ");
    mpOmegaLineEdit = new QLineEdit("1", this);
    mpOmegaLineEdit->setValidator(new QDoubleValidator());

    mpC1Label = new QLabel("Learning factor 1: ");
    mpC1LineEdit = new QLineEdit("2", this);
    mpC1LineEdit->setValidator(new QDoubleValidator());

    mpC2Label = new QLabel("Learning Factor 2: ");
    mpC2LineEdit = new QLineEdit("2", this);
    mpC2LineEdit->setValidator(new QDoubleValidator());

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
    mpSettingsLayout->addWidget(mpAlgorithmLabel,       1, 0);
    mpSettingsLayout->addWidget(mpAlgorithmBox,         1, 1);
    mpSettingsLayout->addWidget(mpIterationsLabel,      2, 0);
    mpSettingsLayout->addWidget(mpIterationsSpinBox,    2, 1);
    mpSettingsLayout->addWidget(mpSearchPointsLabel,    3, 0);
    mpSettingsLayout->addWidget(mpSearchPointsSpinBox,  3, 1);
    mpSettingsLayout->addWidget(mpParticlesLabel,       3, 0);
    mpSettingsLayout->addWidget(mpParticlesSpinBox,     3, 1);
    mpSettingsLayout->addWidget(mpAlphaLabel,           4, 0);
    mpSettingsLayout->addWidget(mpAlphaLineEdit,        4, 1);
    mpSettingsLayout->addWidget(mpOmegaLabel,           4, 0);
    mpSettingsLayout->addWidget(mpOmegaLineEdit,        4, 1);
    mpSettingsLayout->addWidget(mpBetaLabel,            5, 0);
    mpSettingsLayout->addWidget(mpBetaLineEdit,         5, 1);
    mpSettingsLayout->addWidget(mpC1Label,              5, 0);
    mpSettingsLayout->addWidget(mpC1LineEdit,           5, 1);
    mpSettingsLayout->addWidget(mpGammaLabel,           6, 0);
    mpSettingsLayout->addWidget(mpGammaLineEdit,        6, 1);
    mpSettingsLayout->addWidget(mpC2Label,              6, 0);
    mpSettingsLayout->addWidget(mpC2LineEdit,           6, 1);
    mpSettingsLayout->addWidget(mpEpsilonFLabel,        7, 0);
    mpSettingsLayout->addWidget(mpEpsilonFLineEdit,     7, 1);
    mpSettingsLayout->addWidget(mpEpsilonXLabel,        8, 0);
    mpSettingsLayout->addWidget(mpEpsilonXLineEdit,     8, 1);
    mpSettingsLayout->addWidget(mpMultiThreadedCheckBox,9, 0);
    mpSettingsLayout->addWidget(mpThreadsLabel,         10, 0);
    mpSettingsLayout->addWidget(mpThreadsSpinBox,       10, 1);
    mpSettingsLayout->addWidget(mpPlottingCheckBox,     11, 0, 1, 2);
    mpSettingsLayout->addWidget(mpExport2CSVBox,        12, 0, 1, 2);
    mpSettingsLayout->addWidget(new QWidget(this),      13, 0, 1, 2);    //Dummy widget for stretching the layout
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
    mpSettingsWidget = new QWizardPage(this);
    mpSettingsWidget->setLayout(mpSettingsLayout);
    setAlgorithm(0);

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
    mpParametersWidget = new QWizardPage(this);
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
    mpObjectiveWidget = new QWizardPage(this);
    mpObjectiveWidget->setLayout(mpObjectiveLayout);

    //Output box tab
    mpOutputBox = new QTextEdit(this);
    HcomHighlighter *pHighligter = new HcomHighlighter(mpOutputBox->document());
    Q_UNUSED(pHighligter);
    QFont monoFont = mpOutputBox->font();
    monoFont.setFamily("Courier");
    monoFont.setPointSize(11);
    mpOutputBox->setFont(monoFont);
    mpOutputBox->setMinimumWidth(450);
    mpOutputLayout = new QGridLayout(this);
    mpOutputLayout->addWidget(mpOutputBox);
    mpOutputWidget = new QWizardPage(this);
    mpOutputWidget->setLayout(mpOutputLayout);

    //Toolbar
    mpHelpButton = new QToolButton(this);
    mpHelpButton->setToolTip(tr("Show context help"));
    mpHelpButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Help.png"));
    this->setButton(QWizard::HelpButton, mpHelpButton);
    this->setOption(QWizard::HaveHelpButton);
    mpHelpButton->setObjectName("optimizationHelpButton");

    this->addPage(mpSettingsWidget);
    this->addPage(mpParametersWidget);
    this->addPage(mpObjectiveWidget);
    this->addPage(mpOutputWidget);

    setButtonText(QWizard::FinishButton, tr("&Execute Script"));
    setButtonText(QWizard::CustomButton1, tr("&Save To Script File"));
    setOption(QWizard::HaveCustomButton1, true);
    setOption(QWizard::CancelButtonOnLeft, true);
    button(QWizard::CustomButton1)->setDisabled(true);

    //Connections
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(update(int)));
    connect(mpAddFunctionButton,            SIGNAL(clicked()),      this,                   SLOT(addFunction()));
    connect(mpHelpButton,                   SIGNAL(clicked()),    gpMainWindow,           SLOT(openContextHelp()));
    connect(this, SIGNAL(accepted()), this, SLOT(run()));
    connect(button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(saveScriptFile()));
}


void OptimizationDialog::loadConfiguration()
{
    SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();

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
        //Check if component and parameter exists before checking the tree item (otherwise tree item does not exist = crash)
        if(gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->hasModelObject(optSettings.mParamters.at(i).mComponentName) &&
           gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getModelObject(optSettings.mParamters.at(i).mComponentName)->getParameterNames().contains(optSettings.mParamters.at(i).mParameterName))
        {
            findParameterTreeItem(optSettings.mParamters.at(i).mComponentName, optSettings.mParamters.at(i).mParameterName)->setCheckState(0, Qt::Checked);
        }
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

        for(int j=0; j < mFunctionComponents.at(i).size(); ++j)
        {
            QStringList variableInfo;

            variableInfo << mFunctionComponents.at(i).at(j);
            variableInfo << mFunctionPorts.at(i).at(j);
            variableInfo << mFunctionVariables.at(i).at(j);

            objective.mVariableInfo.append(variableInfo);
        }

        for(int j=0; j < mDataLineEditPtrs.at(i).size(); ++j)
        {
            objective.mData.append(mDataLineEditPtrs.at(i).at(j)->text());
        }
        optSettings.mObjectives.append(objective);
    }



    SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();
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
    SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();
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
    mpParametersList->sortItems(0,Qt::AscendingOrder);
    mpParametersList->sortItems(1,Qt::AscendingOrder);
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
        QList<Port*> ports = pSystem->getModelObject(componentNames.at(c))->getPortListPtrs();
        for(int p=0; p<ports.size(); ++p)
        {
            QTreeWidgetItem *pPortItem = new QTreeWidgetItem(QStringList() << ports.at(p)->getName());
            QVector<QString> portNames, portUnits;
            pSystem->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(componentNames.at(c), ports.at(p)->getName(), portNames, portUnits);
            for(int v=0; v<portNames.size(); ++v)
            {
                QTreeWidgetItem *pVariableItem = new QTreeWidgetItem(QStringList() << portNames.at(v));
                pVariableItem->setCheckState(0, Qt::Unchecked);
                pPortItem->insertChild(0, pVariableItem);
            }
            pComponentItem->insertChild(0, pPortItem);
        }
    }
    mpVariablesList->sortItems(0,Qt::AscendingOrder);
    mpVariablesList->sortItems(1,Qt::AscendingOrder);
    mpVariablesList->sortItems(2,Qt::AscendingOrder);
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
    //mpRunButton->setDisabled(true);

    loadConfiguration();

    QDialog::show();
}


void OptimizationDialog::okPressed()
{
    saveConfiguration();

    reject();
}


//! @brief Generates the Python script based upon selections made in the dialog
void OptimizationDialog::generateScriptFile()
{
    if(mpParametersLogCheckBox->isChecked())
    {
        bool itsOk = true;
        for(int i=0; i<mpParameterMinLineEdits.size(); ++i)
        {
            if(mpParameterMinLineEdits[i]->text().toDouble() <= 0 || mpParameterMinLineEdits[i]->text().toDouble() <= 0)
            {
                itsOk = false;
            }
        }
        if(!itsOk)
        {
            QMessageBox::warning(this, "Warning", "Logarithmic scaling is selected, but parameters are allowed to be negative or zero. This will probably not work.");
        }
    }

    if(mSelectedParameters.isEmpty())
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No parameters specified for optimization.");
        return;
    }

    if(mSelectedFunctions.isEmpty())
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No objective functions specified for optimization.");
        return;
    }

    switch (mpAlgorithmBox->currentIndex())
    {
    case 0 :
        generateComplexScript();
        break;
    case 1 :
        generateParticleSwarmScript();
        break;
    default :
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Algorithm type undefined.");
    }
}

void OptimizationDialog::generateComplexScript()
{
    QFile templateFile(gDesktopHandler.getExecPath()+"../Scripts/HCOM/optTemplateComplex.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    QString objFuncs;
    QString totalObj;
    QString objPars;
    QStringList plotVarsList;
    QString plotVars;
    QString setMinMax;
    QString setPars;
    for(int i=0; i<mFunctionName.size(); ++i)
    {
        QString objFunc = mObjectiveFunctionCalls[mObjectiveFunctionDescriptions.indexOf(mFunctionName[i])];
        objFunc.prepend("    ");
        objFunc.replace("\n", "\n    ");
        objFunc.replace("<<<id>>>", QString::number(i+1));
        for(int j=0; j<mFunctionComponents[i].size(); ++j)
        {
            QString varName = mFunctionComponents[i][j]+"."+mFunctionPorts[i][j]+"."+mFunctionVariables[i][j];
            gpMainWindow->mpTerminalWidget->mpHandler->toShortDataNames(varName);
            objFunc.replace("<<<var"+QString::number(j+1)+">>>", varName);

            if(!plotVarsList.contains(varName))
            {
                plotVarsList.append(varName);
                plotVars.append(varName+" ");
            }
        }
        for(int j=0; j<mDataLineEditPtrs[i].size(); ++j)
        {
            objFunc.replace("<<<arg"+QString::number(j+1)+">>>", mDataLineEditPtrs[i][j]->text());
        }
        objFuncs.append(objFunc+"\n");

        if(mSelectedFunctionsMinMax.at(i) == "Minimize")
        {
            totalObj.append("+");
        }
        else
        {
            totalObj.append("-");
        }
        QString idx = QString::number(i+1);
        totalObj.append("w"+idx+"*r"+idx+"*exp(e"+idx+")*obj"+idx);

        objPars.append("w"+idx+"="+mWeightLineEditPtrs[i]->text()+"\n");
        objPars.append("r"+idx+"="+mNormLineEditPtrs[i]->text()+"\n");
        objPars.append("e"+idx+"="+mExpLineEditPtrs[i]->text()+"\n");

    }

    for(int p=0; p<mSelectedParameters.size(); ++p)
    {
        QString par = mSelectedComponents[p]+"."+mSelectedParameters[p];
        gpMainWindow->mpTerminalWidget->mpHandler->toShortDataNames(par);
        setPars.append("    chpa "+par+" par(evalId,"+QString::number(p)+")\n");

        setMinMax.append("opt set limits "+QString::number(p)+" "+mpParameterMinLineEdits[p]->text()+" "+mpParameterMaxLineEdits[p]->text()+"\n");
    }


    templateCode.replace("<<<objfuncs>>>", objFuncs);
    templateCode.replace("<<<totalobj>>>", totalObj);
    templateCode.replace("<<<objpars>>>", objPars);
    templateCode.replace("<<<plotvars>>>", plotVars);
    templateCode.replace("<<<setminmax>>>", setMinMax);
    templateCode.replace("<<<setpars>>>", setPars);
    templateCode.replace("<<<npoints>>>", QString::number(mpSearchPointsSpinBox->value()));
    templateCode.replace("<<<nparams>>>", QString::number(mSelectedParameters.size()));
    templateCode.replace("<<<maxevals>>>", QString::number(mpIterationsSpinBox->value()));
    templateCode.replace("<<<alpha>>>", mpAlphaLineEdit->text());
    templateCode.replace("<<<rfak>>>", mpBetaLineEdit->text());
    templateCode.replace("<<<gamma>>>", mpGammaLineEdit->text());
    templateCode.replace("<<<functol>>>", mpEpsilonFLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());

    mScript = templateCode;
}


void OptimizationDialog::generateParticleSwarmScript()
{
    QFile templateFile(gDesktopHandler.getExecPath()+"../Scripts/HCOM/optTemplateParticle.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    QString objFuncs;
    QString totalObj;
    QString objPars;
    QStringList plotVarsList;
    QString plotVars;
    QString setMinMax;
    QString setPars;
    for(int i=0; i<mFunctionName.size(); ++i)
    {
        QString objFunc = mObjectiveFunctionCalls[mObjectiveFunctionDescriptions.indexOf(mFunctionName[i])];
        objFunc.prepend("    ");
        objFunc.replace("\n", "\n    ");
        objFunc.replace("<<<id>>>", QString::number(i+1));
        for(int j=0; j<mFunctionComponents[i].size(); ++j)
        {
            QString varName = mFunctionComponents[i][j]+"."+mFunctionPorts[i][j]+"."+mFunctionVariables[i][j];
            gpMainWindow->mpTerminalWidget->mpHandler->toShortDataNames(varName);
            objFunc.replace("<<<var"+QString::number(j+1)+">>>", varName);

            if(!plotVarsList.contains(varName))
            {
                plotVarsList.append(varName);
                plotVars.append(varName+" ");
            }
        }
        for(int j=0; j<mDataLineEditPtrs[i].size(); ++j)
        {
            objFunc.replace("<<<arg"+QString::number(j+1)+">>>", mDataLineEditPtrs[i][j]->text());
        }
        objFuncs.append(objFunc+"\n");

        if(mSelectedFunctionsMinMax.at(i) == "Minimize")
        {
            totalObj.append("+");
        }
        else
        {
            totalObj.append("-");
        }
        QString idx = QString::number(i+1);
        totalObj.append("w"+idx+"*r"+idx+"*exp(e"+idx+")*obj"+idx);

        objPars.append("w"+idx+"="+mWeightLineEditPtrs[i]->text()+"\n");
        objPars.append("r"+idx+"="+mNormLineEditPtrs[i]->text()+"\n");
        objPars.append("e"+idx+"="+mExpLineEditPtrs[i]->text()+"\n");

    }

    for(int p=0; p<mSelectedParameters.size(); ++p)
    {
        QString par = mSelectedComponents[p]+"."+mSelectedParameters[p];
        gpMainWindow->mpTerminalWidget->mpHandler->toShortDataNames(par);
        setPars.append("    chpa "+par+" par(evalId,"+QString::number(p)+")\n");

        setMinMax.append("opt set limits "+QString::number(p)+" "+mpParameterMinLineEdits[p]->text()+" "+mpParameterMaxLineEdits[p]->text()+"\n");
    }


    templateCode.replace("<<<objfuncs>>>", objFuncs);
    templateCode.replace("<<<totalobj>>>", totalObj);
    templateCode.replace("<<<objpars>>>", objPars);
    templateCode.replace("<<<plotvars>>>", plotVars);
    templateCode.replace("<<<setminmax>>>", setMinMax);
    templateCode.replace("<<<setpars>>>", setPars);
    templateCode.replace("<<<npoints>>>", QString::number(mpSearchPointsSpinBox->value()));
    templateCode.replace("<<<nparams>>>", QString::number(mSelectedParameters.size()));
    templateCode.replace("<<<maxevals>>>", QString::number(mpIterationsSpinBox->value()));
    templateCode.replace("<<<omega>>>", mpOmegaLineEdit->text());
    templateCode.replace("<<<c1>>>", mpC1LineEdit->text());
    templateCode.replace("<<<c2>>>", mpC2LineEdit->text());
    templateCode.replace("<<<functol>>>", mpEpsilonFLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());

    mScript = templateCode;
}


void OptimizationDialog::setAlgorithm(int i)
{
    //Complex
    mpSearchPointsLabel->setVisible(i==0);
    mpSearchPointsSpinBox->setVisible(i==0);
    mpAlphaLabel->setVisible(i==0);
    mpAlphaLineEdit->setVisible(i==0);
    mpBetaLabel->setVisible(i==0);
    mpBetaLineEdit->setVisible(i==0);
    mpGammaLabel->setVisible(i==0);
    mpGammaLineEdit->setVisible(i==0);

    //Particle swarm
    mpParticlesLabel->setVisible(i==1);
    mpParticlesSpinBox->setVisible(i==1);
    mpOmegaLabel->setVisible(i==1);
    mpOmegaLineEdit->setVisible(i==1);
    mpC1Label->setVisible(i==1);
    mpC1LineEdit->setVisible(i==1);
    mpC2Label->setVisible(i==1);
    mpC2LineEdit->setVisible(i==1);
}


//! @brief Adds a new parameter to the list of selected parameter, and displays it in dialog
//! @param item Tree widget item which represents parameter
void OptimizationDialog::updateChosenParameters(QTreeWidgetItem* item, int /*i*/)
{
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedComponents.append(item->parent()->text(0));
        mSelectedParameters.append(item->text(0));
        SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();
        QString currentValue = pSystem->getModelObject(item->parent()->text(0))->getParameterValue(item->text(0));

        QLabel *pLabel = new QLabel(trUtf8(" <  ") + item->parent()->text(0) + ", " + item->text(0) + " (" + currentValue + trUtf8(")  < "));
        pLabel->setAlignment(Qt::AlignCenter);

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

    return 0;
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
    mSelectedFunctionsMinMax.removeAt(i);
    mFunctionComponents.removeAt(i);
    mFunctionPorts.removeAt(i);
    mFunctionVariables.removeAt(i);

}


//! @brief Generates the script code and shows it in the output box
void OptimizationDialog::update(int idx)
{
    //Finished parameters tab
    if(idx == 2)
    {
        if(mSelectedParameters.isEmpty())
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No parameters specified for optimization.");
            this->back();
            return;
        }
    }

    //Finished objective function tab
    if(idx == 3)
    {
        if(mSelectedFunctions.isEmpty())
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No objective functions specified for optimization.");
            this->back();
            return;
        }
        else
        {
            button(QWizard::CustomButton1)->setDisabled(false);
            generateScriptFile();
            mpOutputBox->clear();
            mpOutputBox->insertPlainText(mScript);
            saveConfiguration();
            return;
        }
    }
}



//! @brief Saves the generated script code to file and executes the script
void OptimizationDialog::run()
{
    saveConfiguration();

    QStringList commands = mpOutputBox->toPlainText().split("\n");
    bool *abort = new bool;
    gpMainWindow->mpTerminalWidget->setEnabledAbortButton(true);
    gpMainWindow->mpTerminalWidget->mpHandler->runScriptCommands(commands, abort);
    gpMainWindow->mpTerminalWidget->setEnabledAbortButton(false);
    delete(abort);
}


//! @brief Saves generated script to a script file
void OptimizationDialog::saveScriptFile()
{
    QString filePath = QFileDialog::getSaveFileName(gpMainWindow, tr("Save Script File"),
                                                 gConfig.getScriptDir(),
                                                 gpMainWindow->tr("HCOM Script (*.hcom)"));

    if(filePath.isEmpty())     //Don't save anything if user presses cancel
    {
        return;
    }

    QFileInfo fileInfo = QFileInfo(filePath);
    gConfig.setScriptDir(fileInfo.absolutePath());

    QFile file(filePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&file);
    out << mpOutputBox->toPlainText();
    file.close();
}


//! @brief Checks if number of selected variables is correct. Gives error messages if they are too many or too low.
//! @param i Selected objective function
bool OptimizationDialog::verifyNumberOfVariables(int idx, int nSelVar)
{
    int nVar = mObjectiveFunctionNumberOfVariables.at(idx);

    if(nSelVar > nVar)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Too many variables selected for this function.");
        return false;
    }
    else if(nSelVar < nVar)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Too few variables selected for this function.");
        return false;
    }
    return true;
}


bool OptimizationDialog::loadObjectiveFunctions()
{
    // Look in both local and global scripts directory in case they are different

    //QDir scriptsDir(gDesktopHandler.getExecPath()+"../Scripts/HCOM/objFuncTemplates");
    QDir scriptsDir(gDesktopHandler.getScriptsPath()+"/HCOM/objFuncTemplates");
    QStringList files = scriptsDir.entryList(QStringList() << "*.hcom");
    int f=0;
    for(; f<files.size(); ++f)
    {
        files[f].prepend(scriptsDir.absolutePath()+"/");
    }
    QDir localScriptsDir(gDesktopHandler.getExecPath()+"/../Scripts/HCOM/objFuncTemplates");
    files.append(localScriptsDir.entryList(QStringList() << "*.hcom"));
    for(int g=f; g<files.size(); ++g)
    {
        files[g].prepend(localScriptsDir.absolutePath()+"/");
    }
    files.removeDuplicates();

    Q_FOREACH(const QString fileName, files)
    {
        QFile templateFile(fileName);
        templateFile.open(QFile::ReadOnly | QFile::Text);
        QString code = templateFile.readAll();
        templateFile.close();

        //Get description
        if(code.startsWith("#"))
        {
            mObjectiveFunctionDescriptions << code.section("#",1,1).section("\n",0,0);
        }
        else
        {
            mObjectiveFunctionDescriptions << QFileInfo(templateFile).fileName();
        }

        //Count variables
        int varCounter=0;
        for(int i=1; ; ++i)
        {
            if(code.contains("var"+QString::number(i)))
            {
                ++varCounter;
            }
            else
            {
                break;
            }
        }
        mObjectiveFunctionNumberOfVariables << varCounter;

        //Count arguments
        QStringList args;
        for(int i=1; ; ++i)
        {
            if(code.contains("arg"+QString::number(i)))
            {
                args << "arg"+QString::number(i);
            }
            else
            {
                break;
            }
        }
        mObjectiveFunctionDataLists << args;

        mObjectiveFunctionUsesTimeVector << false;
        mObjectiveFunctionCalls << code;
    }

    return true;
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
