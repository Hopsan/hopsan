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
//! @file   OptimizationScriptWizard.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2018-05-25
//!
//! @brief Contains a class for the optimization script wizard
//!
//$Id$

//Qt includes
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QScrollArea>
#include <QToolBar>

#include "global.h"

//Hopsan includes
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "HcomHandler.h"
#include "Utilities/HelpPopUpWidget.h"
#include "ModelHandler.h"
#include "OptimizationHandler.h"
#include "Widgets/HcomWidget.h"
#include "Utilities/GUIUtilities.h"
#include "OptimizationScriptWizard.h"



OptimizationScriptWizardPage::OptimizationScriptWizardPage(OptimizationScriptWizard *pParent)
    : QWizardPage(pParent)
{
    mpWizard = pParent;

    connect(mpWizard, SIGNAL(contentsChanged()), this, SIGNAL(completeChanged()));
}

bool OptimizationScriptWizardPage::isComplete() const
{
    if(isFinalPage())
    {
        return (!mpWizard->mSelectedParameters.empty() && !mpWizard->mSelectedFunctions.empty());
    }
    else
    {
        return QWizardPage::isComplete();
    }
}


//! @brief Constructor
OptimizationScriptWizard::OptimizationScriptWizard(SystemObject* pSystem, QWidget *parent)
    : QWizard(parent)
{
        //Set the name and size of the main window
    this->resize(800,600);
    this->setWindowTitle("Optimization");
    this->setPalette(gpConfig->getPalette());

    mpSystem = pSystem;

    //Settings tab
    QLabel *pSettingsLabel = new QLabel("Please choose general settings for optimization algorithm.");
    QFont boldFont = pSettingsLabel->font();
    boldFont.setBold(true);
    pSettingsLabel->setFont(boldFont);

    QLabel *pAlgorithmLabel = new QLabel("Optimiation algorithm:");
    mpAlgorithmBox = new QComboBox(this);
    mpAlgorithmBox->addItems(QStringList() << "Simplex (Nelder-Mead)" << "Complex-RF" << "Complex-RFP" << "Particle Swarm" << "Differential Evolution"<< "Genetic Algorithm" << "Parameter Sweep");
    connect(mpAlgorithmBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setAlgorithm(int)));

    QLabel *pIterationsLabel = new QLabel("Number of iterations:");
    mpIterationsSpinBox = new QSpinBox(this);
    mpIterationsSpinBox->setRange(0, std::numeric_limits<int>::max());
    mpIterationsSpinBox->setValue(100);

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

    mpRhoLabel = new QLabel("Contraction factor: ");
    mpRhoLineEdit = new QLineEdit("0.3", this);
    mpRhoLineEdit->setValidator(new QDoubleValidator());

    mpSigmaLabel = new QLabel("Reduction factor: ");
    mpSigmaLineEdit = new QLineEdit("0.3", this);
    mpSigmaLineEdit->setValidator(new QDoubleValidator());

    mpOmega1Label = new QLabel("Initial inertia weight: ");
    mpOmega1LineEdit = new QLineEdit("1", this);
    mpOmega1LineEdit->setValidator(new QDoubleValidator());

    mpOmega2Label = new QLabel("Final inertia weight: ");
    mpOmega2LineEdit = new QLineEdit("0.5", this);
    mpOmega2LineEdit->setValidator(new QDoubleValidator());

    mpC1Label = new QLabel("Learning factor 1: ");
    mpC1LineEdit = new QLineEdit("2", this);
    mpC1LineEdit->setValidator(new QDoubleValidator());

    mpC2Label = new QLabel("Learning factor 2: ");
    mpC2LineEdit = new QLineEdit("2", this);
    mpC2LineEdit->setValidator(new QDoubleValidator());

    mpVmaxLabel = new QLabel("Maximum particle velocity: ");
    mpVmaxLineEdit = new QLineEdit("2", this);
    mpVmaxLineEdit->setValidator(new QDoubleValidator());

    mpFLabel = new QLabel("Differential weight: ");
    mpFLineEdit = new QLineEdit("1.0", this);
    mpFLineEdit->setValidator(new QDoubleValidator());

    mpCRLabel = new QLabel("Crossover probability: ");
    mpCRLineEdit = new QLineEdit("0.5", this);
    mpCRLineEdit->setValidator(new QDoubleValidator());

    mpCPLabel = new QLabel("Crossover probability: ");
    mpCPLineEdit = new QLineEdit("0.2", this);
    mpCPLineEdit->setValidator(new QDoubleValidator());

    mpMPLabel = new QLabel("Mutation probability: ");
    mpMPLineEdit = new QLineEdit("0.1", this);
    mpMPLineEdit->setValidator(new QDoubleValidator());

    mpElitesLabel = new QLabel("Number of elites: ");
    mpElitesLineEdit = new QLineEdit("4", this);
    mpElitesLineEdit->setValidator(new QIntValidator());

    mpNumModelsLabel = new QLabel("Number of models: ");
    mpNumModelsLineEdit = new QLineEdit(QString::number(qMax(1,gpConfig->getIntegerSetting(cfg::numberofthreads))), this);
    mpNumModelsLineEdit->setValidator(new QIntValidator());

    mpMethodLabel = new QLabel("Parallel method: ");
    mpMethodComboBox = new QComboBox(this);
    mpMethodComboBox->addItems(QStringList() << "Task prediction" << "Multi-distance");

    mpLengthLabel = new QLabel("Length: ");
    mpLengthSpinBox = new QSpinBox(this);
    mpLengthSpinBox->setMaximum(100000000);
    mpLengthSpinBox->setValue(10);

    mpPercDiffLabel = new QLabel("Difference [%]: ");
    mpPercDiffLineEdit = new QLineEdit("0.002", this);
    mpPercDiffLineEdit->setValidator(new QDoubleValidator());

    mpCountMaxLabel = new QLabel("Max Count: ");
    mpCountMaxSpinBox = new QSpinBox(this);
    mpCountMaxSpinBox->setValue(2);

    QLabel *pEpsilonXLabel = new QLabel("Tolerance for parameter convergence: ");
    mpEpsilonXLineEdit = new QLineEdit("0.0001", this);
    mpEpsilonXLineEdit->setValidator(new QDoubleValidator());

    mpPlotBestWorstCheckBox = new QCheckBox("Plot objective function values", this);
    mpPlotBestWorstCheckBox->setChecked(false);

    mpPlotParticlesCheckBox = new QCheckBox("Plot particles", this);
    mpPlotParticlesCheckBox->setChecked(false);

    mpPlotEntropyCheckBox = new QCheckBox("Plot entropy", this);
    mpPlotEntropyCheckBox->setChecked(false);

    mpExport2CSVBox= new QCheckBox("Export trace data to CSV file", this);
    mpExport2CSVBox->setChecked(false);

    mpFinalEvalCheckBox= new QCheckBox("Evaluate all points again after optimization (removes forgetting factor artifacts)", this);
    mpFinalEvalCheckBox->setChecked(false);

    int row=0;
    QGridLayout *pSettingsLayout = new QGridLayout(this);
    pSettingsLayout->addWidget(pSettingsLabel,         row++, 0);
    pSettingsLayout->addWidget(pAlgorithmLabel,        row,   0);
    pSettingsLayout->addWidget(mpAlgorithmBox,         row++, 1);
    pSettingsLayout->addWidget(pIterationsLabel,       row,   0);
    pSettingsLayout->addWidget(mpIterationsSpinBox,    row++, 1);
    pSettingsLayout->addWidget(mpParticlesLabel,       row,   0);
    pSettingsLayout->addWidget(mpParticlesSpinBox,     row++, 1);
    pSettingsLayout->addWidget(mpAlphaLabel,           row,   0);
    pSettingsLayout->addWidget(mpAlphaLineEdit,        row++, 1);
    pSettingsLayout->addWidget(mpOmega1Label,          row,   0);
    pSettingsLayout->addWidget(mpOmega1LineEdit,       row++, 1);
    pSettingsLayout->addWidget(mpOmega2Label,          row,   0);
    pSettingsLayout->addWidget(mpOmega2LineEdit,       row++, 1);
    pSettingsLayout->addWidget(mpBetaLabel,            row,   0);
    pSettingsLayout->addWidget(mpBetaLineEdit,         row++, 1);
    pSettingsLayout->addWidget(mpC1Label,              row,   0);
    pSettingsLayout->addWidget(mpC1LineEdit,           row++, 1);
    pSettingsLayout->addWidget(mpGammaLabel,           row,   0);
    pSettingsLayout->addWidget(mpGammaLineEdit,        row++, 1);
    pSettingsLayout->addWidget(mpRhoLabel,             row,   0);
    pSettingsLayout->addWidget(mpRhoLineEdit,          row++, 1);
    pSettingsLayout->addWidget(mpSigmaLabel,           row,   0);
    pSettingsLayout->addWidget(mpSigmaLineEdit,        row++, 1);
    pSettingsLayout->addWidget(mpC2Label,              row,   0);
    pSettingsLayout->addWidget(mpC2LineEdit,           row++, 1);
    pSettingsLayout->addWidget(mpVmaxLabel,            row,   0);
    pSettingsLayout->addWidget(mpVmaxLineEdit,         row++, 1);
    pSettingsLayout->addWidget(mpFLabel,               row,   0);
    pSettingsLayout->addWidget(mpFLineEdit,            row++, 1);
    pSettingsLayout->addWidget(mpCRLabel,              row,   0);
    pSettingsLayout->addWidget(mpCRLineEdit,           row++, 1);
    pSettingsLayout->addWidget(mpCPLabel,              row,   0);
    pSettingsLayout->addWidget(mpCPLineEdit,           row++, 1);
    pSettingsLayout->addWidget(mpMPLabel,              row,   0);
    pSettingsLayout->addWidget(mpMPLineEdit,           row++, 1);
    pSettingsLayout->addWidget(mpElitesLabel,              row,   0);
    pSettingsLayout->addWidget(mpElitesLineEdit,           row++, 1);
    pSettingsLayout->addWidget(mpNumModelsLabel,       row,   0);
    pSettingsLayout->addWidget(mpNumModelsLineEdit,    row++, 1);
    pSettingsLayout->addWidget(mpMethodLabel,          row,   0);
    pSettingsLayout->addWidget(mpMethodComboBox,       row++, 1);
    pSettingsLayout->addWidget(mpLengthLabel,          row,   0);
    pSettingsLayout->addWidget(mpLengthSpinBox,        row++, 1);
    pSettingsLayout->addWidget(mpPercDiffLabel,        row,   0);
    pSettingsLayout->addWidget(mpPercDiffLineEdit,     row++, 1);
    pSettingsLayout->addWidget(mpCountMaxLabel,        row,   0);
    pSettingsLayout->addWidget(mpCountMaxSpinBox,      row++, 1);
    pSettingsLayout->addWidget(pEpsilonXLabel,         row,   0);
    pSettingsLayout->addWidget(mpEpsilonXLineEdit,     row++, 1);
    pSettingsLayout->addWidget(mpPlotBestWorstCheckBox,row++, 0, 1, 2);
    pSettingsLayout->addWidget(mpPlotParticlesCheckBox,row++, 0, 1, 2);
    pSettingsLayout->addWidget(mpPlotEntropyCheckBox,  row++, 0, 1, 2);
    pSettingsLayout->addWidget(mpExport2CSVBox,        row++, 0, 1, 2);
    pSettingsLayout->addWidget(mpFinalEvalCheckBox,    row++, 0, 1, 2);
    pSettingsLayout->addWidget(new QWidget(this),      row++, 0, 1, 2);    //Dummy widget for stretching the layout
    pSettingsLayout->setRowStretch(row++, 1);
    auto *pSettingsWidget = new OptimizationScriptWizardPage(this);
    pSettingsWidget->setLayout(pSettingsLayout);
    pSettingsWidget->setPalette(gpConfig->getPalette());
    setAlgorithm(0);

    //Parameter tab
    QLabel *pParametersLabel = new QLabel("Choose optimization parameters, and specify their minimum and maximum values.");
    pParametersLabel->setFont(boldFont);
    mpParametersLogCheckBox = new QCheckBox("Use logarithmic parameter scaling", this);
    mpParametersLogCheckBox->setChecked(false);
    mpParametersList = new QTreeWidget(this);
    QLabel *pParameterMinLabel = new QLabel("Min Value");
    pParameterMinLabel->setAlignment(Qt::AlignCenter);
    QLabel *pParameterNameLabel = new QLabel("Parameter Name");
    pParameterNameLabel->setAlignment(Qt::AlignCenter);
    QLabel *pParameterMaxLabel = new QLabel("Max Value");
    pParameterMaxLabel->setAlignment(Qt::AlignCenter);
    pParameterMinLabel->setFont(boldFont);
    pParameterNameLabel->setFont(boldFont);
    pParameterMaxLabel->setFont(boldFont);
    mpParametersLayout = new QGridLayout(this);
    mpParametersLayout->addWidget(pParametersLabel,        0, 0, 1, 4);
    mpParametersLayout->addWidget(mpParametersLogCheckBox,  1, 0, 1, 4);
    mpParametersLayout->addWidget(mpParametersList,         2, 0, 1, 4);
    mpParametersLayout->addWidget(pParameterMinLabel,      3, 0, 1, 1);
    mpParametersLayout->addWidget(pParameterNameLabel,     3, 1, 1, 1);
    mpParametersLayout->addWidget(pParameterMaxLabel,      3, 2, 1, 1);
    OptimizationScriptWizardPage *pParametersWidget = new OptimizationScriptWizardPage(this);
    pParametersWidget->setLayout(mpParametersLayout);

    //Objective function tab
    QLabel *pObjectiveLabel = new QLabel("Create an objective function by first choosing variables in the list and then choosing a function below.");
    pObjectiveLabel->setFont(boldFont);
    mpVariablesList = new QTreeWidget(this);
    mpMinMaxComboBox = new QComboBox(this);
    mpMinMaxComboBox->addItems(QStringList() << "Minimize" << "Maximize");
    mpFunctionsComboBox = new QComboBox(this);
    mpFunctionsComboBox->setStyleSheet("QComboBox { combobox-popup: 10; }");
    mpAddFunctionButton = new QPushButton("Add Function");
    QLabel *pWeightLabel = new QLabel("Weight");
    QLabel *pNormLabel = new QLabel("Norm. Factor");
    QLabel *pExpLabel = new QLabel("Exp. Factor");
    QLabel *pDescriptionLabel = new QLabel("Description");
    QLabel *pDataLabel = new QLabel("Data");
    pWeightLabel->setFont(boldFont);
    pNormLabel->setFont(boldFont);
    pExpLabel->setFont(boldFont);
    pDescriptionLabel->setFont(boldFont);
    pDataLabel->setFont(boldFont);
    mpObjectiveLayout = new QGridLayout(this);
    mpObjectiveLayout->addWidget(pObjectiveLabel,          0, 0, 1, 7);
    mpObjectiveLayout->addWidget(mpVariablesList,           1, 0, 1, 7);
    mpObjectiveLayout->addWidget(mpMinMaxComboBox,          2, 0, 1, 1);
    mpObjectiveLayout->addWidget(mpFunctionsComboBox,       2, 1, 1, 4);
    mpObjectiveLayout->addWidget(mpAddFunctionButton,       2, 5, 1, 2);
    mpObjectiveLayout->addWidget(pWeightLabel,             3, 0, 1, 1);
    mpObjectiveLayout->addWidget(pNormLabel,               3, 1, 1, 1);
    mpObjectiveLayout->addWidget(pExpLabel,                3, 2, 1, 1);
    mpObjectiveLayout->addWidget(pDescriptionLabel,        3, 3, 1, 2);
    mpObjectiveLayout->addWidget(pDataLabel,               3, 5, 1, 2);
    mpObjectiveLayout->addWidget(new QWidget(this),         4, 0, 1, 7);
    mpObjectiveLayout->setRowStretch(0, 0);
    mpObjectiveLayout->setRowStretch(1, 1);
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
    OptimizationScriptWizardPage *pObjectiveWidget = new OptimizationScriptWizardPage(this);
    pObjectiveWidget->setLayout(mpObjectiveLayout);

    //Tool bar
    QToolButton *pHelpButton = new QToolButton(this);
    pHelpButton->setToolTip(tr("Show context help"));
    pHelpButton->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Help.svg"));
    this->setButton(QWizard::HelpButton, pHelpButton);
    this->setOption(QWizard::HaveHelpButton);
    pHelpButton->setObjectName("optimizationHelpButton");



    this->addPage(pSettingsWidget);
    this->addPage(pParametersWidget);
    this->addPage(pObjectiveWidget);

    setButtonText(QWizard::FinishButton, tr("&Generate Script"));
    setButtonText(QWizard::CancelButton, tr("&Abort"));
    button(QWizard::FinishButton)->setEnabled(true);
    button(QWizard::FinishButton)->setHidden(true);
    button(QWizard::CancelButton)->setEnabled(true);
    button(QWizard::CancelButton)->setHidden(true);


    //Connections
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(update(int)));
    connect(mpAddFunctionButton,            SIGNAL(clicked()),      this,                   SLOT(addFunction()));
    connect(pHelpButton,                   SIGNAL(clicked()),    gpHelpPopupWidget,           SLOT(openContextHelp()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveConfiguration()));
}


//! @brief Searches parameter tree for specified tree item
QTreeWidgetItem* OptimizationScriptWizard::findParameterTreeItem(QString componentName, QString parameterName)
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


//! @brief Reimplementation of open() slot, used to initialize the dialog
void OptimizationScriptWizard::open()
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
    QStringList componentNames = mpSystem->getModelObjectNames();
    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpParametersList->insertTopLevelItem(0, pComponentItem);
        QStringList parameterNames = mpSystem->getModelObject(componentNames.at(c))->getParameterNames();
        for(int p=0; p<parameterNames.size(); ++p)
        {
            QTreeWidgetItem *pParameterItem = new QTreeWidgetItem(QStringList() << parameterNames.at(p));
            pParameterItem->setCheckState(0, Qt::Unchecked);
            Port *pPort = mpSystem->getModelObject(componentNames.at(c))->getPort(parameterNames[p].remove("#Value"));
            if (pPort)
            {
                //! @todo we would need to differ between output variables and input variables so that we know if it is a "default startvalue" (in) or a startvalue (out)
                //! @todo CAnt add this extra text as the text is directly used as the parameter name
                //pParameterItem->setText(0, pParameterItem->text(0)+"  (Default StartValue)");
                if (pPort->isConnected())
                {
                    pParameterItem->setTextColor(0, QColor("gray"));
                    //! @todo we cant disable unless we know if it is an input variable (default startvalue), startvalues (output vars) should be enabled even if connected
                    //pParameterItem->setDisabled(true);
                    QFont italicFont = pParameterItem->font(0);
                    italicFont.setItalic(true);
                    pParameterItem->setFont(0, italicFont);
                }
            }
            pComponentItem->insertChild(0, pParameterItem);
        }
    }
    QTreeWidgetItem *pSystemParametersItem = new QTreeWidgetItem(QStringList() << "_System Parameters");
    QFont componentFont = pSystemParametersItem->font(0);
    componentFont.setBold(true);
    pSystemParametersItem->setFont(0, componentFont);
    mpParametersList->insertTopLevelItem(0, pSystemParametersItem);
    QStringList parameterNames = mpSystem->getParameterNames();
    for(int p=0; p<parameterNames.size(); ++p)
    {
        QTreeWidgetItem *pParameterItem = new QTreeWidgetItem(QStringList() << parameterNames.at(p));
        pParameterItem->setCheckState(0, Qt::Unchecked);
        pSystemParametersItem->insertChild(0, pParameterItem);
    }
    mpParametersList->sortItems(0,Qt::AscendingOrder);
    mpParametersList->sortItems(1,Qt::AscendingOrder);
    connect(mpParametersList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(updateChosenParameters(QTreeWidgetItem*,int)), Qt::UniqueConnection);

    //Clear all objective functions
    mpVariablesList->clear();
    QTreeWidgetItem *pAliasItem = new QTreeWidgetItem(QStringList() << "_Alias");
    QFont aliasFont = pAliasItem->font(0);
    aliasFont.setBold(true);
    pAliasItem->setFont(0, aliasFont);
    mpVariablesList->insertTopLevelItem(0, pAliasItem);
    QStringList aliasNames = mpSystem->getAliasNames();
    for(int a=0; a<aliasNames.size(); ++a)
    {
        QTreeWidgetItem *pVariableItem = new QTreeWidgetItem(QStringList() << aliasNames.at(a));
        pVariableItem->setCheckState(0, Qt::Unchecked);
        pAliasItem->insertChild(0, pVariableItem);
    }

    for(int c=0; c<componentNames.size(); ++c)
    {
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem(QStringList() << componentNames.at(c));
        QFont componentFont = pComponentItem->font(0);
        componentFont.setBold(true);
        pComponentItem->setFont(0, componentFont);
        mpVariablesList->insertTopLevelItem(0, pComponentItem);
        QList<Port*> ports = mpSystem->getModelObject(componentNames.at(c))->getPortListPtrs();
        for(int p=0; p<ports.size(); ++p)
        {
            QTreeWidgetItem *pPortItem = new QTreeWidgetItem(QStringList() << ports.at(p)->getName());
            QVector<QString> varNames, portUnits;
            mpSystem->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(componentNames.at(c), ports.at(p)->getName(), varNames, portUnits);
            for(int v=0; v<varNames.size(); ++v)
            {
                QTreeWidgetItem *pVariableItem = new QTreeWidgetItem(QStringList() << varNames.at(v));
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

        mpParameterLabels.at(i)->deleteLater();
        mpParameterMinLineEdits.at(i)->deleteLater();
        mpParameterMaxLineEdits.at(i)->deleteLater();
        mpParameterRemoveButtons.at(i)->deleteLater();
    }

    mpParameterLabels.clear();
    mpParameterMinLineEdits.clear();
    mpParameterMaxLineEdits.clear();
    mpParameterRemoveButtons.clear();

    //mpRunButton->setDisabled(true);

    loadConfiguration();

    QDialog::show();
}


//! @brief Slot that handles accept event (clicking "Close" button)
void OptimizationScriptWizard::accept()
{
    if(mSelectedFunctions.isEmpty())
    {
        //Error message (cannot accept wizard with illegal settings)
        return;
    }
    else
    {
        if(generateScript())
        {
            gpOptimizationDialog->setCode(mScript);
            saveConfiguration();
            QDialog::accept();
        }
    }
}


//! @brief Toggles visibility of widgets depending on specified algorithm
void OptimizationScriptWizard::setAlgorithm(int i)
{
    mpAlphaLabel->setVisible(false);
    mpAlphaLineEdit->setVisible(false);
    mpBetaLabel->setVisible(false);
    mpBetaLineEdit->setVisible(false);
    mpGammaLabel->setVisible(false);
    mpGammaLineEdit->setVisible(false);
    mpRhoLabel->setVisible(false);
    mpRhoLineEdit->setVisible(false);
    mpSigmaLabel->setVisible(false);
    mpSigmaLineEdit->setVisible(false);
    mpOmega1Label->setVisible(false);
    mpOmega1LineEdit->setVisible(false);
    mpOmega2Label->setVisible(false);
    mpOmega2LineEdit->setVisible(false);
    mpC1Label->setVisible(false);
    mpC1LineEdit->setVisible(false);
    mpC2Label->setVisible(false);
    mpC2LineEdit->setVisible(false);
    mpVmaxLabel->setVisible(false);
    mpVmaxLineEdit->setVisible(false);
    mpFLabel->setVisible(false);
    mpFLineEdit->setVisible(false);
    mpCRLabel->setVisible(false);
    mpCRLineEdit->setVisible(false);
    mpCPLabel->setVisible(false);
    mpCPLineEdit->setVisible(false);
    mpElitesLabel->setVisible(false);
    mpElitesLineEdit->setVisible(false);
    mpMPLabel->setVisible(false);
    mpMPLineEdit->setVisible(false);
    mpLengthLabel->setVisible(false);
    mpLengthSpinBox->setVisible(false);
    mpPercDiffLabel->setVisible(false);
    mpPercDiffLineEdit->setVisible(false);
    mpCountMaxLabel->setVisible(false);
    mpCountMaxSpinBox->setVisible(false);
    mpMethodLabel->setVisible(false);
    mpMethodComboBox->setVisible(false);

    ++i;    //i=0 means undefined
    switch(i)
    {
    case Ops::NelderMead:
        mpAlphaLabel->setVisible(true);
        mpAlphaLineEdit->setVisible(true);
        mpGammaLabel->setVisible(true);
        mpGammaLabel->setText("Expansion Factor");  //Used by multiple algorithms
        mpGammaLineEdit->setVisible(true);
        mpRhoLabel->setVisible(true);
        mpRhoLineEdit->setVisible(true);
        mpSigmaLabel->setVisible(true);
        mpSigmaLineEdit->setVisible(true);
        mpAlphaLineEdit->setText("1.0");
        mpGammaLineEdit->setText("2.0");
        mpRhoLineEdit->setText("-0.5");
        mpSigmaLineEdit->setText("0.5");
        break;
    case Ops::ComplexRF:
        mpAlphaLabel->setVisible(true);
        mpAlphaLineEdit->setVisible(true);
        mpBetaLabel->setVisible(true);
        mpBetaLineEdit->setVisible(true);
        mpGammaLabel->setVisible(true);
        mpGammaLabel->setText("Forgetting Factor");  //Used by multiple algorithms
        mpGammaLineEdit->setVisible(true);
        mpAlphaLineEdit->setText("1.3");
        mpBetaLineEdit->setText("0.3");
        mpRhoLineEdit->setText("0.3");
        break;
    case Ops::ComplexRFP:
        mpAlphaLabel->setVisible(true);
        mpAlphaLineEdit->setVisible(true);
        mpBetaLabel->setVisible(true);
        mpBetaLineEdit->setVisible(true);
        mpGammaLabel->setVisible(true);
        mpGammaLabel->setText("Forgetting Factor");  //Used by multiple algorithms
        mpGammaLineEdit->setVisible(true);
        mpNumModelsLabel->setVisible(true);
        mpNumModelsLineEdit->setVisible(true);
        mpMethodLabel->setVisible(true);
        mpMethodComboBox->setVisible(true);
        break;
    case Ops::ParticleSwarm:
        mpOmega1Label->setVisible(true);
        mpOmega1LineEdit->setVisible(true);
        mpOmega2Label->setVisible(true);
        mpOmega2LineEdit->setVisible(true);
        mpC1Label->setVisible(true);
        mpC1LineEdit->setVisible(true);
        mpC2Label->setVisible(true);
        mpC2LineEdit->setVisible(true);
        mpVmaxLabel->setVisible(true);
        mpVmaxLineEdit->setVisible(true);
        break;
    case Ops::DifferentialEvolution:
        mpFLabel->setVisible(true);
        mpFLineEdit->setVisible(true);
        mpCRLabel->setVisible(true);
        mpCRLineEdit->setVisible(true);
        break;
    case Ops::Genetic:
        mpCPLabel->setVisible(true);
        mpCPLineEdit->setVisible(true);
        mpMPLabel->setVisible(true);
        mpMPLineEdit->setVisible(true);
        mpElitesLabel->setVisible(true);
        mpElitesLineEdit->setVisible(true);
    case Ops::ParameterSweep:
        break;
    default:
        break;
    }
}


//! @brief Adds a new parameter to the list of selected parameter, and displays it in dialog
//! @param item Tree widget item which represents parameter
void OptimizationScriptWizard::updateChosenParameters(QTreeWidgetItem* item, int /*i*/)
{
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedComponents.append(item->parent()->text(0));
        mSelectedParameters.append(item->text(0));
        QString currentValue;
        if(item->parent()->text(0) == "_System Parameters")
        {
            currentValue = mpSystem->getParameterValue(item->text(0));
        }
        else
        {
            currentValue = mpSystem->getModelObject(item->parent()->text(0))->getParameterValue(item->text(0));
        }

        QLabel *pLabel = new QLabel(trUtf8(" <  ") + item->parent()->text(0) + ", " + item->text(0) + " (" + currentValue + trUtf8(")  < "));
        pLabel->setAlignment(Qt::AlignCenter);

        OptimizationSettings optSettings;
        mpSystem->getOptimizationSettings(optSettings);
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
        pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Discard.svg"));
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

        mpParameterLabels.at(i)->deleteLater();
        mpParameterMinLineEdits.at(i)->deleteLater();
        mpParameterMaxLineEdits.at(i)->deleteLater();
        mpParameterRemoveButtons.at(i)->deleteLater();

        mpParameterLabels.removeAt(i);
        mpParameterMinLineEdits.removeAt(i);
        mpParameterMaxLineEdits.removeAt(i);
        mpParameterRemoveButtons.removeAt(i);

        mSelectedParameters.removeAt(i);
        mSelectedComponents.removeAt(i);
    }
}


//! @brief Removes an objective function from the selected functions
void OptimizationScriptWizard::removeParameter()
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

    mpParameterLabels.at(i)->deleteLater();
    mpParameterMinLineEdits.at(i)->deleteLater();
    mpParameterMaxLineEdits.at(i)->deleteLater();
    mpParameterRemoveButtons.at(i)->deleteLater();

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
void OptimizationScriptWizard::updateChosenVariables(QTreeWidgetItem *item, int /*i*/)
{
    QStringList variable;
    if(item->parent()->text(0) == "_Alias")
    {
        variable << "" << "" << item->text(0);
    }
    else
    {
        variable << item->parent()->parent()->text(0) << item->parent()->text(0) << item->text(0);
    }
    mSelectedVariables.removeAll(variable);
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedVariables.append(variable);
    }
}


//! @brief Adds a new objective function from combo box and selected variables
void OptimizationScriptWizard::addFunction()
{
    int idx = mpFunctionsComboBox->currentIndex();
    addObjectiveFunction(idx, 1.0, 1.0, 2.0, mSelectedVariables, QStringList());
    emit(contentsChanged());
}


//! @brief Removes an objective function from the selected functions
void OptimizationScriptWizard::removeFunction()
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

    emit(contentsChanged());
}


//! @brief Generates the script code and shows it in the output box
void OptimizationScriptWizard::update(int idx)
{
    button(QWizard::CustomButton3)->setVisible(false);      //Should be hidden on all tabs except code tab

    //Finished parameters tab
    if(idx == 2)
    {
        if(mSelectedParameters.isEmpty())
        {
            return;
        }
    }
}


//! @brief Saves wizard settings to configuration
void OptimizationScriptWizard::saveConfiguration()
{
    if(!mpSystem)
    {
        return;
    }

    OptimizationSettings optSettings;

    //Settings
    optSettings.mNiter = mpIterationsSpinBox->value();
    optSettings.mNsearchp = mpParticlesSpinBox->value();
    optSettings.mRefcoeff = mpAlphaLineEdit->text().toDouble();
    optSettings.mRandfac = mpBetaLineEdit->text().toDouble();
    optSettings.mForgfac = mpGammaLineEdit->text().toDouble();
    optSettings.mPartol = mpEpsilonXLineEdit->text().toDouble();
    optSettings.mSavecsv = mpExport2CSVBox->isChecked();
    optSettings.mFinalEval = mpFinalEvalCheckBox->isChecked();
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

    mpSystem->setOptimizationSettings(optSettings);
}


//! @brief Generates the script based upon selections made in the dialog
bool OptimizationScriptWizard::generateScript()
{
    for(int i=0; i<mpParameterMinLineEdits.size(); ++i)
    {
        double minVal = mpParameterMinLineEdits[i]->text().toDouble();
        double maxVal = mpParameterMaxLineEdits[i]->text().toDouble();
        if(maxVal < minVal)
        {
            QMessageBox::warning(this, "Error", "All maximum parameter values must be greater than their minimum parameter values.");
            return false;
        }
        if(minVal <= 0 && mpParametersLogCheckBox->isChecked())
        {
            QMessageBox::warning(this, "Error", "Logarithmic scaling requires all parameters to be strictly greater than zero.");
            return false;
        }
    }

    bool algorithmOk=true;
    int idx = mpAlgorithmBox->currentIndex()+1;
    switch (idx)
    {
    case Ops::NelderMead :
        generateNelderMeadScript();
        break;
    case Ops::ComplexRF :
        generateComplexRFScript("complexrf");
        break;
    case Ops::ComplexRFP :
        generateComplexRFScript("complexrfp");
        break;
    case Ops::ParticleSwarm :
        generateParticleSwarmScript();
        break;
    case Ops::DifferentialEvolution :
        generateDifferentialEvolutionScript();
        break;
    case Ops::Genetic :
        generateGeneticScript();
        break;
    case Ops::ParameterSweep :
        generateParameterSweepScript();
        break;
    default:
        algorithmOk=false;
    }

    return algorithmOk;
}


//! @brief Generates script for Nelder-Mead
void OptimizationScriptWizard::generateNelderMeadScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateNelderMead.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    generateObjectiveFunctionCode(templateCode);
    generateParameterCode(templateCode);
    generateCommonOptions(templateCode);

    templateCode.replace("<<<alpha>>>", mpAlphaLineEdit->text());
    templateCode.replace("<<<gamma>>>", mpGammaLineEdit->text());
    templateCode.replace("<<<rho>>>", mpRhoLineEdit->text());
    templateCode.replace("<<<sigma>>>", mpSigmaLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());

    mScript = templateCode;
}


//! @brief Generates script for Complex-RF
void OptimizationScriptWizard::generateComplexRFScript(const QString &subAlgorithm)
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateComplex.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    generateObjectiveFunctionCode(templateCode);
    generateParameterCode(templateCode);
    generateCommonOptions(templateCode);

    QString extraVars;
    int nmodels = mpNumModelsLineEdit->text().toInt();
    if(subAlgorithm == "complexrfp")
    {
        extraVars.append("opt set nmodels "+QString::number(nmodels));
        if(mpMethodComboBox->currentIndex() == 0)
        {
            extraVars.append("\nopt set method 0");
            int nstep = qMax(1,nmodels/2);
            int nret = nmodels-nstep;
            extraVars.append("\nopt set npredictions "+QString::number(nstep));
            extraVars.append("\nopt set nretractions "+QString::number(nret));
        }
        else
        {
            extraVars.append("\nopt set method 1");
            extraVars.append("\nopt set ndist "+QString::number(nmodels));
            extraVars.append("\nopt set alphamin 0.0");
            extraVars.append("\nopt set alphamin 2.0");
        }
    }

    templateCode.replace("<<<subalgorithm>>>", subAlgorithm);
    templateCode.replace("<<<alpha>>>", mpAlphaLineEdit->text());
    templateCode.replace("<<<beta>>>", mpBetaLineEdit->text());
    templateCode.replace("<<<gamma>>>", mpGammaLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());
    templateCode.replace("<<<extravars>>>", extraVars);

    mScript = templateCode;
}


//! @brief Generates script for particle swarm optimization
void OptimizationScriptWizard::generateParticleSwarmScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateParticle.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    generateObjectiveFunctionCode(templateCode);
    generateParameterCode(templateCode);
    generateCommonOptions(templateCode);

    templateCode.replace("<<<omega1>>>", mpOmega1LineEdit->text());
    templateCode.replace("<<<omega2>>>", mpOmega2LineEdit->text());
    templateCode.replace("<<<c1>>>", mpC1LineEdit->text());
    templateCode.replace("<<<c2>>>", mpC2LineEdit->text());
    templateCode.replace("<<<vmax>>>", mpVmaxLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());

    mScript = templateCode;
}


//! @brief Generates script for differential evolution
void OptimizationScriptWizard::generateDifferentialEvolutionScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateDifferential.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    generateObjectiveFunctionCode(templateCode);
    generateParameterCode(templateCode);
    generateCommonOptions(templateCode);

    templateCode.replace("<<<f>>>", mpFLineEdit->text());
    templateCode.replace("<<<cr>>>", mpCRLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());

    mScript = templateCode;
}


//! @brief Generates script for genetic algorithm
void OptimizationScriptWizard::generateGeneticScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateGenetic.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    generateObjectiveFunctionCode(templateCode);
    generateParameterCode(templateCode);
    generateCommonOptions(templateCode);

    templateCode.replace("<<<cp>>>", mpCPLineEdit->text());
    templateCode.replace("<<<mp>>>", mpMPLineEdit->text());
    templateCode.replace("<<<elites>>>", mpElitesLineEdit->text());
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());
    templateCode.replace("<<<nmodels>>>", mpNumModelsLineEdit->text());

    mScript = templateCode;
}


//! @brief Generates script for parameter sweep
void OptimizationScriptWizard::generateParameterSweepScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateParameterSweep.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    generateObjectiveFunctionCode(templateCode);
    generateParameterCode(templateCode);
    generateCommonOptions(templateCode);

    int nThreads = gpConfig->getIntegerSetting(cfg::numberofthreads);
    templateCode.replace("<<<evals>>>", QString::number(mpLengthSpinBox->value()/double(nThreads)));
    templateCode.replace("<<<nmodels>>>", QString::number(nThreads));


    mScript = templateCode;
}


//! @brief  Generates objective function code to script
void OptimizationScriptWizard::generateObjectiveFunctionCode(QString &templateCode)
{
    QString objFuncs, totalObj;
    objFuncs.append("echo off\n");
    for(int i=0; i<mFunctionName.size(); ++i)
    {
        QString objFunc = mObjectiveFunctionCalls[mObjectiveFunctionDescriptions.indexOf(mFunctionName[i])];
        objFunc.replace("<<<id>>>", QString::number(i+1));
        for(int j=0; j<mFunctionComponents[i].size(); ++j)
        {
            QString varName;
            if(mFunctionComponents[i][j].isEmpty())   //Alias
            {
                varName = mFunctionVariables[i][j];
            }
            else
            {
                varName = mFunctionComponents[i][j]+"."+mFunctionPorts[i][j]+"."+mFunctionVariables[i][j];
            }
            gpTerminalWidget->mpHandler->toShortDataNames(varName);
            objFunc.replace("<<<var"+QString::number(j+1)+">>>", varName);

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
        totalObj.append(mWeightLineEditPtrs[i]->text()+"*"+mNormLineEditPtrs[i]->text()+"*exp("+mExpLineEditPtrs[i]->text()+")*obj"+idx);
    }
    objFuncs.append("echo on");

    replacePattern("<<<objfuncs>>>", objFuncs, templateCode);
    replacePattern("<<<totalobj>>>", totalObj, templateCode);
}


//! @brief Generates parameter code to script
void OptimizationScriptWizard::generateParameterCode(QString &templateCode)
{
    QString setMinMax, setPars;
    setPars.append("echo off\n");
    for(int p=0; p<mSelectedParameters.size(); ++p)
    {
        QString par;
        if(mSelectedComponents[p] == "_System Parameters")
        {
            par = mSelectedParameters[p];
        }
        else
        {
            par = mSelectedComponents[p]+"."+mSelectedParameters[p];
        }
        gpTerminalWidget->mpHandler->toShortDataNames(par);
        setPars.append("chpa "+par+" optpar(optvar(evalid),"+QString::number(p)+")\n");

        setMinMax.append("opt set limits "+QString::number(p)+" "+mpParameterMinLineEdits[p]->text()+" "+mpParameterMaxLineEdits[p]->text()+"\n");
    }
    setPars.append("echo on");
    // Remove last newlines
    setMinMax.chop(1);

    replacePattern("<<<setminmax>>>", setMinMax, templateCode);
    replacePattern("<<<setpars>>>", setPars, templateCode);
}


//! @brief Generates common options to script
void OptimizationScriptWizard::generateCommonOptions(QString &templateCode)
{
    if(mpExport2CSVBox->isChecked())
    {
        templateCode.replace("<<<log>>>","on");
    }
    else
    {
        templateCode.replace("<<<log>>>","off");
    }
    if(mpFinalEvalCheckBox->isChecked())
    {
        templateCode.replace("<<<finaleval>>>","on");
    }
    else
    {
        templateCode.replace("<<<finaleval>>>","off");
    }
    if(mpPlotParticlesCheckBox->isChecked())
    {
        replacePattern("<<<plotpoints>>>", "on", templateCode);
    }
    else
    {
        replacePattern("<<<plotpoints>>>", "off", templateCode);
    }
    if(mpPlotEntropyCheckBox->isChecked())
    {
        templateCode.replace("<<<plotentropy>>>","on");
    }
    else
    {
        templateCode.replace("<<<plotentropy>>>","off");
    }
    if(mpPlotBestWorstCheckBox->isChecked())
    {
        templateCode.replace("<<<plotobjectives>>>","on");
    }
    else
    {
        templateCode.replace("<<<plotobjectives>>>","off");
    }

    templateCode.replace("<<<npoints>>>", QString::number(mpParticlesSpinBox->value()));
    templateCode.replace("<<<nparams>>>", QString::number(mSelectedParameters.size()));
    templateCode.replace("<<<maxevals>>>", QString::number(mpIterationsSpinBox->value()));
}


//! @brief Returns the calling code to one of the selected functions
//! @param i Index of selected function
QString OptimizationScriptWizard::generateFunctionCode(int i)
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

    QString w = mWeightLineEditPtrs.at(i)->text();
    QString n = mNormLineEditPtrs.at(i)->text();
    QString e = mExpLineEditPtrs.at(i)->text();

    retval.append(w+QString().setNum(i)+"*("+mObjectiveFunctionCalls.at(fnc)+"(data"+QString().setNum(i)+"0");
    for(int v=1; v<mObjectiveFunctionNumberOfVariables.at(fnc); ++v)
        retval.append(", data"+QString().setNum(i)+QString().setNum(v));
    if(mObjectiveFunctionUsesTimeVector.at(fnc))
        retval.append(", time");
    for(int d=0; d<mObjectiveFunctionDataLists.at(fnc).size(); ++d)
    {
        double num = mDataLineEditPtrs.at(i).at(d)->text().toDouble();
        retval.append(", "+QString().setNum(num));
    }
    retval.append(")/"+n+QString().setNum(i)+")**"+e+QString().setNum(i));

    return retval;
}


//! @brief Checks if number of selected variables is correct. Gives error messages if they are too many or too low.
//! @param i Selected objective function
bool OptimizationScriptWizard::verifyNumberOfVariables(int idx, int nSelVar, bool printWarning)
{
    int nVar = mObjectiveFunctionNumberOfVariables.at(idx);

    if(nSelVar != nVar)
    {
        if(printWarning)
        {
            QMessageBox::warning(this, QString("Wrong number of arguments"),
                                 QString("The selected function requires exactly %1 variables, but you selected %2.").arg(nVar).arg(nSelVar),
                                 QMessageBox::Ok);
        }
        return false;
    }
    return true;
}


//! @brief Loads objective functions from template files
bool OptimizationScriptWizard::loadObjectiveFunctions()
{
    mObjectiveFunctionDescriptions.clear();
    mObjectiveFunctionCalls.clear();
    mObjectiveFunctionDataLists.clear();
    mObjectiveFunctionNumberOfVariables.clear();
    mObjectiveFunctionUsesTimeVector.clear();

    // Look in both local and global scripts directory in case they are different

    QDir scriptsDir(gpDesktopHandler->getScriptsPath()+"/HCOM/objFuncTemplates");
    QStringList files = scriptsDir.entryList(QStringList() << "*.hcom");
    int f=0;
    for(; f<files.size(); ++f)
    {
        files[f].prepend(scriptsDir.absolutePath()+"/");
    }
    QDir localScriptsDir(gpDesktopHandler->getExecPath()+"/../Scripts/HCOM/objFuncTemplates");
    files.append(localScriptsDir.entryList(QStringList() << "*.hcom"));
    for(int g=f; g<files.size(); ++g)
    {
        files[g].prepend(localScriptsDir.absolutePath()+"/");
    }
    files.removeDuplicates();

    for(const QString &fileName : files) {
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


//! @brief Loads wizard settings from configuration
void OptimizationScriptWizard::loadConfiguration()
{
    OptimizationSettings optSettings;
    mpSystem->getOptimizationSettings(optSettings);

    mpIterationsSpinBox->setValue(optSettings.mNiter);
    mpParticlesSpinBox->setValue(optSettings.mNsearchp);
    mpAlphaLineEdit->setText(QString().setNum(optSettings.mRefcoeff));
    mpBetaLineEdit->setText(QString().setNum(optSettings.mRandfac));
    mpGammaLineEdit->setText(QString().setNum(optSettings.mForgfac));
    mpEpsilonXLineEdit->setText(QString().setNum(optSettings.mPartol));
    mpExport2CSVBox->setChecked(optSettings.mSavecsv);
    mpFinalEvalCheckBox->setChecked(optSettings.mFinalEval);
    mpParametersLogCheckBox->setChecked(optSettings.mlogPar);

    //Parameters
    for(int i=0; i<optSettings.mParamters.size(); ++i)
    {
        //Check if component and parameter exists before checking the tree item (otherwise tree item does not exist = crash)
        if(gpModelHandler->getCurrentViewContainerObject()->hasModelObject(optSettings.mParamters.at(i).mComponentName) &&
           gpModelHandler->getCurrentViewContainerObject()->getModelObject(optSettings.mParamters.at(i).mComponentName)->getParameterNames().contains(optSettings.mParamters.at(i).mParameterName))
        {
            QTreeWidgetItem *pItem = findParameterTreeItem(optSettings.mParamters.at(i).mComponentName, optSettings.mParamters.at(i).mParameterName);
            if(!pItem->isDisabled())
            {
                pItem->setCheckState(0, Qt::Checked);
            }
        }
    }
    //Objectives
    for(int i=0; i<optSettings.mObjectives.size(); ++i)
    {
        //! @todo Find a good way of setting the objective functions

        auto& objectives = optSettings.mObjectives.at(i);
        int idx = mpFunctionsComboBox->findText(objectives.mFunctionName);
        if(idx > -1) //found!
        {//Lägg till variabel i XML -> compname, portname, varname, ska vara i mSelectedVariables
            mpFunctionsComboBox->setCurrentIndex(idx);
            addObjectiveFunction(idx, objectives.mWeight, objectives.mNorm, objectives.mExp, objectives.mVariableInfo, objectives.mData, false);
        }
    }
}

//! @brief Adds a new objective function
//! @param[in] idx                  Function index
//! @param[in] weight               Weight factor of objective function
//! @param[in] norm                 Normalization factor of objective function
//! @param[in] exp                  Expononential of objective function
//! @param[in] selectedVariables    List with selected model variables
//! @param[in] objData              Scalar parameters for optimization function
//! @param[in] printWarning         Tells whether or not to warn user when using wrong number of variables
void OptimizationScriptWizard::addObjectiveFunction(int idx, double weight, double norm, double exp, QList<QStringList> selectedVariables, QStringList objData, bool printWarning)
{
    if(!verifyNumberOfVariables(idx, selectedVariables.size(), printWarning))
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

    QString variablesText;
    if(mFunctionComponents.last().first().isEmpty())
    {
        variablesText = "<b>"+mFunctionVariables.last().first()+"</b>";
    }
    else
    {
        variablesText = "<b>"+mFunctionComponents.last().first()+", "+mFunctionPorts.last().first()+", "+mFunctionVariables.last().first()+"</b>";
    }
    for(int i=1; i<mFunctionVariables.last().size(); ++i)
    {
        if(mFunctionComponents.last().at(i).isEmpty())
        {
            variablesText.append(" and <b>"+mFunctionVariables.last().at(i)+"</b>");
        }
        else
        {
            variablesText.append(" and <b>" + mFunctionComponents.last().at(i)+", "+mFunctionPorts.last().at(i)+", "+mFunctionVariables.last().at(i)+"</b>");
        }
    }
    QLabel *pFunctionLabel = new QLabel(mpMinMaxComboBox->currentText() + " " + mObjectiveFunctionDescriptions.at(idx)+" for "+variablesText, this);
    //QLabel::setTextFormat(Qt::AutoText);
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
    pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Discard.svg"));
    pRemoveButton->setToolTip("Remove Function");
    pWeightLineEdit->setMaximumWidth(60);
    pNormLineEdit->setMaximumWidth(60);
    pExpLineEdit->setMaximumWidth(60);
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
