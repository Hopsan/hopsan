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

#include "global.h"
#include "Configuration.h"
#include "GUIPort.h"
#include "PlotWindow.h"
#include "Dialogs/SensitivityAnalysisDialog.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/ModelWidget.h"
#include "ModelHandler.h"
#include "MainWindow.h"

#ifndef WIN32
#include <unistd.h> //Needed for sysctl
#endif


//! @brief Constructor



//! @brief Reimplementation of open() slot, used to initialize the dialog
SensitivityAnalysisDialog::SensitivityAnalysisDialog(QWidget *parent)
    : QDialog(parent)
{
    //Set the name and size of the main window
    this->resize(640,640);
    this->setWindowTitle("Sensitivity Analysis");
    this->setPalette(gpConfig->getPalette());

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

    QGroupBox *pSettingsGroupBox = new QGroupBox();
    QVBoxLayout *pSettingsLayout = new QVBoxLayout();
    pSettingsLayout->addLayout(pDistributionLayout);
    pSettingsLayout->addLayout(pStepsLayout);
    pSettingsGroupBox->setLayout(pSettingsLayout);

    //Buttons
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    pCancelButton->setAutoDefault(false);
    QPushButton *pRunButton = new QPushButton(tr("&Start Analysis"), this);
    pRunButton->setDefault(true);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pRunButton, QDialogButtonBox::ActionRole);

    //Toolbar
    QAction *pHelpAction = new QAction("Show Context Help", this);
    pHelpAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Help.png"));
    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pHelpAction);


    //Main layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(pParametersGroupBox,   0, 0, 1, 2);
    pLayout->addWidget(pOutputGroupBox,       1, 0, 1, 2);
    pLayout->addWidget(pSettingsGroupBox,     2, 0, 1, 2);
    pLayout->addWidget(pButtonBox,            4, 1, 1, 1);
    pLayout->addWidget(pToolBar,              4, 0, 1, 1);
    setLayout(pLayout);

    //Connections
    connect(pCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(pRunButton,                    SIGNAL(clicked()),      this,                   SLOT(run()));
    connect(pHelpAction,                   SIGNAL(triggered()),    gpMainWindow,           SLOT(openContextHelp()));
    connect(mpNormalDistributionRadioButton, SIGNAL(toggled(bool)), pParameterAverageLabel, SLOT(setVisible(bool)));
    connect(mpNormalDistributionRadioButton, SIGNAL(toggled(bool)), pParameterSigmaLabel, SLOT(setVisible(bool)));
    connect(mpUniformDistributionRadioButton, SIGNAL(toggled(bool)), pParameterMinLabel, SLOT(setVisible(bool)));
    connect(mpUniformDistributionRadioButton, SIGNAL(toggled(bool)), pParameterMaxLabel, SLOT(setVisible(bool)));
}

void SensitivityAnalysisDialog::open()
{
    mpParametersList->clear();
    SystemContainer *pSystem = gpModelHandler->getCurrentTopLevelSystem();
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
    mpParametersList->sortItems(0, Qt::AscendingOrder);
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
    mpParameterAverageLineEdits.clear();
    mpParameterLabels.clear();
    mpParameterMaxLineEdits.clear();
    mpParameterMinLineEdits.clear();
    mpParameterSigmaLineEdits.clear();

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
        QString averageValue = gpModelHandler->getCurrentViewContainerObject()->getModelObject(item->parent()->text(0))->getParameterValue(item->text(0));
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
    DistributionEnumT type;
    if(mpUniformDistributionRadioButton->isChecked())
    {
        type = UniformDistribution;
    }
    else
    {
        type = NormalDistribution;
    }


    int nThreads = gpConfig->getNumberOfThreads();
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

    if(gpConfig->getUseMulticore())
    {
        //Close all other containers
        for(int i=0; i<gpModelHandler->count(); ++i)
        {
            if(gpModelHandler->getViewContainerObject(i) != gpModelHandler->getCurrentViewContainerObject())
            {
                gpModelHandler->closeModel(i);
                --i;
            }
        }

        //Load more tabs with same model
        for(int i=1; i<nThreads; ++i)
        {
            gpModelHandler->loadModel(gpModelHandler->getCurrentViewContainerObject()->getModelFileInfo().absoluteFilePath(), true);
        }
    }
    else
    {
        nThreads = 1;
    }

    int nTabs = gpModelHandler->count();

    if(gpConfig->getUseMulticore())
    {
        bool noChange=false;
        for(int i=0; i<nSteps/nThreads; ++i)
        {
            for(int t=0; t<nTabs; ++t)
            {
                for(int p=0; p<nParameteres; ++p)
                {
                    double randPar;
                    if(type == UniformDistribution)
                    {
                        double min = mpParameterMinLineEdits.at(p)->text().toDouble();
                        double max = mpParameterMaxLineEdits.at(p)->text().toDouble();
                        randPar = uniformDistribution(min, max);
                    }
                    else
                    {
                        randPar = normalDistribution(mpParameterAverageLineEdits.at(p)->text().toDouble(), mpParameterSigmaLineEdits.at(p)->text().toDouble());
                    }
                    gpModelHandler->getViewContainerObject(t)->getModelObject(mSelectedComponents.at(p))->setParameterValue(mSelectedParameters.at(p), QString().setNum(randPar));
                }
            }
            gpModelHandler->simulateAllOpenModels_blocking(noChange);
            noChange=true;
        }
    }
    else
    {
        for(int i=0; i<nSteps; ++i)
        {
            for(int p=0; p<nParameteres; ++p)
            {
                double randPar;
                if(type == UniformDistribution)
                {
                    double min = mpParameterMinLineEdits.at(p)->text().toDouble();
                    double max = mpParameterMaxLineEdits.at(p)->text().toDouble();
                    randPar = uniformDistribution(min, max);
                }
                else
                {
                    randPar = normalDistribution(mpParameterAverageLineEdits.at(p)->text().toDouble(), mpParameterSigmaLineEdits.at(p)->text().toDouble());
                }
                gpModelHandler->getCurrentViewContainerObject()->getModelObject(mSelectedComponents.at(p))->setParameterValue(mSelectedParameters.at(p), QString().setNum(randPar));
            }
            gpModelHandler->getCurrentModel()->simulate_blocking();
        }
    }

    if(gpConfig->getUseMulticore())
    {
        for(int v=0; v<mOutputVariables.size(); ++v)
        {
            gpModelHandler->setCurrentModel(0);

            QString component = mOutputVariables.at(v).at(0);
            QString port = mOutputVariables.at(v).at(1);
            QString variable = mOutputVariables.at(v).at(2);

            QString fullName = makeConcatName(component,port,variable);

            PlotWindow *pPlotWindow = gpModelHandler->getViewContainerObject(0)->getModelObject(component)->getPort(port)->plot(variable, QString(), QColor("Blue"));
            pPlotWindow->hidePlotCurveInfo();
            pPlotWindow->setLegendsVisible(false);

            int nGenerations = gpModelHandler->getViewContainerObject(0)->getLogDataHandler()->getLatestGeneration()+1;
            for(int g=nGenerations-nSteps/nThreads; g<nGenerations-1; ++g)
            {
                gpModelHandler->getViewContainerObject(0)->getLogDataHandler()->plotVariable(pPlotWindow, fullName, g, QwtPlot::yLeft, QColor("Blue"));
            }

            //! @todo Why is there two loop bellow why not begin at tab 0 and jsut have one, why nGen-1 on the first one
            for(int t=1; t<nTabs; ++t)
            {
                gpModelHandler->setCurrentModel(t);
                for(int g=nGenerations-nSteps/nThreads; g<nGenerations; ++g)
                {
                    gpModelHandler->getViewContainerObject(t)->getLogDataHandler()->plotVariable(pPlotWindow, fullName, g, QwtPlot::yLeft, QColor("Blue"));
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
            PlotWindow *pPlotWindow = gpModelHandler->getCurrentViewContainerObject()->getModelObject(component)->getPort(port)->plot(variable, QString(), QColor("Blue"));
            pPlotWindow->hidePlotCurveInfo();
            pPlotWindow->setLegendsVisible(false);

            QString fullName = makeConcatName(component,port,variable);
            int nGenerations = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getLatestGeneration()+1;
            for(int g=nGenerations - nSteps; g<nGenerations; ++g)
            {
                gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->plotVariable(pPlotWindow, fullName, g, QwtPlot::yLeft, QColor("Blue"));
            }
        }
    }
}
