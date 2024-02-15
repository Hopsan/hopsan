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
//! @file   SensitivityAnalysisDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the sensitivity analysis dialog
//!
//$Id$

#include <QGroupBox>
#include <QDialogButtonBox>
#include <QAction>
#include <QProgressBar>
#include <QTreeWidget>
#include <QPushButton>
#include <QRadioButton>

#include "global.h"
#include "Configuration.h"
#include "GUIPort.h"
#include "PlotWindow.h"
#include "DesktopHandler.h"
#include "Dialogs/SensitivityAnalysisDialog.h"
#include "Utilities/HelpPopUpWidget.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/ModelWidget.h"
#include "ModelHandler.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotArea.h"
#include "PlotCurve.h"
#include "MessageHandler.h"
#include "SimulationThreadHandler.h"
#include "OpsWorkerParameterSweep.h"
#include "OptimizationHandler.h"

#ifndef _WIN32
#include <unistd.h> //Needed for sysctl
#endif

#ifdef USEZMQ
#include "RemoteSimulationUtils.h"
#endif

//! @brief Constructor
SensitivityAnalysisDialog::SensitivityAnalysisDialog(QWidget *parent)
    : QDialog(parent)
{
    //Set the name and size of the main window
    this->resize(1024,768);
    this->setWindowTitle("Sensitivity Analysis");
    this->setPalette(gpConfig->getPalette());

    //Settings
    mpSettings = new SensitivityAnalysisSettings();

    //Parameters list
    QLabel *pParametersLabel = new QLabel("Choose uncertain parameters, and specify their standard deviation.");
    QFont boldFont = pParametersLabel->font();
    boldFont.setBold(true);
    //mpParametersLabel->setFont(boldFont);
    mpParametersList = new QTreeWidget(this);
    QLabel *pParameterNameLabel = new QLabel("Parameter Name");
    QLabel *pParameterAverageLabel = new QLabel("Mean Value");
    QLabel *pParameterSigmaLabel = new QLabel("Standard Deviation");
    QLabel *pParameterMinLabel = new QLabel("Minimum Value");
    QLabel *pParameterMaxLabel = new QLabel("Maximum Value");
    pParameterAverageLabel->setVisible(false);
    pParameterSigmaLabel->setVisible(false);
    pParameterNameLabel->setFont(boldFont);
    pParameterAverageLabel->setFont(boldFont);
    pParameterSigmaLabel->setFont(boldFont);
    pParameterMinLabel->setFont(boldFont);
    pParameterMaxLabel->setFont(boldFont);
    mpParametersLayout = new QGridLayout(this);
    mpParametersLayout->addWidget(pParametersLabel,        0, 0, 1, 3);
    mpParametersLayout->addWidget(mpParametersList,        1, 0, 1, 3);
    mpParametersLayout->addWidget(pParameterNameLabel,     2, 0, 1, 1);
    mpParametersLayout->addWidget(pParameterAverageLabel,  2, 1, 1, 1);
    mpParametersLayout->addWidget(pParameterSigmaLabel,    2, 2, 1, 1);
    mpParametersLayout->addWidget(pParameterMinLabel,      2, 1, 1, 1);
    mpParametersLayout->addWidget(pParameterMaxLabel,      2, 2, 1, 1);
    QGroupBox *pParametersGroupBox = new QGroupBox(this);
    pParametersGroupBox->setLayout(mpParametersLayout);

    //Output variables list
    QLabel *pOutputLabel = new QLabel("Choose output variables:");
    mpOutputList = new QTreeWidget(this);
    QLabel *pOutputNameLabel = new QLabel("Variable Name");
    pOutputNameLabel->setFont(boldFont);
    mpOutputLayout = new QGridLayout(this);
    mpOutputLayout->addWidget(pOutputLabel,        0, 0, 1, 3);
    mpOutputLayout->addWidget(mpOutputList,         1, 0, 1, 3);
    mpOutputLayout->addWidget(pOutputNameLabel,     2, 0, 1, 1);
    QGroupBox *pOutputGroupBox = new QGroupBox(this);
    pOutputGroupBox->setLayout(mpOutputLayout);

    //Settings
    QLabel *pDistributionTypeLabel = new QLabel("Distribution type: ");
    mpUniformDistributionRadioButton = new QRadioButton("Uniform distribution", this);
    mpUniformDistributionRadioButton->setChecked(true);
    mpNormalDistributionRadioButton = new QRadioButton("Normal distribution", this);
    QVBoxLayout *pDistributionRadioButtonsLayout = new QVBoxLayout(this);
    pDistributionRadioButtonsLayout->addWidget(mpUniformDistributionRadioButton);
    pDistributionRadioButtonsLayout->addWidget(mpNormalDistributionRadioButton);
    QWidget *pDistributionGroupBox = new QWidget(this);
    pDistributionGroupBox->setLayout(pDistributionRadioButtonsLayout);
    QHBoxLayout *pDistributionLayout = new QHBoxLayout();
    pDistributionLayout->addWidget(pDistributionTypeLabel);
    pDistributionLayout->addWidget(pDistributionGroupBox);

    QLabel *pStepsLabel = new QLabel("Number of simulation steps: ");
    mpStepsSpinBox = new QSpinBox(this);
    mpStepsSpinBox->setValue(100);
    mpStepsSpinBox->setMinimum(1);
    mpStepsSpinBox->setMaximum(1000000);
    mpStepsSpinBox->setSingleStep(1);
    QHBoxLayout *pStepsLayout = new QHBoxLayout();
    pStepsLayout->addWidget(pStepsLabel);
    pStepsLayout->addWidget(mpStepsSpinBox);

    mpUseRemoteSimulatorsCheckBox = new QCheckBox("Use remote simulators");
    mpNumRemoteParallelModelsSpinBox = new QSpinBox(this);
    mpNumRemoteParallelModelsSpinBox->setRange(1, 2048);
    mpNumRemoteParallelModelsSpinBox->setSingleStep(1);
    mpNumRemoteParallelModelsSpinBox->setValue(2);
    QHBoxLayout *pNumRemoteLayout = new QHBoxLayout();
    pNumRemoteLayout->addWidget(new QLabel("Number of remote parallel models: ", this));
    pNumRemoteLayout->addWidget(mpNumRemoteParallelModelsSpinBox);

    QGroupBox *pSettingsGroupBox = new QGroupBox();
    QVBoxLayout *pSettingsLayout = new QVBoxLayout();
    pSettingsLayout->addLayout(pDistributionLayout);
    pSettingsLayout->addLayout(pStepsLayout);
    pSettingsLayout->addWidget(mpUseRemoteSimulatorsCheckBox);
    pSettingsLayout->addLayout(pNumRemoteLayout);

    pSettingsGroupBox->setLayout(pSettingsLayout);

    //Buttons
    QPushButton *pCancelButton = new QPushButton(tr("&Close"), this);
    pCancelButton->setAutoDefault(false);
    mpAbortButton = new QPushButton(tr("&Abort"), this);
    QPushButton *pRunButton = new QPushButton(tr("&Start Analysis"), this);
    pRunButton->setDefault(true);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpAbortButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pRunButton, QDialogButtonBox::ActionRole);

    //Toolbar
    QAction *pHelpAction = new QAction("Show Context Help", this);
    pHelpAction->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Help.svg"));
    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pHelpAction);

    //Progress bar
    QLabel *pProgressLabel = new QLabel("Progress:", this);
    mpProgressBar = new QProgressBar(this);


    //Main layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(pParametersGroupBox,   0, 0, 1, 1);
    pLayout->addWidget(pOutputGroupBox,       0, 1, 1, 2);
    pLayout->addWidget(pSettingsGroupBox,     1, 0, 3, 1);
    pLayout->addWidget(pProgressLabel,        1, 1, 1, 2);
    pLayout->addWidget(mpProgressBar,          2, 1, 1, 2);
    pLayout->addWidget(pButtonBox,            3, 2, 1, 1);
    pLayout->addWidget(pToolBar,              3, 1, 1, 1);
    setLayout(pLayout);

    //Connections
    connect(pCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(pRunButton,                    SIGNAL(clicked()),      this,                   SLOT(run()));
    connect(pHelpAction,                   SIGNAL(triggered()),    gpHelpPopupWidget,           SLOT(openContextHelp()));
    connect(mpNormalDistributionRadioButton, SIGNAL(toggled(bool)), pParameterAverageLabel, SLOT(setVisible(bool)));
    connect(mpNormalDistributionRadioButton, SIGNAL(toggled(bool)), pParameterSigmaLabel, SLOT(setVisible(bool)));
    connect(mpUniformDistributionRadioButton, SIGNAL(toggled(bool)), pParameterMinLabel, SLOT(setVisible(bool)));
    connect(mpUniformDistributionRadioButton, SIGNAL(toggled(bool)), pParameterMaxLabel, SLOT(setVisible(bool)));
    connect(this, SIGNAL(accepted()), SLOT(saveSettings()));
    connect(this, SIGNAL(rejected()), SLOT(saveSettings()));
}

//! @brief Reimplementation of open() slot, used to initialize the dialog
void SensitivityAnalysisDialog::open()
{
    if(gpModelHandler->count() == 0)
    {
        return;
    }
    mpModel = gpModelHandler->getCurrentModel();
    connect(mpModel, SIGNAL(destroyed()), this, SLOT(close()));

    loadSettings();
    QDialog::open();
}


void SensitivityAnalysisDialog::loadSettings()
{
    mpParametersList->clear();
    SystemObject *pSystem = mpModel->getTopLevelSystemContainer();
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
    QTreeWidgetItem *pSystemParametersItem = new QTreeWidgetItem(QStringList() << "_System Parameters");
    QFont componentFont = pSystemParametersItem->font(0);
    componentFont.setBold(true);
    pSystemParametersItem->setFont(0, componentFont);
    mpParametersList->insertTopLevelItem(0, pSystemParametersItem);
    QStringList parameterNames = pSystem->getParameterNames();
    for(int p=0; p<parameterNames.size(); ++p)
    {
        QTreeWidgetItem *pParameterItem = new QTreeWidgetItem(QStringList() << parameterNames.at(p));
        pParameterItem->setCheckState(0, Qt::Unchecked);
        pSystemParametersItem->insertChild(0, pParameterItem);
    }
    mpParametersList->sortItems(0,Qt::AscendingOrder);
    mpParametersList->sortItems(1,Qt::AscendingOrder);
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
    mpOutputList->sortItems(0, Qt::AscendingOrder);
    connect(mpOutputList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(updateChosenVariables(QTreeWidgetItem*,int)), Qt::UniqueConnection);

    mOutputVariables.clear();
    mSelectedParameters.clear();
    mSelectedComponents.clear();

    for(int i=0; i<mpOutputLabels.size(); ++i)
    {
        mpOutputLayout->removeWidget(mpOutputLabels[i]);
        delete(mpOutputLabels[i]);
    }

    mpOutputLabels.clear();

    for(int i=0; i<mpParameterAverageLineEdits.size(); ++i)
    {
        mpParametersLayout->removeWidget(mpParameterAverageLineEdits[i]);
        mpParametersLayout->removeWidget(mpParameterLabels[i]);
        mpParametersLayout->removeWidget(mpParameterMaxLineEdits[i]);
        mpParametersLayout->removeWidget(mpParameterMinLineEdits[i]);
        mpParametersLayout->removeWidget(mpParameterSigmaLineEdits[i]);
        delete(mpParameterAverageLineEdits[i]);
        delete(mpParameterLabels[i]);
        delete(mpParameterMaxLineEdits[i]);
        delete(mpParameterMinLineEdits[i]);
        delete(mpParameterSigmaLineEdits[i]);
    }

    mpParameterAverageLineEdits.clear();
    mpParameterLabels.clear();
    mpParameterMaxLineEdits.clear();
    mpParameterMinLineEdits.clear();
    mpParameterSigmaLineEdits.clear();

    if(mpModel->getTopLevelSystemContainer())
    {
        mpModel->getTopLevelSystemContainer()->getSensitivityAnalysisSettings(*mpSettings);
    }
    else
    {
        delete mpSettings;
        mpSettings = new SensitivityAnalysisSettings();
    }

    mpStepsSpinBox->setValue(mpSettings->nIter);
    mpUniformDistributionRadioButton->setChecked(mpSettings->distribution == SensitivityAnalysisSettings::UniformDistribution);
    mpNormalDistributionRadioButton->setChecked(mpSettings->distribution == SensitivityAnalysisSettings::NormalDistribution);

    for(const SensitivityAnalysisParameter &par : mpSettings->parameters)
    {
        QTreeWidgetItemIterator it(mpParametersList);
        while(*it)
        {
            QTreeWidgetItem *pItem = (*it);
            if(pItem->parent() && pItem->parent()->text(0) == par.compName && pItem->text(0) == par.parName)
            {
                pItem->setCheckState(0, Qt::Checked);
                //updateChosenParameters(pItem, 0);
                mpParameterMinLineEdits.last()->setText(QString::number(par.min));
                mpParameterMaxLineEdits.last()->setText(QString::number(par.max));
                mpParameterAverageLineEdits.last()->setText(QString::number(par.aver));
                mpParameterSigmaLineEdits.last()->setText(QString::number(par.sigma));
            }
            ++it;
        }
    }

    for(const SensitivityAnalysisVariable &var : mpSettings->variables) {
        QTreeWidgetItemIterator it(mpOutputList);
        while(*it)
        {
            QTreeWidgetItem *pItem = (*it);
            if(pItem->parent() && pItem->parent()->parent() && pItem->parent()->text(0) == var.portName &&
               pItem->parent()->parent()->text(0) == var.compName && pItem->text(0) == var.varName)
            {
                pItem->setCheckState(0, Qt::Checked);
            }
            ++it;
        }
    }

    QDialog::show();
}

void SensitivityAnalysisDialog::saveSettings()
{
    mpSettings->nIter = mpStepsSpinBox->value();
    if(mpUniformDistributionRadioButton->isChecked())
    {
        mpSettings->distribution = SensitivityAnalysisSettings::UniformDistribution;
    }
    else if(mpNormalDistributionRadioButton->isChecked())
    {
        mpSettings->distribution = SensitivityAnalysisSettings::NormalDistribution;
    }

    mpSettings->parameters.clear();
    for(int i=0; i<mSelectedComponents.size(); ++i)
    {
        SensitivityAnalysisParameter par;
        par.compName = mSelectedComponents[i];
        par.parName = mSelectedParameters[i];
        par.min = mpParameterMinLineEdits[i]->text().toDouble();
        par.max = mpParameterMaxLineEdits[i]->text().toDouble();
        par.aver = mpParameterAverageLineEdits[i]->text().toDouble();
        par.sigma = mpParameterSigmaLineEdits[i]->text().toDouble();
        mpSettings->parameters.append(par);
    }

    mpSettings->variables.clear();
    for(int i=0; i<mOutputVariables.size(); ++i)
    {
        SensitivityAnalysisVariable var;
        var.compName = mOutputVariables[i][0];
        var.portName = mOutputVariables[i][1];
        var.varName = mOutputVariables[i][2];
        mpSettings->variables.append(var);
    }

    mpModel->getTopLevelSystemContainer()->setSensitivityAnalysisSettings(*mpSettings);

    QDialog::hide();
}


void SensitivityAnalysisDialog::updateChosenParameters(QTreeWidgetItem* item, int /*i*/)
{
    if(item->checkState(0) == Qt::Checked)
    {
        QLabel *pLabel;
        QString averageValue;
        if(item->parent()->text(0) == "_System Parameters")
        {
            mSelectedComponents.append("");
            mSelectedParameters.append(item->text(0));
            pLabel = new QLabel(item->text(0) + ": ");
            averageValue = mpModel->getTopLevelSystemContainer()->getParameterValue(item->text(0));
        }
        else
        {
            mSelectedComponents.append(item->parent()->text(0));
            mSelectedParameters.append(item->text(0));
            pLabel = new QLabel(item->parent()->text(0) + ", " + item->text(0) + ": ");
            averageValue = mpModel->getTopLevelSystemContainer()->getModelObject(item->parent()->text(0))->getParameterValue(item->text(0));
        }
        //pLabel->setAlignment(Qt::AlignCenter);

        QLineEdit *pAverageLineEdit = new QLineEdit(averageValue, this);
        QLineEdit *pSigmaLineEdit = new QLineEdit("0.0", this);
        QLineEdit *pMinLineEdit = new QLineEdit("0.0", this);
        QLineEdit *pMaxLineEdit = new QLineEdit("1.0", this);
        pSigmaLineEdit->setValidator(new QDoubleValidator());
        pMinLineEdit->setValidator(new QDoubleValidator());
        pMaxLineEdit->setValidator(new QDoubleValidator());
        mpParameterLabels.append(pLabel);
        mpParameterAverageLineEdits.append(pAverageLineEdit);
        mpParameterSigmaLineEdits.append(pSigmaLineEdit);
        mpParameterMinLineEdits.append(pMinLineEdit);
        mpParameterMaxLineEdits.append(pMaxLineEdit);
        mpParameterAverageLineEdits.last()->setVisible(mpNormalDistributionRadioButton->isChecked());
        mpParameterSigmaLineEdits.last()->setVisible(mpNormalDistributionRadioButton->isChecked());
        mpParameterMinLineEdits.last()->setVisible(mpUniformDistributionRadioButton->isChecked());
        mpParameterMaxLineEdits.last()->setVisible(mpUniformDistributionRadioButton->isChecked());
        connect(mpNormalDistributionRadioButton, SIGNAL(toggled(bool)), mpParameterAverageLineEdits.last(), SLOT(setVisible(bool)));
        connect(mpNormalDistributionRadioButton, SIGNAL(toggled(bool)), mpParameterSigmaLineEdits.last(), SLOT(setVisible(bool)));
        connect(mpUniformDistributionRadioButton, SIGNAL(toggled(bool)), mpParameterMinLineEdits.last(), SLOT(setVisible(bool)));
        connect(mpUniformDistributionRadioButton, SIGNAL(toggled(bool)), mpParameterMaxLineEdits.last(), SLOT(setVisible(bool)));

        int row = mpParametersLayout->rowCount();
        mpParametersLayout->addWidget(pLabel, row, 0);
        mpParametersLayout->addWidget(pAverageLineEdit, row, 1);
        mpParametersLayout->addWidget(pSigmaLineEdit, row, 2);
        mpParametersLayout->addWidget(pMinLineEdit, row, 1);
        mpParametersLayout->addWidget(pMaxLineEdit, row, 2);
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
            if(mSelectedComponents.at(i).isEmpty() &&
               item->parent()->text(0) == "_System Parameters" &&
               mSelectedParameters.at(i) == item->text(0))
            {
                break;
            }
        }

        //Remove widgets from layout
        mpParametersLayout->removeWidget(mpParameterLabels.at(i));
        mpParametersLayout->removeWidget(mpParameterAverageLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterSigmaLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterMinLineEdits.at(i));
        mpParametersLayout->removeWidget(mpParameterMaxLineEdits.at(i));

        //Store local pointers to widgets
        QLabel *pParameterLabel = mpParameterLabels.at(i);
        QLineEdit *pParameterAverageLineEdit = mpParameterAverageLineEdits.at(i);
        QLineEdit *pParameterSigmaLineEdit = mpParameterSigmaLineEdits.at(i);
        QLineEdit *pParameterMinLineEdit = mpParameterMinLineEdits.at(i);
        QLineEdit *pParameterMaxLineEdit = mpParameterMaxLineEdits.at(i);

        //Remove widgets from widget lists
        mpParameterLabels.removeAt(i);
        mpParameterAverageLineEdits.removeAt(i);
        mpParameterSigmaLineEdits.removeAt(i);
        mpParameterMinLineEdits.removeAt(i);
        mpParameterMaxLineEdits.removeAt(i);

        //Delete widgets
        delete(pParameterLabel);
        delete(pParameterAverageLineEdit);
        delete(pParameterSigmaLineEdit);
        delete(pParameterMinLineEdit);
        delete(pParameterMaxLineEdit);

        mSelectedParameters.removeAt(i);
        mSelectedComponents.removeAt(i);
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
//    mAborted = false;

//    DistributionEnumT type;
//    if(mpUniformDistributionRadioButton->isChecked())
//    {
//        type = UniformDistribution;
//    }
//    else
//    {
//        type = NormalDistribution;
//    }


//    int nThreads = gpConfig->getIntegerSetting(cfg::numberofthreads);
//    if(nThreads == 0)
//    {
//#ifdef _WIN32
//        std::string temp = getenv("NUMBER_OF_PROCESSORS");
//        nThreads = atoi(temp.c_str());
//#else
//        nThreads = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
//#endif
//    }
//    if(nThreads == 0)   //Extra check, just to be sure
//    {
//        nThreads = 1;
//    }
//    if(!gpConfig->getUseMulticore())
//    {
//        nThreads = 1;
//    }

//    int nSteps = mpStepsSpinBox->value();
//    int nParameteres = mSelectedParameters.size();

//    //Save hidden copy of model to load multiple copies of and run sensitivity analysis against
//    QString name = mpModel->getTopLevelSystemContainer()->getName();
//    QString appearanceDataBasePath = mpModel->getTopLevelSystemContainer()->getAppearanceData()->getBasePath();
//    QDir().mkpath(gpDesktopHandler->getDataPath()+"sensitivity/");
//    QString savePath = gpDesktopHandler->getDataPath()+"sensitivity/"+name+".hmf";
//    mpModel->saveTo(savePath);
//    mpModel->getTopLevelSystemContainer()->setAppearanceDataBasePath(appearanceDataBasePath);

//    //Load correct number of models depending on number of cores
//    mModelPtrs.clear();
//    if(gpConfig->getUseMulticore())
//    {
//        for(int i=0; i<nThreads; ++i)
//        {
//            mModelPtrs.append(gpModelHandler->loadModel(savePath, true, true));
//        }
//    }
//    else
//    {
//        mModelPtrs.append(gpModelHandler->loadModel(savePath, true, true));
//    }

//    //Add base path from original model as search path, for components that load files with relative paths
//    for(int m=0; m<mModelPtrs.size(); ++m)
//    {
//        mModelPtrs.at(m)->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->addSearchPath(appearanceDataBasePath);
//    }

//    bool progressBarOrgSetting = gpConfig->getBoolSetting(cfg::progressbar);
//    gpConfig->setBoolSetting(cfg::progressbar, false);
//    for(int i=0; i<nSteps/nThreads; ++i)
//    {
//        if(mAborted)
//        {
//            return;
//        }

//        for(int m=0; m<mModelPtrs.size(); ++m)
//        {
//            for(int p=0; p<nParameteres; ++p)
//            {
//                double randPar;
//                if(type == UniformDistribution)
//                {
//                    double min = mpParameterMinLineEdits.at(p)->text().toDouble();
//                    double max = mpParameterMaxLineEdits.at(p)->text().toDouble();
//                    randPar = uniformDistribution(min, max);
//                }
//                else
//                {
//                    randPar = normalDistribution(mpParameterAverageLineEdits.at(p)->text().toDouble(), mpParameterSigmaLineEdits.at(p)->text().toDouble());
//                }
//                if(mSelectedComponents.at(p).isEmpty())
//                {
//                    mModelPtrs[m]->getTopLevelSystemContainer()->setParameterValue(mSelectedParameters.at(p), QString().setNum(randPar));
//                }
//                else
//                {
//                    mModelPtrs[m]->getTopLevelSystemContainer()->getModelObject(mSelectedComponents.at(p))->setParameterValue(mSelectedParameters.at(p), QString().setNum(randPar));
//                }
//            }
//        }
//        if(gpConfig->getUseMulticore())
//        {
//            if(!gpModelHandler->simulateMultipleModels_blocking(mModelPtrs))
//            {
//                gpMessageHandler->addErrorMessage("Unable to perform sensitivity analysis: Failed to simulate model.");
//                gpConfig->setBoolSetting(cfg::progressbar, progressBarOrgSetting);
//                return;
//            }
//        }
//        else
//        {
//            mModelPtrs.first()->simulate_blocking();
//        }
//        mpProgressBar->setValue(double(i)*double(nThreads)/double(nSteps)*100);
//    }

//    mpProgressBar->setValue(100);   //Just to make it look better
//    gpConfig->setBoolSetting(cfg::progressbar, progressBarOrgSetting);

//    for(int v=0; v<mOutputVariables.size(); ++v)
//    {
//        int nGenerations = mModelPtrs.first()->getTopLevelSystemContainer()->getLogDataHandler()->getCurrentGenerationNumber()+1;
//        int nSamples = mModelPtrs.first()->getTopLevelSystemContainer()->getNumberOfLogSamples();

//        QVector<double> vMin(nSamples, 100000000000.0);
//        QVector<double> vMax(nSamples, -100000000000.0);
//        double totalMin=100000000000.0;
//        double totalMax=-100000000000.0;

//        QString component = mOutputVariables.at(v).at(0);
//        QString port = mOutputVariables.at(v).at(1);
//        QString variable = mOutputVariables.at(v).at(2);

//        //! @todo there is no subsystem support here right now
//        QString fullName = makeFullVariableName(QStringList(), component,port,variable);

//        //PlotWindow *pPlotWindow = mModelPtrs.first()->getViewContainerObject()->getModelObject(component)->getPort(port)->plot(variable, QString(), QColor("Blue"));




//        for(int m=0; m<mModelPtrs.size(); ++m)
//        {
//            for(int g=nGenerations-nSteps/nThreads; g<nGenerations; ++g)
//            {
//                SharedVectorVariableT pVector = mModelPtrs[m]->getTopLevelSystemContainer()->getLogDataHandler()->getVectorVariable(fullName, g);
//                if(!pVector.isNull())
//                {
//                    QVector<double> temp = pVector->getDataVectorCopy();
//                    for(int i=0; i<temp.size(); ++i)
//                    {
//                        if(temp[i] > vMax[i]) vMax[i] = temp[i];
//                        if(temp[i] < vMin[i]) vMin[i] = temp[i];
//                        if(temp[i] > totalMax) totalMax = temp[i];
//                        if(temp[i] < totalMin) totalMin = temp[i];
//                    }
//                }
//            }
//        }

//        //Commented out code = add curve for max and min
//        SharedVectorVariableT pTime = mModelPtrs.first()->getTopLevelSystemContainer()->getLogDataHandler()->getTimeVectorVariable(nGenerations-1);
//        SharedVariableDescriptionT minDesc(new VariableDescription);
//        minDesc.data()->mAliasName = "min("+fullName+")";
//        SharedVectorVariableT pMinData(new TimeDomainVariable(pTime, vMin, -1, minDesc, SharedMultiDataVectorCacheT(0)));
//        SharedVariableDescriptionT maxDesc = SharedVariableDescriptionT(new VariableDescription);
//        maxDesc.data()->mAliasName = "max("+fullName+")";
//        SharedVectorVariableT pMaxData(new TimeDomainVariable(pTime, vMax, -1, maxDesc, SharedMultiDataVectorCacheT(0)));

//        PlotWindow *pPlotWindow = gpPlotHandler->createNewUniquePlotWindow("Sensitivity Analysis");
//        gpPlotHandler->plotDataToWindow(pPlotWindow, pMaxData, QwtPlot::yLeft);
//        pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineColor(QColor(0,0,255,200));
//        gpPlotHandler->plotDataToWindow(pPlotWindow, pMinData, QwtPlot::yLeft);
//        pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineColor(QColor(0,0,255,200));
//        pPlotWindow->hidePlotCurveControls();
//        pPlotWindow->setLegendsVisible(false);

//        //! @todo Implement interval curve type support in plot window instead!
//        //! @note This is not compatible with most plot functions
//        QwtPlotIntervalCurve *pCurve = new QwtPlotIntervalCurve();
//        pCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
//        QVector<QwtIntervalSample> data;
//        QVector<double> time = pTime->getDataVectorCopy();
//        for(int i=0; i<vMin.size(); ++i)
//        {
//            data.append(QwtIntervalSample(time[i], vMin[i], vMax[i]));
//        }
//        pCurve->setSamples(data);
//        pCurve->setPen(QColor(0,0,255,150), 1.0);
//        pCurve->setBrush(QColor(0,0,255,150));

//        pCurve->attach(static_cast<QwtPlot*>(pPlotWindow->getPlotTabWidget()->getCurrentTab()->getQwtPlot()));
////        pPlotWindow->getCurrentPlotTab()->toggleAxisLock();
////        pPlotWindow->getCurrentPlotTab()->getPlot()->setAxisScale(QwtPlot::yLeft, totalMin, totalMax);
////        pPlotWindow->getCurrentPlotTab()->getPlot()->setAxisScale(QwtPlot::xBottom, time.first(), time.last());
//    }




    int nSteps = mpStepsSpinBox->value();
    int nParameters = mSelectedParameters.size();

    int nThreads = gpConfig->getIntegerSetting(cfg::numberofthreads);
    if(nThreads == 0)
    {
#ifdef _WIN32
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
    if(!gpConfig->getUseMulticore())
    {
        nThreads = 1;
    }

    QVector<QPair<double, double> > parLimits;
    for(int p=0; p<mpParameterMinLineEdits.size(); ++p)
    {
        QPair<double, double> minMax;
        minMax.first = mpParameterMinLineEdits.at(p)->text().toDouble();
        minMax.second = mpParameterMaxLineEdits.at(p)->text().toDouble();
        parLimits.append(minMax);
    }

    int nParallelModels = nThreads;
    mpEvaluator = new SensitivityAnalysisEvaluator(this, mpModel, parLimits, mSelectedComponents, mSelectedParameters, mOutputVariables, nParallelModels);
    if (mpUseRemoteSimulatorsCheckBox->isChecked())
    {
        mpEvaluator->mUseRemoteSimulators = true;
        mpEvaluator->mNumParallelModels = mpNumRemoteParallelModelsSpinBox->value();
        nParallelModels = mpNumRemoteParallelModelsSpinBox->value();
    }
    SensitivityAnalysisMessageHandler *pOptMessageHandler = new SensitivityAnalysisMessageHandler(this);
    connect(mpAbortButton, SIGNAL(clicked(bool)), pOptMessageHandler, SLOT(abort()));
    mpWorker = new Ops::WorkerParameterSweep(mpEvaluator, pOptMessageHandler);

    mpWorker->setNumberOfCandidates(nParallelModels);
    mpWorker->setNumberOfPoints(nParallelModels);
    mpWorker->setNumberOfParameters(nParameters);
    mpWorker->setMaxNumberOfIterations(nSteps/nParallelModels);
    mNumIterations = nSteps/nThreads;   //Used by progress bar
    for(int p=0; p<parLimits.size(); ++p)
    {
        mpWorker->setParameterLimits(p, parLimits[p].first, parLimits[p].second);
    }

    //connect(mpWorker, SIGNAL(stepCompleted(int)), this, SLOT(updateProgressBar(int)));

    bool rc = mpEvaluator->init();
    if (rc)
    {
        mpWorker->initialize();
        mpWorker->run();
    }
    mpEvaluator->finalize();
}

void SensitivityAnalysisDialog::abort()
{
    mAborted = true;
}

void SensitivityAnalysisDialog::updateProgressBar(int i)
{
    mpProgressBar->setValue(100*i/mpWorker->getMaxNumberOfIterations());
}

void SensitivityAnalysisEvaluator::plot()
{
    mpDialog->updateProgressBar(mpWorker->getMaxNumberOfIterations());   //Just to make it look better
    //    gpConfig->setBoolSetting(cfg::progressbar, progressBarOrgSetting);

    if (!mModelPtrs.empty())
    {
        for(int v=0; v<mOutputVars.size(); ++v)
        {
            int nGenerations = mModelPtrs.first()->getTopLevelSystemContainer()->getLogDataHandler()->getCurrentGenerationNumber()+1;
            SharedVectorVariableT pCurrentGenTimeData = mModelPtrs.first()->getTopLevelSystemContainer()->getLogDataHandler()->getTimeVectorVariable(nGenerations-1);
            int nSamples = 0;
            if (pCurrentGenTimeData)
            {
                nSamples = pCurrentGenTimeData->getDataSize();
            }
            else
            {
                continue;
            }

            QVector<double> vMin(nSamples, 100000000000.0);
            QVector<double> vMax(nSamples, -100000000000.0);
            double totalMin=100000000000.0;
            double totalMax=-100000000000.0;

            QString component = mOutputVars.at(v).at(0);
            QString port = mOutputVars.at(v).at(1);
            QString variable = mOutputVars.at(v).at(2);

            //! @todo there is no subsystem support here right now
            QString fullName = makeFullVariableName(QStringList(), component,port,variable);

            //PlotWindow *pPlotWindow = mModelPtrs.first()->getViewContainerObject()->getModelObject(component)->getPort(port)->plot(variable, QString(), QColor("Blue"));

            for(int m=0; m<mModelPtrs.size(); ++m)
            {
                for(int g=nGenerations-mpWorker->getMaxNumberOfIterations(); g<nGenerations; ++g)
                {
                    SharedVectorVariableT pVector = mModelPtrs[m]->getTopLevelSystemContainer()->getLogDataHandler()->getVectorVariable(fullName, g);
                    if(!pVector.isNull())
                    {
                        QVector<double> temp = pVector->getDataVectorCopy();
                        for(int i=0; i<temp.size(); ++i)
                        {
                            if(temp[i] > vMax[i]) vMax[i] = temp[i];
                            if(temp[i] < vMin[i]) vMin[i] = temp[i];
                            if(temp[i] > totalMax) totalMax = temp[i];
                            if(temp[i] < totalMin) totalMin = temp[i];
                        }
                    }
                }
            }

            //Commented out code = add curve for max and min
            SharedVariableDescriptionT minDesc(new VariableDescription);
            minDesc.data()->mAliasName = "min("+fullName+")";
            SharedVectorVariableT pMinData(new TimeDomainVariable(pCurrentGenTimeData, vMin, -1, minDesc, SharedMultiDataVectorCacheT(0)));
            SharedVariableDescriptionT maxDesc = SharedVariableDescriptionT(new VariableDescription);
            maxDesc.data()->mAliasName = "max("+fullName+")";
            SharedVectorVariableT pMaxData(new TimeDomainVariable(pCurrentGenTimeData, vMax, -1, maxDesc, SharedMultiDataVectorCacheT(0)));

            PlotWindow *pPlotWindow = gpPlotHandler->createNewUniquePlotWindow("Sensitivity Analysis");
            gpPlotHandler->plotDataToWindow(pPlotWindow, pMaxData, QwtPlot::yLeft);
            pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineColor(QColor(0,0,255,200));
            gpPlotHandler->plotDataToWindow(pPlotWindow, pMinData, QwtPlot::yLeft);
            pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineColor(QColor(0,0,255,200));
            pPlotWindow->hidePlotCurveControls();
            pPlotWindow->setLegendsVisible(false);

            //! @todo Implement interval curve type support in plot window instead!
            //! @note This is not compatible with most plot functions
            QwtPlotIntervalCurve *pCurve = new QwtPlotIntervalCurve();
            pCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
            QVector<QwtIntervalSample> data;
            QVector<double> time = pCurrentGenTimeData->getDataVectorCopy();
            for(int i=0; i<vMin.size(); ++i)
            {
                data.append(QwtIntervalSample(time[i], vMin[i], vMax[i]));
            }
            pCurve->setSamples(data);
            pCurve->setPen(QColor(0,0,255,150), 1.0);
            pCurve->setBrush(QColor(0,0,255,150));

            pCurve->attach(static_cast<QwtPlot*>(pPlotWindow->getPlotTabWidget()->getCurrentTab()->getQwtPlot()));
        }
    }
}



SensitivityAnalysisEvaluator::SensitivityAnalysisEvaluator(SensitivityAnalysisDialog *pDialog, ModelWidget *pModel, QVector<QPair<double, double> > parLimits, QStringList &parComps, QStringList &pars, QList<QStringList> &outputVars, int nThreads)
    : Ops::Evaluator()
{
    mpDialog = pDialog;
    mLimits = parLimits;
    mpModel = pModel;
    mPars = pars;
    mParComps = parComps;
    mOutputVars = outputVars;
    mNumParallelModels = nThreads;
}



bool SensitivityAnalysisEvaluator::init()
{
    // Save hidden copy of model to load multiple copies of and run sensitivity analysis against
    QString name = mpModel->getTopLevelSystemContainer()->getName();
    QString appearanceDataBasePath = mpModel->getTopLevelSystemContainer()->getAppearanceData()->getBasePath();
    QDir().mkpath(gpDesktopHandler->getDataPath()+"sensitivity/");
    QString savePath = gpDesktopHandler->getDataPath()+"sensitivity/"+name+".hmf";
    mpModel->saveTo(savePath);
    mpModel->getTopLevelSystemContainer()->setAppearanceDataBasePath(appearanceDataBasePath);

    // Load correct number of models depending on number of cores
    mModelPtrs.clear();
    for(int i=0; i<mNumParallelModels; ++i)
    {
        mModelPtrs.append(gpModelHandler->loadModel(savePath, ModelHandler::IgnoreAlreadyOpen | ModelHandler::Detatched));
    }

    // Add base path from original model as search path, for components that load files with relative paths
    for(int m=0; m<mModelPtrs.size(); ++m)
    {
        mModelPtrs.at(m)->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->addSearchPath(appearanceDataBasePath);
    }

#ifdef USEZMQ
    removeRemoteSimulationQueueHandler(mpRemoteSimulationQueueHandler);
    if (mUseRemoteSimulators)
    {
        mpRemoteSimulationQueueHandler = createRemoteSimulationQueueHandler(Basic);

        // We assume that we never get more then 2:1 speedup from threads, so it is best to run single core simulation
        // and as many parallel models as possible
        mpRemoteSimulationQueueHandler->setupModelQueues(mModelPtrs, 1);
        if (!mpRemoteSimulationQueueHandler->hasQueues())
        {
            gpMessageHandler->addErrorMessage("Could not setup remote queues");
            return false;
        }
    }
#endif
    return true;
}

void SensitivityAnalysisEvaluator::finalize()
{
#ifdef USEZMQ
    // Clear and disconnect from parallel server queues
    if (mUseRemoteSimulators)
    {
        mpRemoteSimulationQueueHandler->clear();
    }
    removeRemoteSimulationQueueHandler(mpRemoteSimulationQueueHandler);
#endif

    plot();

    // Clear all models
    for(int m=0; m<mModelPtrs.size(); ++m)
    {
        //gpModelHandler->closeModel(mModelPtrs[m], true);
        delete mModelPtrs[m];
        //! @todo we delete her since the models are detached, but it would be better if we could use the closeModel function and let the handler close and delete the models
    }
    mModelPtrs.clear();
}


void SensitivityAnalysisEvaluator::setParameters(std::vector<std::vector<double> > *pPoints)
{
    int nParameters = mLimits.size();

    for(int m=0; m<mModelPtrs.size(); ++m)
    {
        for(int p=0; p<nParameters; ++p)
        {
            if(mParComps.at(p).isEmpty())
            {
                mModelPtrs[m]->getTopLevelSystemContainer()->setParameterValue(mPars.at(p), QString().setNum(pPoints->at(m).at(p)));
            }
            else
            {
                mModelPtrs[m]->getTopLevelSystemContainer()->getModelObject(mParComps.at(p))->setParameterValue(mPars.at(p), QString().setNum(pPoints->at(m).at(p)));
            }
        }
    }
}


void SensitivityAnalysisEvaluator::evaluateAllPoints()
{
    setParameters(&mpWorker->getPoints());

#ifdef USEZMQ
    if (mUseRemoteSimulators)
    {
        if (mpRemoteSimulationQueueHandler && mpRemoteSimulationQueueHandler->hasServers())
        {
            //! @todo this simulation does not care about numpoint vs numcandidates
            mpRemoteSimulationQueueHandler->simulateModels();
        }
        //! @todo need error message here if something goes wrong
    }
    else
    {
#endif
        // Note! The "mid()" function are used to make sure that the number of models simulated equals number of candidates (in case more models are opened)
        gpModelHandler->simulateMultipleModels_blocking(mModelPtrs.mid(0,mpWorker->getNumberOfPoints()));
#ifdef USEZMQ
    }
#endif
}

void SensitivityAnalysisEvaluator::evaluateAllCandidates()
{
    setParameters(&mpWorker->getCandidatePoints());

#ifdef USEZMQ
    if (mUseRemoteSimulators)
    {
        if (mpRemoteSimulationQueueHandler && mpRemoteSimulationQueueHandler->hasServers())
        {
            //! @todo this simulation does not care about numpoint vs numcandidates
            mpRemoteSimulationQueueHandler->simulateModels();
        }
        //! @todo need error message here if something goes wrong
    }
    else
    {
#endif
        // Note! The "mid()" function are used to make sure that the number of models simulated equals number of candidates (in case more models are opened)
        gpModelHandler->simulateMultipleModels_blocking(mModelPtrs.mid(0,mpWorker->getNumberOfCandidates()));
#ifdef USEZMQ
    }
#endif
}

SensitivityAnalysisMessageHandler::SensitivityAnalysisMessageHandler(SensitivityAnalysisDialog *pDialog)
{
    mpDialog = pDialog;
}

void SensitivityAnalysisMessageHandler::stepCompleted(size_t step)
{
    mpDialog->updateProgressBar(step);
}

void SensitivityAnalysisMessageHandler::abort()
{
    mIsAborted = true;
}
