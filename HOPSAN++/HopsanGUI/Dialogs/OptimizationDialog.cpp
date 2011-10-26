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
#include "Dialogs/OptimizationDialog.h"
#include "Configuration.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"

class ProjectTabWidget;

OptimizationDialog::OptimizationDialog(MainWindow *parent)
    : QDialog(parent)
{
        //Set the name and size of the main window
    //this->resize(640,480);
    this->setWindowTitle("Optimization");
    this->setPalette(gConfig.getPalette());
    //this->setWindowState();

    mpParametersList = new QTreeWidget(this);

    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    mpGenerateButton = new QPushButton(tr("&Generate Script"), this);
    mpGenerateButton->setDefault(true);
    mpRunButton = new QPushButton(tr("&Run Optimization"), this);
    mpRunButton->setDefault(true);

    mpOutputBox = new QTextEdit(this);
    QFont tempFont = mpOutputBox->font();
    tempFont.setFamily("Courier");
    mpOutputBox->setFont(tempFont);

    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpGenerateButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpRunButton, QDialogButtonBox::ActionRole);

    QGridLayout *pLayout = new QGridLayout;
    pLayout->addWidget(mpParametersList, 1, 1, 1, 1);
    pLayout->addWidget(mpOutputBox, 1, 3, 1, 1);
    pLayout->addWidget(mpButtonBox, 2, 1, 1, 3);
    setLayout(pLayout);

    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpGenerateButton,               SIGNAL(clicked()),      this,                   SLOT(updateOutputBox()));
    connect(mpRunButton,                    SIGNAL(clicked()),      this,                   SLOT(run()));
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
            pParameterItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            pComponentItem->insertChild(0, pParameterItem);
        }
    }

    QDialog::show();
}


void OptimizationDialog::generateScriptFile()
{
    //! @todo Finish

    QTextStream scriptStream(&mScript);

    scriptStream << "############################################\n";
    scriptStream << "## Complex Optimization Script for Hopsan ##\n";
    scriptStream << "############################################\n";
    scriptStream << "\n";
    scriptStream << "import random\n";
    scriptStream << "from time import sleep\n";
    scriptStream << "from numpy import zeros\n";
    scriptStream << "from types import FloatType\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### AUXILIARY FUNCTIONS #####\n";
    scriptStream << "\n";
    scriptStream << "#Help functions for objectives\n";
    scriptStream << "\n";
    scriptStream << "#Returns the first time where vector data is at value x\n";
    scriptStream << "def firstTimeAt(data,time, x):\n";
    scriptStream << "  for i in range(len(data)):\n";
    scriptStream << "    if data[i] > x:\n";
    scriptStream << "      return time[i]\n";
    scriptStream << "  return time[i]\n";
    scriptStream << "\n";
    scriptStream << "#Returns the maximum value of vector data\n";
    scriptStream << "def maxValue(data):\n";
    scriptStream << "  max = data[0]\n";
    scriptStream << "  for i in range(len(data)):\n";
    scriptStream << "    if data[i] > max:\n";
    scriptStream << "      max = data[i]\n";
    scriptStream << "  return max\n";
    scriptStream << "  \n";
    scriptStream << "#Returns the amount of overshoot above specified limit\n";
    scriptStream << "def overShoot(data, limit):\n";
    scriptStream << "  os=0;\n";
    scriptStream << "  for i in range(len(data)):\n";
    scriptStream << "    if data[i]-limit > os:\n";
    scriptStream << "      os=data[i]-limit\n";
    scriptStream << "  return os\n";
    scriptStream << "   \n";
    scriptStream << "#Auxiliary optimization funcions\n";
    scriptStream << "\n";
    scriptStream << "#Returns the index of the maximum value in vector data\n";
    scriptStream << "def indexOfMax(data):\n";
    scriptStream << "  max = data[0]\n";
    scriptStream << "  maxId=0\n";
    scriptStream << "  for i in range(len(data)):\n";
    scriptStream << "    if data[i] > max:\n";
    scriptStream << "      max=data[i]\n";
    scriptStream << "      maxId=i\n";
    scriptStream << "  return maxId\n";
    scriptStream << "\n";
    scriptStream << "#Sums the elements with index i in each subvector in a vector of vectors\n";
    scriptStream << "def sum(vector, i):\n";
    scriptStream << "  retval=0\n";
    scriptStream << "  for j in range(len(vector)):\n";
    scriptStream << "    retval = retval + vector[j][i]\n";
    scriptStream << "  return retval\n";
    scriptStream << "  \n";
    scriptStream << "#Reflects the worst point in vector through the centroid of the remaining points, with reflection coefficient alpha\n";
    scriptStream << "def reflectWorst(vector, worstId, alpha):\n";
    scriptStream << "  n = len(vector)\n";
    scriptStream << "  k = len(vector[0])\n";
    scriptStream << "  x_w = vector[worstId]\n";
    scriptStream << "  x_c = []\n";
    scriptStream << "  for i in range(k):\n";
    scriptStream << "    x_c.append(1.0/(n-1.0)*(sum(vector,i)-x_w[i]))\n";
    scriptStream << "  x_new = []\n";
    scriptStream << "  for i in range(k):\n";
    scriptStream << "    x_new.append(max(minValues[i], min(maxValues[i], x_c[i]+alpha*(x_c[i]-x_w[i]))))\n";
    scriptStream << "  vector[worstId] = x_new  \n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Simulation settings #####\n";
    scriptStream << "\n";
    scriptStream << "timestep = 0.001\n";
    scriptStream << "time=5\n";
    scriptStream << "iterations=100\n";
    scriptStream << "hopsan.setTimeStep(timestep)\n";
    scriptStream << "hopsan.setStartTime(0)\n";
    scriptStream << "hopsan.setFinishTime(time)\n";
    scriptStream << "hopsan.turnOffProgressBar()\n";
    scriptStream << "alpha=1.3\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Optimization parameters #####\n";
    scriptStream << "\n";
    scriptStream << "parameters = [[0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0],\n";
    scriptStream << "              [0.0, 0.0]]\n";
    scriptStream << "componentNames = [\"GainP\", \"GainI\"]   #Names of components where parameters are located\n";
    scriptStream << "parameterNames = [\"k\", \"k\"]           #Names of parameters to optimize\n";
    scriptStream << "minValues = [0.0, 0.0]                    #Minimum value for each parameter\n";
    scriptStream << "maxValues = [0.01, 0.01]                #Maximum value for each parameter\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "\n";
    scriptStream << "##### Objective function #####\n";
    scriptStream << "\n";
    scriptStream << "obj = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]\n";
    scriptStream << "def getObjective():\n";
    scriptStream << "  e1=0.5\n";
    scriptStream << "  e2=0.5\n";
    scriptStream << "  data1=hopsan.component(\"Translational Mass\").port(\"P2\").getDataVector(\"Position\")\n";
    scriptStream << "  time1=hopsan.component(\"Translational Mass\").port(\"P2\").getTimeVector()\n";
    scriptStream << "  data2=hopsan.component(\"Translational Mass\").port(\"P2\").getDataVector(\"Position\")\n";
    scriptStream << "  return e1*firstTimeAt(data1,time1,0.65) + e2*overShoot(data2, 0.7)\n";
    scriptStream << "\n";
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
    scriptStream << "hopsan.simulate()\n";
    scriptStream << "hopsan.plot(\"Translational Mass\",\"P2\",\"Position\")\n";
    scriptStream << "\n";
    scriptStream << "previousWorstId = -1\n";
    scriptStream << "for k in range(iterations):\n";
    scriptStream << "#def iterate():\n";
    scriptStream << "  for i in range(len(parameters)):\n";
    scriptStream << "    for j in range(len(parameterNames)):\n";
    scriptStream << "      hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[i][j])\n";
    scriptStream << "    hopsan.simulate()\n";
    scriptStream << "    #hopsan.closeLastPlotWindow()\n";
    scriptStream << "    #hopsan.plot(\"Translational Mass\",\"P2\",\"Position\")\n";
    scriptStream << "    #hopsan.refreshLastPlotWindow()\n";
    scriptStream << "    obj[i] = getObjective()\n";
    scriptStream << "  worstId = indexOfMax(obj)\n";
    scriptStream << "  if worstId == previousWorstId:\n";
    scriptStream << "    reflectWorst(parameters,worstId,alpha/2.0)\n";
    scriptStream << "  else:\n";
    scriptStream << "    reflectWorst(parameters,worstId,alpha)\n";
    scriptStream << "  previousWorstId=worstId\n";
    scriptStream << "  \n";
}


void OptimizationDialog::updateOutputBox()
{
    //! @todo Implement

    generateScriptFile();
    mpOutputBox->insertPlainText(mScript);
}



void OptimizationDialog::run()
{
    //! @todo Implement

    close();
}
