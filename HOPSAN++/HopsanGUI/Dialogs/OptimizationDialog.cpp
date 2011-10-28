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

OptimizationDialog::OptimizationDialog(MainWindow *parent)
    : QDialog(parent)
{
        //Set the name and size of the main window
    this->resize(900,480);
    this->setWindowTitle("Optimization");
    this->setPalette(gConfig.getPalette());

    //Settings tab
    mpIterationsLabel = new QLabel("Number of iterations:");
    mpIterationsSpinBox = new QSpinBox(this);
    mpIterationsSpinBox->setRange(0, std::numeric_limits<int>::max());
    mpIterationsSpinBox->setValue(100);
    mpSearchPointsLabel = new QLabel("Number of search points:" );
    mpSearchPointsSpinBox = new QSpinBox(this);
    mpSearchPointsSpinBox->setRange(1, std::numeric_limits<int>::max());
    mpSearchPointsSpinBox->setValue(8);
    mpAlphaLabel = new QLabel("Reflection coefficient: ");
    mpAlphaSpinBox = new QDoubleSpinBox(this);
    mpAlphaSpinBox->setRange(0, std::numeric_limits<double>::max());
    mpAlphaSpinBox->setSingleStep(0.1);
    mpAlphaSpinBox->setValue(1.3);
    mpBetaLabel = new QLabel("Randomization factor: ");
    mpBetaSpinBox = new QDoubleSpinBox(this);
    mpBetaSpinBox->setRange(0, std::numeric_limits<double>::max());
    mpBetaSpinBox->setSingleStep(0.1);
    mpBetaSpinBox->setValue(0.3);
    mpGammaLabel = new QLabel("Forgetting factor: ");
    mpGammaSpinBox = new QDoubleSpinBox(this);
    mpGammaSpinBox->setRange(0, std::numeric_limits<double>::max());
    mpGammaSpinBox->setSingleStep(0.1);
    mpGammaSpinBox->setValue(0.5);
    mpPlottingCheckBox = new QCheckBox("Plot each iteration", this);
    mpPlottingCheckBox->setChecked(true);
    mpSettingsLayout = new QGridLayout(this);
    mpSettingsLayout->addWidget(mpIterationsLabel, 0, 0);
    mpSettingsLayout->addWidget(mpIterationsSpinBox, 0, 1);
    mpSettingsLayout->addWidget(mpSearchPointsLabel, 1, 0);
    mpSettingsLayout->addWidget(mpSearchPointsSpinBox, 1, 1);
    mpSettingsLayout->addWidget(mpAlphaLabel, 2, 0);
    mpSettingsLayout->addWidget(mpAlphaSpinBox, 2, 1);
    mpSettingsLayout->addWidget(mpBetaLabel, 3, 0);
    mpSettingsLayout->addWidget(mpBetaSpinBox, 3, 1);
    mpSettingsLayout->addWidget(mpGammaLabel, 4, 0);
    mpSettingsLayout->addWidget(mpGammaSpinBox, 4, 1);
    mpSettingsLayout->addWidget(mpPlottingCheckBox, 5, 0, 1, 2);
    mpSettingsLayout->addWidget(new QWidget(this), 6, 0, 1, 2);
    mpSettingsLayout->setRowStretch(0, 0);
    mpSettingsLayout->setRowStretch(1, 0);
    mpSettingsLayout->setRowStretch(2, 0);
    mpSettingsLayout->setRowStretch(3, 0);
    mpSettingsLayout->setRowStretch(4, 0);
    mpSettingsLayout->setRowStretch(4, 0);
    mpSettingsLayout->setRowStretch(6, 1);
    mpSettingsWidget = new QWidget(this);
    mpSettingsWidget->setLayout(mpSettingsLayout);

    //Parameter tab
    mpParametersList = new QTreeWidget(this);
    mpParametersLayout = new QGridLayout(this);
    mpParametersLayout->addWidget(mpParametersList, 0, 0, 1, 3);
    mpParametersWidget = new QWidget(this);
    mpParametersWidget->setLayout(mpParametersLayout);

    //Objective function tab
    mFunctions << "Minimize highest value" << "Minimize lowest value" << "Maximize highest value" << "Maximize lowest value" << "Overshoot over value" << "First time to value" << "Absolute difference from value at time";
    mpVariablesList = new QTreeWidget(this);
    mpFunctionsComboBox = new QComboBox(this);
    mpFunctionsComboBox->addItems(mFunctions);
    mpAddFunctionButton = new QPushButton("Add Function");
    QLabel *pWeightLabel = new QLabel("Weight");
    QLabel *pNormLabel = new QLabel("Norm. Factor");
    QLabel *pExpLabel = new QLabel("Exp. Factor");
    QLabel *pDescriptionLabel = new QLabel("Description");
    QLabel *pDataLabel = new QLabel("Data");
    QFont tempFont = pDescriptionLabel->font();
    tempFont.setBold(true);
    pWeightLabel->setFont(tempFont);
    pNormLabel->setFont(tempFont);
    pExpLabel->setFont(tempFont);
    pDescriptionLabel->setFont(tempFont);
    pDataLabel->setFont(tempFont);
    mpObjectiveLayout = new QGridLayout(this);
    mpObjectiveLayout->addWidget(mpVariablesList,           0, 0, 1, 7);
    mpObjectiveLayout->addWidget(mpFunctionsComboBox,       1, 0, 1, 4);
    mpObjectiveLayout->addWidget(mpAddFunctionButton,       1, 5, 1, 3);
    mpObjectiveLayout->addWidget(pWeightLabel,              2, 0, 1, 1);
    mpObjectiveLayout->addWidget(pNormLabel,                2, 1, 1, 1);
    mpObjectiveLayout->addWidget(pExpLabel,                 2, 2, 1, 1);
    mpObjectiveLayout->addWidget(pDescriptionLabel,         2, 3, 1, 2);
    mpObjectiveLayout->addWidget(pDataLabel,                2, 5, 1, 2);
    mpObjectiveLayout->addWidget(new QWidget(this),         3, 0, 1, 7);
    mpObjectiveLayout->setRowStretch(0, 0);
    mpObjectiveLayout->setRowStretch(1, 0);
    mpObjectiveLayout->setRowStretch(2, 0);
    mpObjectiveLayout->setRowStretch(3, 1);
    mpObjectiveLayout->setColumnStretch(0, 0);
    mpObjectiveLayout->setColumnStretch(1, 0);
    mpObjectiveLayout->setColumnStretch(2, 0);
    mpObjectiveLayout->setColumnStretch(3, 0);
    mpObjectiveLayout->setColumnStretch(4, 1);
    mpObjectiveLayout->setColumnStretch(5, 0);
    mpObjectiveLayout->setColumnStretch(6, 0);
    mpObjectiveWidget = new QWidget(this);
    mpObjectiveWidget->setLayout(mpObjectiveLayout);

    //Tab widget
    mpTabWidget = new QTabWidget(this);
    mpTabWidget->addTab(mpSettingsWidget, "Settings");
    mpTabWidget->addTab(mpParametersWidget, "Parameters");
    mpTabWidget->addTab(mpObjectiveWidget, "Objective Function");

    //Output box
    mpOutputBox = new QTextEdit(this);
    tempFont = mpOutputBox->font();
    tempFont.setFamily("Courier");
    mpOutputBox->setFont(tempFont);
    mpOutputBox->setMinimumWidth(450);

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
    pLayout->addWidget(mpTabWidget, 1, 1, 1, 1);
    pLayout->addWidget(mpOutputBox, 1, 3, 1, 1);
    pLayout->addWidget(mpButtonBox, 2, 1, 1, 3);
    setLayout(pLayout);

    //Connections
    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpGenerateButton,               SIGNAL(clicked()),      this,                   SLOT(updateOutputBox()));
    connect(mpRunButton,                    SIGNAL(clicked()),      this,                   SLOT(run()));
    connect(mpAddFunctionButton,            SIGNAL(clicked()),      this,                   SLOT(addFunction()));
}


void OptimizationDialog::open()
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


    mScript.clear();
    mpOutputBox->clear();
    mpRunButton->setDisabled(true);

    QDialog::show();
}


void OptimizationDialog::generateScriptFile()
{
    //! @todo Finish

    if(mSelectedParameters.isEmpty())
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("No parameters specified for optimization.");
        return;
    }

    if(mFunctionVariables.isEmpty())
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("No objective functions specified for optimization.");
    }

    QString iterationsString = QString().setNum(mpIterationsSpinBox->value());
    QString alphaString = QString().setNum(mpAlphaSpinBox->value());
    QString betaString = QString().setNum(mpBetaSpinBox->value());
    QString gammaString = QString().setNum(mpGammaSpinBox->value());
    QString nParString = QString().setNum(mpParameterLabels.size());
    mScript.clear();

    QTextStream scriptStream(&mScript);

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
    scriptStream << "from HopsanOptimization import *\n";
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
    scriptStream << "minValues = ["+QString().setNum(mpParameterMinSpinBoxes.at(0)->value());
    for(int i=1; i<mSelectedParameters.size(); ++i)
        scriptStream << ", "+QString().setNum(mpParameterMinSpinBoxes.at(i)->value());
    scriptStream << "]                    #Minimum value for each parameter\n";
    scriptStream << "maxValues = ["+QString().setNum(mpParameterMaxSpinBoxes.at(0)->value());
    for(int i=1; i<mSelectedParameters.size(); ++i)
        scriptStream << ", "+QString().setNum(mpParameterMaxSpinBoxes.at(i)->value());
    scriptStream << "]                    #Maximum value for each parameter\n";
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
    for(int i=0; i<mWeightSpinBoxPtrs.size(); ++i)
        scriptStream << "  w"+QString().setNum(i)+"="+QString().setNum(mWeightSpinBoxPtrs.at(i)->value())+"\n";
    for(int i=0; i<mNormSpinBoxPtrs.size(); ++i)
        scriptStream << "  n"+QString().setNum(i)+"="+QString().setNum(mNormSpinBoxPtrs.at(i)->value())+"\n";
    for(int i=0; i<mExpSpinBoxPtrs.size(); ++i)
        scriptStream << "  g"+QString().setNum(i)+"="+QString().setNum(mExpSpinBoxPtrs.at(i)->value())+"\n";

    scriptStream << "  time=hopsan.component(\""+mFunctionComponents.first().first()+"\").port(\""+mFunctionPorts.first().first()+"\").getTimeVector()\n";
    for(int i=0; i<mFunctionVariables.size(); ++i)
        for(int j=0; j<mFunctionVariables.at(i).size(); ++j)
            scriptStream << "  data"+QString().setNum(i)+QString().setNum(j)+"=hopsan.component(\""+mFunctionComponents.at(i).at(j)+"\").port(\""+mFunctionPorts.at(i).at(j)+"\").getDataVector(\""+mFunctionVariables.at(i).at(j)+"\")\n";
    scriptStream << "  return ";
    for(int i=0; i<mFunctionVariables.size(); ++i)
        scriptStream << getFunctionCode(i);
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
    scriptStream << "for i in range(len(parameters)):\n";
    scriptStream << "  for j in range(len(parameterNames)):\n";
    scriptStream << "    hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[i][j])\n";
    scriptStream << "  hopsan.simulate()\n";
    scriptStream << "  obj[i] = getObjective()\n";
    if(mpPlottingCheckBox->isChecked())
    {
        scriptStream << "#Run one simulation first and open plot window, if user wants to see plots in real-time\n";
        for(int i=0; i<mFunctionVariables.size(); ++i)
            for(int j=0; j<mFunctionVariables.at(i).size(); ++j)
                scriptStream << "hopsan.plot(\""+mFunctionComponents.at(i).at(j)+"\",\""+mFunctionPorts.at(i).at(j)+"\",\""+mFunctionVariables.at(i).at(j)+"\")\n";
        scriptStream << "\n";
    }
    scriptStream << "worstId = indexOfMax(obj)\n";
    scriptStream << "previousWorstId = -1\n";
    scriptStream << "\n";
    scriptStream << "for k in range(iterations):\n";
    scriptStream << "  kf=1-(alpha/2)**(gamma/(2*"+nParString+"))\n";
    scriptStream << "  objspread=max(obj)-min(obj)\n";
    scriptStream << "  for i in range(len(parameters)):\n";
    scriptStream << "    obj[i] = obj[i] + objspread*kf\n";
    scriptStream << "  for j in range(len(parameterNames)):\n";
    scriptStream << "    hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[worstId][j])\n";
    scriptStream << "  hopsan.simulate()\n";
    scriptStream << "  obj[worstId] = getObjective()\n";
    scriptStream << "  worstId = indexOfMax(obj)\n";
    scriptStream << "  if worstId == previousWorstId:\n";
    scriptStream << "    reflectWorst(parameters,worstId,alpha/2.0,minValues,maxValues,beta)  #Same as previous, move halfway to centroid\n";
    scriptStream << "  else:\n";
    scriptStream << "    reflectWorst(parameters,worstId,alpha,minValues,maxValues,beta)      #Reflect worst through centroid of the remaining points\n";
    scriptStream << "  previousWorstId=worstId\n";
    scriptStream << "  \n";
    if(!mpPlottingCheckBox->isChecked())
    {
        scriptStream << "#Plot when simulation is finished\n";
        for(int i=0; i<mFunctionVariables.size(); ++i)
            for(int j=0; j<mFunctionVariables.at(i).size(); ++j)
                scriptStream << "hopsan.plot(\""+mFunctionComponents.at(i).at(j)+"\",\""+mFunctionPorts.at(i).at(j)+"\",\""+mFunctionVariables.at(i).at(j)+"\")\n";
        scriptStream << "\n";
    }
}


//! @brief Adds a new parameter to the list of selected parameter, and displays it in dialog
//! @param item Tree widget item which represents parameter
//! @param i Not used
void OptimizationDialog::updateChosenParameters(QTreeWidgetItem* item, int i)
{
    if(item->checkState(0) == Qt::Checked)
    {
        mSelectedComponents.append(item->parent()->text(0));
        mSelectedParameters.append(item->text(0));
        QLabel *pLabel = new QLabel(trUtf8(" ≤  ") + item->parent()->text(0) + ", " + item->text(0) + trUtf8("  ≤ "));
        pLabel->setAlignment(Qt::AlignCenter);
        QDoubleSpinBox *pMinSpinBox = new QDoubleSpinBox(this);
        pMinSpinBox->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
        pMinSpinBox->setValue(0.0);
        QDoubleSpinBox *pMaxSpinBox = new QDoubleSpinBox(this);
        pMaxSpinBox->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
        pMaxSpinBox->setValue(1.0);
        mpParameterLabels.append(pLabel);
        mpParameterMinSpinBoxes.append(pMinSpinBox);
        mpParameterMaxSpinBoxes.append(pMaxSpinBox);

        int row = mpParametersLayout->rowCount();
        mpParametersLayout->addWidget(pMinSpinBox, row, 0);
        mpParametersLayout->addWidget(pLabel, row, 1);
        mpParametersLayout->addWidget(pMaxSpinBox, row, 2);
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
        mpParametersLayout->removeWidget(mpParameterMinSpinBoxes.at(i));
        mpParametersLayout->removeWidget(mpParameterMaxSpinBoxes.at(i));
        QLabel *pParameterLabel = mpParameterLabels.at(i);
        QDoubleSpinBox *pParameterMinSpinBox = mpParameterMinSpinBoxes.at(i);
        QDoubleSpinBox *pParameterMaxSpinBoxes = mpParameterMaxSpinBoxes.at(i);
        mpParameterLabels.removeAt(i);
        mpParameterMinSpinBoxes.removeAt(i);
        mpParameterMaxSpinBoxes.removeAt(i);
        mSelectedParameters.removeAt(i);
        mSelectedComponents.removeAt(i);
        delete(pParameterLabel);
        delete(pParameterMinSpinBox);
        delete(pParameterMaxSpinBoxes);
    }
}


//! @brief Adds a new variable to the list of selected variables
//! @param item Tree widget item which represents variable
//! @param i Not used
void OptimizationDialog::updateChosenVariables(QTreeWidgetItem *item, int i)
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
    int i=mpFunctionsComboBox->currentIndex();

    if(!verifyFunctionVariables(i))
        return;

    QStringList data;
    data = getFunctionDataList(i);

    mSelectedFunctions.append(i);
    mFunctionComponents.append(QStringList());
    mFunctionPorts.append(QStringList());
    mFunctionVariables.append(QStringList());
    for(int i=0; i<mSelectedVariables.size(); ++i)
    {
        mFunctionComponents.last().append(mSelectedVariables.at(i).at(0));
        mFunctionPorts.last().append(mSelectedVariables.at(i).at(1));
        mFunctionVariables.last().append(mSelectedVariables.at(i).at(2));
    }

    QDoubleSpinBox *pWeightSpinBox = new QDoubleSpinBox(this);
    pWeightSpinBox->setRange(0.0,1000000.0);
    pWeightSpinBox->setSingleStep(0.1);
    pWeightSpinBox->setValue(1.0);
    QDoubleSpinBox *pNormSpinBox = new QDoubleSpinBox(this);
    pNormSpinBox->setRange(0.0,1000000.0);
    pNormSpinBox->setSingleStep(0.1);
    pNormSpinBox->setValue(1.0);
    QDoubleSpinBox *pExpSpinBox = new QDoubleSpinBox(this);
    pExpSpinBox->setRange(0.0,1000000.0);
    pExpSpinBox->setSingleStep(0.1);
    pExpSpinBox->setValue(2.0);

    QString variablesText = mFunctionComponents.last().first()+", "+mFunctionPorts.last().first()+", "+mFunctionVariables.last().first();
    for(int i=1; i<mFunctionVariables.last().size(); ++i)
    {
        variablesText.append(" and " + mFunctionComponents.last().at(i)+", "+mFunctionPorts.last().at(i)+", "+mFunctionVariables.last().at(i));
    }
    QLabel *pFunctionLabel = new QLabel(mFunctions.at(i)+" for "+variablesText, this);
    pFunctionLabel->setWordWrap(true);
    QWidget *pDataWidget = new QWidget(this);
    QGridLayout *pDataGrid = new QGridLayout(this);
    pDataWidget->setLayout(pDataGrid);
    QList<QDoubleSpinBox*> dummyList;
    for(int i=0; i<data.size(); ++i)
    {
        QLabel *pDataLabel = new QLabel(data.at(i), this);
        QDoubleSpinBox *pDataSpinBox = new QDoubleSpinBox(this);
        pDataSpinBox->setRange(-1000, 1000);
        pDataSpinBox->setValue(0);
        pDataGrid->addWidget(pDataLabel, i, 0);
        pDataGrid->addWidget(pDataSpinBox, i, 1);
        dummyList.append(pDataSpinBox);
    }
    mDataSpinBoxPtrs.append(dummyList);
    QToolButton *pRemoveButton = new QToolButton(this);
    pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
    pRemoveButton->setToolTip("Remove Function");
    mWeightSpinBoxPtrs.append(pWeightSpinBox);
    mNormSpinBoxPtrs.append(pNormSpinBox);
    mExpSpinBoxPtrs.append(pExpSpinBox);
    mFunctionLabelPtrs.append(pFunctionLabel);
    mDataWidgetPtrs.append(pDataWidget);
    mRemoveFunctionButtonPtrs.append(pRemoveButton);

    int row = mpObjectiveLayout->rowCount()-1;
    mpObjectiveLayout->addWidget(pWeightSpinBox, row,   0, 1, 1);
    mpObjectiveLayout->addWidget(pNormSpinBox, row,     1, 1, 1);
    mpObjectiveLayout->addWidget(pExpSpinBox, row,      2, 1, 1);
    mpObjectiveLayout->addWidget(pFunctionLabel, row,   3, 1, 2);
    mpObjectiveLayout->addWidget(pDataWidget, row,      5, 1, 1);
    mpObjectiveLayout->addWidget(pRemoveButton, row,    6, 1, 1);
    mpObjectiveLayout->setRowStretch(row, 0);
    mpObjectiveLayout->setRowStretch(row+1, 1);
    mpObjectiveLayout->setColumnStretch(0, 0);
    mpObjectiveLayout->setColumnStretch(1, 0);
    mpObjectiveLayout->setColumnStretch(2, 1);
    mpObjectiveLayout->setColumnStretch(3, 0);
    mpObjectiveLayout->setColumnStretch(4, 0);

    connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeFunction()));
}


void OptimizationDialog::removeFunction()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mRemoveFunctionButtonPtrs.indexOf(button);

    mpObjectiveLayout->removeWidget(mWeightSpinBoxPtrs.at(i));
    mpObjectiveLayout->removeWidget(mNormSpinBoxPtrs.at(i));
    mpObjectiveLayout->removeWidget(mExpSpinBoxPtrs.at(i));
    mpObjectiveLayout->removeWidget(mFunctionLabelPtrs.at(i));
    mpObjectiveLayout->removeWidget(mRemoveFunctionButtonPtrs.at(i));
    mpObjectiveLayout->removeWidget(mDataWidgetPtrs.at(i));

    for(int j=0; j<mDataSpinBoxPtrs.at(i).size(); ++j)
    {
        delete(mDataSpinBoxPtrs.at(i).at(j));
    }
    delete(mWeightSpinBoxPtrs.at(i));
    delete(mNormSpinBoxPtrs.at(i));
    delete(mExpSpinBoxPtrs.at(i));
    delete(mFunctionLabelPtrs.at(i));
    delete(mRemoveFunctionButtonPtrs.at(i));
    delete(mDataWidgetPtrs.at(i));

    mWeightSpinBoxPtrs.removeAt(i);
    mNormSpinBoxPtrs.removeAt(i);
    mExpSpinBoxPtrs.removeAt(i);
    mFunctionLabelPtrs.removeAt(i);
    mRemoveFunctionButtonPtrs.removeAt(i);
    mDataSpinBoxPtrs.removeAt(i);
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
}


//! @brief Saves the generated script code to file and executes the script
void OptimizationDialog::run()
{
    QString dateString = QDateTime::currentDateTime().toString();
    dateString.replace(":", "_");
    dateString.replace(".", "_");
    dateString.replace(" ", "_");
    QString pyPath = gExecPath+QString(SCRIPTPATH)+"OptimizationScript_"+dateString+".py";
    QFile pyFile(pyPath);
    if (!pyFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + pyPath);
        return;
    }
    QTextStream pyStream(&pyFile);
    //pyStream << mScript;
    pyStream << mpOutputBox->toPlainText();
    pyFile.close();

    gpMainWindow->mpPyDockWidget->runPyScript(pyPath);
    close();
}


bool OptimizationDialog::verifyNumberOfVariables(int n)
{
    if(mSelectedVariables.size() > n)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Too many variables selected for this function.");
        return false;
    }
    else if(mSelectedVariables.size() < n)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Too few variables selected for this function.");
        return false;
    }
    return true;
}


QString OptimizationDialog::getFunctionCode(int i)
{
    QString retval;
    switch(mSelectedFunctions.at(i))
    {
    case 0:
        retval = "-w"+QString().setNum(i)+"*(maxValue(data"+QString().setNum(i)+"0)/n"+QString().setNum(i)+")**g"+QString().setNum(i);
        break;
    case 1:
        retval = "-w"+QString().setNum(i)+"*(maxValue(data"+QString().setNum(i)+"0)/n"+QString().setNum(i)+")**g"+QString().setNum(i);
        break;
    case 2:
        retval = "+w"+QString().setNum(i)+"*(maxValue(data"+QString().setNum(i)+"0)/n"+QString().setNum(i)+")**g"+QString().setNum(i);
        break;
    case 3:
        retval = "+w"+QString().setNum(i)+"*(minValue(data"+QString().setNum(i)+"0)/n"+QString().setNum(i)+")**g"+QString().setNum(i);
        break;
    case 4:
        retval = "+w"+QString().setNum(i)+"*(overShoot(data"+QString().setNum(i)+"0, "+QString().setNum(mDataSpinBoxPtrs.at(i).first()->value())+")/n"+QString().setNum(i)+")**g"+QString().setNum(i);
        break;
    case 5:
        retval = "+w"+QString().setNum(i)+"*(firstTimeAt(data"+QString().setNum(i)+"0, time, "+QString().setNum(mDataSpinBoxPtrs.at(i).first()->value())+")/n"+QString().setNum(i)+")**g"+QString().setNum(i);
        break;
    case 6:
        retval = "+w"+QString().setNum(i)+"*(diffFromValueAtTime(data00, time, "+QString().setNum(mDataSpinBoxPtrs.at(i).first()->value())+","+QString().setNum(mDataSpinBoxPtrs.at(i).at(1)->value())+")/n"+QString().setNum(i)+")**g"+QString().setNum(i);
        break;
    }
    return retval;
}


QStringList OptimizationDialog::getFunctionDataList(int i)
{
    QStringList data;
    switch(i)
    {
    case 4:
        data << "x";
        break;
    case 5:
        data << "x";
        break;
    case 6:
        data << "x" << "t";
        break;
    }
    return data;
}



bool OptimizationDialog::verifyFunctionVariables(int i)
{
    switch(i)
    {
    case 0:
        return(verifyNumberOfVariables(1));
    case 1:
        return(verifyNumberOfVariables(1));
    case 2:
        return(verifyNumberOfVariables(1));
    case 3:
        return(verifyNumberOfVariables(1));
    case 4:
        return(verifyNumberOfVariables(1));
    case 5:
        return(verifyNumberOfVariables(1));
    case 6:
        return(verifyNumberOfVariables(1));
    }
}
