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
//! @file   OptimizationDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-10-24
//!
//! @brief Contains a class for the optimization dialog
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

//C++ includes
#include <limits>
#ifndef _WIN32
#include <unistd.h>
#endif

//Hopsan includes
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "HcomHandler.h"
#include "Utilities/HelpPopUpWidget.h"
#include "ModelHandler.h"
#include "OptimizationHandler.h"
#include "Utilities/HighlightingUtilities.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "Utilities/GUIUtilities.h"

class CentralTabWidget;


//! @brief Constructor
OptimizationDialog::OptimizationDialog(QWidget *parent)
    : QWizard(parent)
{
        //Set the name and size of the main window
    this->resize(1024,768);
    this->setWindowTitle("Optimization");
    this->setPalette(gpConfig->getPalette());

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
    mpCPLineEdit = new QLineEdit("0.5", this);
    mpCPLineEdit->setValidator(new QDoubleValidator());

    mpMPLabel = new QLabel("Mutation probability: ");
    mpMPLineEdit = new QLineEdit("0.5", this);
    mpMPLineEdit->setValidator(new QDoubleValidator());

    mpNumModelsLabel = new QLabel("Number of models: ");
    mpNumModelsLineEdit = new QLineEdit(QString::number(qMax(1,gpConfig->getIntegerSetting(CFG_NUMBEROFTHREADS))), this);
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
    connect(mpAlgorithmBox, SIGNAL(currentIndexChanged(int)), this, SLOT(recreateCoreProgressBars()));
    pSettingsLayout->addWidget(pIterationsLabel,       row,   0);
    pSettingsLayout->addWidget(mpIterationsSpinBox,    row++, 1);
    pSettingsLayout->addWidget(mpParticlesLabel,       row,   0);
    pSettingsLayout->addWidget(mpParticlesSpinBox,     row++, 1);
    connect(mpParticlesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(recreateCoreProgressBars()));
    connect(mpParticlesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(recreateParameterOutputLineEdits()));
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
    QWizardPage *pSettingsWidget = new QWizardPage(this);
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
    QWizardPage *pParametersWidget = new QWizardPage(this);
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
    QWizardPage *pObjectiveWidget = new QWizardPage(this);
    pObjectiveWidget->setLayout(mpObjectiveLayout);

    //Output box tab
    mpOutputBox = new QTextEdit(this);
    HcomHighlighter *pHighligter = new HcomHighlighter(mpOutputBox->document());
    Q_UNUSED(pHighligter);
    QFont monoFont = mpOutputBox->font();
    monoFont.setFamily("Courier");
    monoFont.setPointSize(11);
    mpOutputBox->setFont(monoFont);
    mpOutputBox->setMinimumWidth(450);
    QGridLayout *pOutputLayout = new QGridLayout(this);
    pOutputLayout->addWidget(mpOutputBox);
    QWizardPage *pOutputWidget = new QWizardPage(this);
    pOutputWidget->setLayout(pOutputLayout);

    //Tool bar
    QToolButton *pHelpButton = new QToolButton(this);
    pHelpButton->setToolTip(tr("Show context help"));
    pHelpButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Help.png"));
    this->setButton(QWizard::HelpButton, pHelpButton);
    this->setOption(QWizard::HaveHelpButton);
    pHelpButton->setObjectName("optimizationHelpButton");

    //Run tab
    mpStartButton = new QPushButton("Start Optimization", this);
    mpModelNameLabel = new QLabel("Model name: ", this);
    mpScriptFileLabel = new QLabel("Script File:", this);
    mpTotalProgressBar = new QProgressBar(this);
    mpTotalProgressBar->hide();
    mpTotalProgressBar->setPalette(gpConfig->getPalette());
    mpCoreProgressBarsLayout = new QGridLayout();
    QWidget *pScrollAreaWidget = new QWidget(this);
    pScrollAreaWidget->setPalette(gpConfig->getPalette());
    pScrollAreaWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *pScrollAreaLayout = new QHBoxLayout(pScrollAreaWidget);
    mpParametersOutputTextEditsLayout = new QGridLayout();
    pScrollAreaLayout->addLayout(mpParametersOutputTextEditsLayout);
    pScrollAreaLayout->addLayout(mpCoreProgressBarsLayout);
    QScrollArea *pParametersOutputScrollArea = new QScrollArea(this);
    pParametersOutputScrollArea->setPalette(gpConfig->getPalette());
    pParametersOutputScrollArea->setWidget(pScrollAreaWidget);
    pParametersOutputScrollArea->setWidgetResizable(true);
    pParametersOutputScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mpTerminal = new TerminalWidget(this);
    mpTerminal->mpHandler->setAcceptsOptimizationCommands(true);
    mpMessageHandler = mpTerminal->mpHandler->mpOptHandler->getMessageHandler();
    QGridLayout *pRunLayout = new QGridLayout(this);
    pRunLayout->addWidget(mpModelNameLabel,               0,0,1,1);
    pRunLayout->addWidget(mpScriptFileLabel,              1,0,1,1);
    pRunLayout->addWidget(mpStartButton,                  2,0,1,1);
    pRunLayout->addWidget(pParametersOutputScrollArea,    3,0,1,2);
    //pRunLayout->addLayout(mpCoreProgressBarsLayout,             1,1,1,1);
    pRunLayout->addWidget(mpTerminal,                           4,0,1,2);
    pRunLayout->setRowStretch(3,2.4);
    pRunLayout->setRowStretch(4,2.6);
    pRunLayout->setColumnStretch(0,1);
    pRunLayout->setColumnMinimumWidth(1,400);
    pRunLayout->addWidget(mpTotalProgressBar,           5,1,1,2);
    QWizardPage *pRunWidget = new QWizardPage(this);
    pRunWidget->setLayout(pRunLayout);
    pRunWidget->setPalette(gpConfig->getPalette());

    this->addPage(pSettingsWidget);
    this->addPage(pParametersWidget);
    this->addPage(pObjectiveWidget);
    this->addPage(pOutputWidget);
    this->addPage(pRunWidget);

    setButtonText(QWizard::FinishButton, tr("&Close Dialog"));
    setButtonText(QWizard::CustomButton1, tr("&Save To Script File"));
    setButtonText(QWizard::CustomButton2, tr("&Load From Script File"));
    setButtonText(QWizard::CustomButton3, tr("&Regenerate Script"));
    setOption(QWizard::HaveCustomButton1, true);
    setOption(QWizard::HaveCustomButton2, true);
    setOption(QWizard::HaveCustomButton3, true);
    setOption(QWizard::CancelButtonOnLeft, false);
    //button(QWizard::CustomButton1)->setDisabled(true);
    button(QWizard::FinishButton)->setEnabled(true);
    button(QWizard::FinishButton)->setHidden(true);
    button(QWizard::CustomButton3)->setHidden(true);
    button(QWizard::CustomButton3)->setDisabled(true);
    button(QWizard::CustomButton1)->setDisabled(true);

    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), this, SLOT(updateCoreProgressBars()));
    mpTimer->setSingleShot(false);

    //Connections
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(update(int)));
    connect(mpAddFunctionButton,            SIGNAL(clicked()),      this,                   SLOT(addFunction()));
    connect(pHelpButton,                   SIGNAL(clicked()),    gpHelpPopupWidget,           SLOT(openContextHelp()));
    connect(mpStartButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(saveScriptFile()));
    connect(button(QWizard::CustomButton2), SIGNAL(clicked()), this, SLOT(loadScriptFile()));
    connect(button(QWizard::CustomButton3), SIGNAL(clicked()), this, SLOT(regenerateScript()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveConfiguration()));
}


//! @brief Updates output boxes displaying the parameters
void OptimizationDialog::updateParameterOutputs(const std::vector<double> &objectives, const std::vector<std::vector<double> > &values, const int bestId, const int worstId)
{
    if(mOutputDisabled || !this->isVisible()) return;

    if(mParametersOutputLineEditPtrs.size() != objectives.size())
    {
        recreateParameterOutputLineEdits();
    }

//    double temp = objectives[0];
//    objectives[0] = objectives[bestId];
//    objectives.insert(bestId, temp);

//    QVector<double> vTemp = values[0];
//    values[0] = values[bestId];
//    values.insert(bestId, vTemp);

//    temp = objectives[1];
//    objectives[1] = objectives[worstId];
//    objectives.insert(worstId, temp);

//    vTemp = values[1];
//    values[1] = values[worstId];
//    values.insert(worstId, vTemp);

//    bestId = 0;
//    worstId = 1;

    bool ok;
    int nPoints = mpTerminal->mpHandler->mpOptHandler->getOptVar("npoints", ok);        //! @todo Slow to use strings, should use direct access somehow
    if(nPoints != mParametersOutputLineEditPtrs.size())
    {
        mpParticlesSpinBox->setValue(nPoints);
        recreateParameterOutputLineEdits();
    }


    mParameterOutputIndexes.clear();
    mParameterOutputIndexes.append(bestId);
    mParameterOutputIndexes.append(worstId);
    if(bestId == worstId)
    {
        mParameterOutputIndexes.remove(0);
    }
    for(int i=0; i<values.size(); ++i)
    {
        if(!mParameterOutputIndexes.contains(i))
        {
            mParameterOutputIndexes.append(i);
        }
    }

    //QStringList *pParNames = mpTerminal->mpHandler->mpOptHandler->getOptParNamesPtr();

    for(int x=0; x<mParameterOutputIndexes.size(); ++x)
    {
        int i = mParameterOutputIndexes[x];
        if(i >= objectives.size()) continue;

        QString output = "obj: ";
        QString objStr;
        //! @todo This is an error in the code, this must be solved, previous code assumed that i is in range of objects -> ASSERT failed in QVector
        qDebug() << "objectives size: " << objectives.size() << " " << i;
        if (i < objectives.size())
        {
            objStr = QString::number(objectives[i], 'g', 8);
        }
        else
        {
            mpMessageHandler->addErrorMessage("In code: updateParameterOutputs objectives.size() < i");
            return;
        }
        while(objStr.size() < 12)
        {
            objStr.append(" ");
        }
        output.append(objStr);
        output.append(" [ ");
        for(int j=0; j<values[i].size(); ++j)
        {
            QString numStr = QString::number(values[i][j], 'g', 8);
            numStr.append(",");
            while(numStr.size() < 15)
            {
                numStr.append(" ");
            }
            output.append(numStr);
        }
        output.append("]");
        QPalette palette;
        if(i == bestId)
            palette.setColor(QPalette::Text,Qt::darkGreen);
        else if(i == worstId)
            palette.setColor(QPalette::Text,Qt::darkRed);
        else
            palette.setColor(QPalette::Text,Qt::black);
        mParametersOutputLineEditPtrs[x]->setPalette(palette);
        mParametersOutputLineEditPtrs[x]->setText(output);
        mParametersOutputLineEditPtrs[x]->setCursorPosition(0);
    }
}

void OptimizationDialog::updateTotalProgressBar(double progress)
{
    if(mOutputDisabled || !this->isVisible()) return;

    mpTotalProgressBar->setValue(progress);
}

void OptimizationDialog::setOptimizationFinished()
{
    mpStartButton->setEnabled(true);
    for(int i=0; i<mParametersApplyButtonPtrs.size(); ++i)
    {
        mParametersApplyButtonPtrs[i]->setEnabled(true);
    }
}

void OptimizationDialog::setCode(const QString &code)
{
    this->next();
    this->next();
    this->next();
    this->next();
    this->next();
    this->back();   //Ugly hack to show correct page
    mpOutputBox->setPlainText(code);
}


void OptimizationDialog::loadConfiguration()
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

void OptimizationDialog::regenerateScript()
{
    generateScriptFile();
    mpOutputBox->clear();
    mpOutputBox->insertPlainText(mScript);
    saveConfiguration();
    button(QWizard::CustomButton3)->setEnabled(true);
}


//! @brief Reimplementation of open() slot, used to initialize the dialog
void OptimizationDialog::open()
{
    mpSystem = gpModelHandler->getCurrentTopLevelSystem();
    connect(mpSystem, SIGNAL(destroyed()), this, SLOT(close()));

    //Set correct working directory for HCOM terminal
    mpTerminal->mpHandler->setWorkingDirectory(gpTerminalWidget->mpHandler->getWorkingDirectory());

    //Copy local variables from main HCOM terminal
    mpTerminal->mpHandler->setLocalVariables(gpTerminalWidget->mpHandler->getLocalVariables());

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

    recreateCoreProgressBars();
    recreateParameterOutputLineEdits();

    if(!mpTerminal->mpHandler->mpOptHandler->isRunning())
    {
        this->mpTerminal->mpHandler->mpOptHandler->clearModels();
    }

    QDialog::show();
}


//! @brief Slot that handles accept event (clicking "Close" button)
void OptimizationDialog::accept()
{
    //! @todo Duplicated code with reject() function
    if(mpTerminal->mpHandler->mpOptHandler->isRunning())    //Optimization is running, ask user about it
    {
        QMessageBox closeWarningBox(QMessageBox::Warning, tr("Warning"),tr("An optimization is still running. Do you wish to abort it?\n\nHint: Optimzations can be aborted without closing the dialog with the \"Abort Script\" button."), 0, 0);
        closeWarningBox.addButton(QMessageBox::Yes);
        closeWarningBox.addButton(QMessageBox::No);
        closeWarningBox.addButton(QMessageBox::Cancel);
        closeWarningBox.setWindowIcon(gpMainWindowWidget->windowIcon());

        int rc = closeWarningBox.exec();
        if(rc == QMessageBox::Yes)
        {
            this->mpTerminal->mpHandler->abortHCOM();
        }
        else if(rc == QMessageBox::Cancel)
        {
            return;
        }
    }

    QDialog::accept();
}


//! @brief Slot that handles reject event (clicking "Cancel" button)
void OptimizationDialog::reject()
{
    //! @todo Duplicated code with accept() function
    if(mpTerminal->mpHandler->mpOptHandler->isRunning())    //Optimization is running, ask user about it
    {
        QMessageBox closeWarningBox(QMessageBox::Warning, tr("Warning"),tr("An optimization is still running. Do you wish to abort it?\n\nHint: Optimzations can be aborted without closing the dialog with the \"Abort Script\" button."), 0, 0);
        closeWarningBox.addButton(QMessageBox::Yes);
        closeWarningBox.addButton(QMessageBox::No);
        closeWarningBox.addButton(QMessageBox::Cancel);
        closeWarningBox.setWindowIcon(gpMainWindowWidget->windowIcon());

        int rc = closeWarningBox.exec();
        if(rc == QMessageBox::Yes)
        {
            this->mpTerminal->mpHandler->abortHCOM();
        }
        else if(rc == QMessageBox::Cancel)
        {
            return;
        }
    }

    QDialog::reject();
}


//! @brief Slot that triggers when "ok" button in dialog is pressed
void OptimizationDialog::okPressed()
{
    saveConfiguration();

    reject();
}

void OptimizationDialog::setOutputDisabled(bool disabled)
{
    mOutputDisabled = disabled;
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
        //This error makes no sense, since it is showed on the last page only, so user will see it too late
        //mpMessageHandler->addErrorMessage("No parameters specified for optimization.");
        return;
    }

    if(mSelectedFunctions.isEmpty())
    {
        //This error makes no sense, since it is showed on the last page only, so user will see it too late
        //mpMessageHandler->addErrorMessage("No objective functions specified for optimization.");
        return;
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

    if(algorithmOk)
    {
        button(QWizard::CustomButton1)->setEnabled(true);
        mpScriptFileLabel->setText("Script File: Generated");
    }
    else
        mpMessageHandler->addErrorMessage("Algorithm type undefined.");

}

void OptimizationDialog::generateNelderMeadScript()
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

void OptimizationDialog::generateComplexRFScript(const QString &subAlgorithm)
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


void OptimizationDialog::generateParticleSwarmScript()
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

void OptimizationDialog::generateDifferentialEvolutionScript()
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

void OptimizationDialog::generateGeneticScript()
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
    templateCode.replace("<<<partol>>>", mpEpsilonXLineEdit->text());
    templateCode.replace("<<<nmodels>>>", mpNumModelsLineEdit->text());

    mScript = templateCode;
}


void OptimizationDialog::generateParameterSweepScript()
{
    QFile templateFile(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optTemplateParameterSweep.hcom");
    templateFile.open(QFile::ReadOnly | QFile::Text);
    QString templateCode = templateFile.readAll();
    templateFile.close();

    generateObjectiveFunctionCode(templateCode);
    generateParameterCode(templateCode);
    generateCommonOptions(templateCode);

    int nThreads = gpConfig->getIntegerSetting(CFG_NUMBEROFTHREADS);
    templateCode.replace("<<<evals>>>", QString::number(mpLengthSpinBox->value()/double(nThreads)));
    templateCode.replace("<<<nmodels>>>", QString::number(nThreads));


    mScript = templateCode;
}

void OptimizationDialog::generateObjectiveFunctionCode(QString &templateCode)
{
    QString objFuncs, totalObj;
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
    objFuncs.chop(1);

    replacePattern("<<<objfuncs>>>", objFuncs, templateCode);
    replacePattern("<<<totalobj>>>", totalObj, templateCode);
}

void OptimizationDialog::generateParameterCode(QString &templateCode)
{
    QString setMinMax, setPars;
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
    // Remove last newlines
    setPars.chop(1);
    setMinMax.chop(1);

    replacePattern("<<<setminmax>>>", setMinMax, templateCode);
    replacePattern("<<<setpars>>>", setPars, templateCode);
}

void OptimizationDialog::generateCommonOptions(QString &templateCode)
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



void OptimizationDialog::setAlgorithm(int i)
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

    //mpAlgorithmBox->addItems(QStringList() << "Simplex" << "Complex-RF" << "Complex-RFP" << "Particle Swarm" << "Differential Evolution" << "Parameter Sweep");

    //enum AlgorithmT {Undefined, ComplexRF, ComplexRFP, NelderMead, ParticleSwarm, ParameterSweep, DifferentialEvolution};

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
    case Ops::ParameterSweep:
        break;
    default:
        break;
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


//! @brief Removes an objective function from the selected functions
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
    pRemoveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
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


//! @brief Removes an objective function from the selected functions
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
    button(QWizard::CustomButton3)->setVisible(false);      //Should be hidden on all tabs except code tab

    //Finished parameters tab
    if(idx == 2)
    {
        if(mSelectedParameters.isEmpty())
        {
            //This error makes no sense, since it is showed on the last page only, so user will see it too late
            //mpMessageHandler->addWarningMessage("No parameters specified for optimization.");
//            this->back();
            return;
        }
    }

    //Finished objective function tab
    if(idx == 3)
    {
        button(QWizard::CustomButton3)->setVisible(true);
        button(QWizard::CustomButton3)->setEnabled(!mpOutputBox->toPlainText().isEmpty());

        if(mSelectedFunctions.isEmpty())
        {
            //This error message makes no sense anymore, since it is only showed on last tab, after user has probably fixed the errors already
            //mpMessageHandler->addWarningMessage("No objective functions specified for optimization.");
           // this->back();
            return;
        }
        else if(mpOutputBox->toPlainText().isEmpty())
        {
            //button(QWizard::CustomButton1)->setDisabled(false);
            generateScriptFile();
            mpOutputBox->clear();
            mpOutputBox->insertPlainText(mScript);
            saveConfiguration();
            button(QWizard::CustomButton3)->setEnabled(true);
            return;
        }
    }

    if(idx == 4)
    {
        mpTerminal->mpHandler->setModelPtr(gpModelHandler->getCurrentModel());
        mpModelNameLabel->setText("Model name: "+gpModelHandler->getCurrentModel()->getTopLevelSystemContainer()->getName());
    }
}



//! @brief Saves the generated script code to file and executes the script
void OptimizationDialog::run()
{
    //saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    saveScriptFile(gpDesktopHandler->getBackupPath()+"optimization_script"+QDateTime::currentDateTime().toString("_yyyy-MM-dd_hh_mm")+".hcom");

    mCoreProgressBarsRecreated = false;

    saveConfiguration();

    recreateParameterOutputLineEdits();

    mpStartButton->setEnabled(false);
    for(int i=0; i<mParametersApplyButtonPtrs.size(); ++i)
    {
        mParametersApplyButtonPtrs[i]->setEnabled(false);
    }

    QStringList commands = mpOutputBox->toPlainText().split("\n");
    bool *abort = new bool;
    *abort = false;
    mpTerminal->setAbortButtonEnabled(true);
    mpTimer->start(10);
    mpTerminal->mpHandler->runScriptCommands(commands, abort);
    mpTerminal->setAbortButtonEnabled(false);
    setOptimizationFinished();
    mpTimer->stop();
    delete(abort);
}


//! @brief Saves generated script to path specified by user
void OptimizationDialog::saveScriptFile()
{
    if(mpOutputBox->toPlainText().isEmpty())
    {
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Script File"),
                                                 gpConfig->getStringSetting(CFG_SCRIPTDIR),
                                                 this->tr("HCOM Script (*.hcom)"));

    if(filePath.isEmpty())     //Don't save anything if user presses cancel
    {
        return;
    }

    QFileInfo fileInfo = QFileInfo(filePath);
    gpConfig->setStringSetting(CFG_SCRIPTDIR, fileInfo.absolutePath());

    saveScriptFile(filePath);

    mpScriptFileLabel->setText("Script file: "+fileInfo.fileName());
}

//! @brief Saves generated script to specified path
void OptimizationDialog::saveScriptFile(const QString &filePath)
{
    QFile file(filePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&file);
    out << mpOutputBox->toPlainText();
    file.close();
}


void OptimizationDialog::loadScriptFile()
{
    QString filePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Load Script File)"),
                                                    gpConfig->getStringSetting(CFG_SCRIPTDIR),
                                                    tr("HCOM Script (*.hcom)"));
    if(filePath.isEmpty())      //Canceled by user
        return;

    gpConfig->setStringSetting(CFG_SCRIPTDIR, QFileInfo(filePath).absolutePath());

    QFile file(filePath);
    file.open(QFile::Text | QFile::ReadOnly);
    QString script = file.readAll();
    file.close();

    setCode(script);

    mScript = script;

    mpScriptFileLabel->setText("Script file: "+QFileInfo(file).fileName());

    button(QWizard::CustomButton1)->setEnabled(true);
}

void OptimizationDialog::updateCoreProgressBars()
{
    if(mOutputDisabled || !this->isVisible()) return;

    if(!mCoreProgressBarsRecreated && mpTerminal->mpHandler->mpOptHandler->isRunning())
    {
        recreateCoreProgressBars();
        mCoreProgressBarsRecreated = true;
    }

    for(int p=0; p<mCoreProgressBarPtrs.size(); ++p)
    {
        OptimizationHandler *pOptHandler = mpTerminal->mpHandler->mpOptHandler;
        if(pOptHandler && pOptHandler->mpWorker)
        {
            if(pOptHandler->mModelPtrs.size() > p)
            {
                ModelWidget *pOptModel = pOptHandler->mModelPtrs[p];
#ifdef USEZMQ
                if (pOptModel->isRemoteCoreConnected())
                {
                    mCoreProgressBarPtrs[p]->setValue(pOptModel->getSimulationProgress()*100);
                }
                else
                {
                    double stopT = pOptModel->getStopTime().toDouble();
                    if(pOptModel)
                    {
                        CoreSystemAccess *pCoreSystem = pOptModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr();
                        mCoreProgressBarPtrs[p]->setValue(pCoreSystem->getCurrentTime() / stopT *100);
                    }
                }
#else
                double stopT = pOptModel->getStopTime().toDouble();
                if(pOptModel)
                {
                    CoreSystemAccess *pCoreSystem = pOptModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr();
                    mCoreProgressBarPtrs[p]->setValue(pCoreSystem->getCurrentTime() / stopT *100);
                }
#endif
            }
        }
    }
}

void OptimizationDialog::recreateCoreProgressBars()
{
    //Clear all previous stuff
    QLayoutItem *item;
    while((item = mpCoreProgressBarsLayout->takeAt(0)))
    {
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }
    mCoreProgressBarPtrs.clear();

    // Decide if we should show progress per particle or just current simulation
    bool showProgressPerParticle = gpConfig->getUseMulticore() || gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION);

    //Add new stuff depending on algorithm and number of threads
    switch (mpTerminal->mpHandler->mpOptHandler->getAlgorithm())
    {
    case Ops::NelderMead :    //Complex-RF
        mCoreProgressBarPtrs.append(new QProgressBar(this));
        mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
        mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        break;
    case Ops::ComplexRF :    //Complex-RF
        mCoreProgressBarPtrs.append(new QProgressBar(this));
        mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
        mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        break;
    case Ops::ComplexRFP :    //Complex-RFP
        if(showProgressPerParticle)
        {
            for(int n=0; n<mpTerminal->mpHandler->mpOptHandler->mModelPtrs.size(); ++n)
            {
                mCoreProgressBarPtrs.append(new QProgressBar(this));
                mpCoreProgressBarsLayout->addWidget(new QLabel("Particle "+QString::number(n)+":", this), n, 0);
                mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(), n, 1);
            }
        }
        else
        {
            mCoreProgressBarPtrs.append(new QProgressBar(this));
            mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
            mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        }
        break;
    case Ops::ParticleSwarm :    //Particle swarm
        if(showProgressPerParticle)
        {
            for(int n=0; n<mpTerminal->mpHandler->mpOptHandler->mModelPtrs.size(); ++n)
            {
                mCoreProgressBarPtrs.append(new QProgressBar(this));
                mpCoreProgressBarsLayout->addWidget(new QLabel("Particle "+QString::number(n)+":", this), n, 0);
                mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(), n, 1);
            }
        }
        else
        {
            mCoreProgressBarPtrs.append(new QProgressBar(this));
            mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
            mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        }
        break;
    case Ops::ParameterSweep :    //Particle swarm
        if(showProgressPerParticle)
        {
            for(int n=0; n<mpTerminal->mpHandler->mpOptHandler->mModelPtrs.size(); ++n)
            {
                mCoreProgressBarPtrs.append(new QProgressBar(this));
                mpCoreProgressBarsLayout->addWidget(new QLabel("Particle "+QString::number(n)+":", this), n, 0);
                mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(), n, 1);
            }
        }
        else
        {
            mCoreProgressBarPtrs.append(new QProgressBar(this));
            mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
            mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        }
        break;
    case Ops::Genetic:    //Particle swarm
        if(showProgressPerParticle)
        {
            for(int n=0; n<mpTerminal->mpHandler->mpOptHandler->mModelPtrs.size(); ++n)
            {
                mCoreProgressBarPtrs.append(new QProgressBar(this));
                mpCoreProgressBarsLayout->addWidget(new QLabel("Particle "+QString::number(n)+":", this), n, 0);
                mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(), n, 1);
            }
        }
        else
        {
            mCoreProgressBarPtrs.append(new QProgressBar(this));
            mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
            mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        }
        break;
    default:
        break;
    }

    mpTotalProgressBar = new QProgressBar(this);
    mpCoreProgressBarsLayout->addWidget(new QLabel("Total: ", this),mCoreProgressBarPtrs.size(), 0);
    mpCoreProgressBarsLayout->addWidget(mpTotalProgressBar, mCoreProgressBarPtrs.size(), 1);
    mpCoreProgressBarsLayout->addWidget(new QWidget(this), mCoreProgressBarPtrs.size()+1, 0, 1, 2);
    mpCoreProgressBarsLayout->setRowStretch(mCoreProgressBarPtrs.size()+1, 1);
    return;
}

void OptimizationDialog::recreateParameterOutputLineEdits()
{
    int nPoints = mpTerminal->mpHandler->mpOptHandler->getOptVar("npoints");
//    switch(mpTerminal->mpHandler->mpOptHandler->getAlgorithm())
//    {
//    case Ops::NelderMead:
//        nPoints=mpSearchPointsSpinBox->value();
//        break;
//    case Ops::ComplexRF:     //Complex-RF
//        nPoints=mpSearchPointsSpinBox->value();
//        break;
////    case OptimizationHandler::ComplexRFM:     //Complex-RFM
////        nPoints=mpSearchPointsSpinBox->value();
////        break;
//    case Ops::ComplexRFP:     //Complex-RFP
//        nPoints=mpSearchPointsSpinBox->value();
//        break;
//    case Ops::ParticleSwarm:     //Complex
//        nPoints=mpParticlesSpinBox->value();
//        break;
//    default:
//        nPoints=0;
//    }

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    while(mParametersOutputLineEditPtrs.size() < nPoints)
    {
        mParametersOutputLineEditPtrs.append(new QLineEdit(this));
        mParametersOutputLineEditPtrs.last()->setFont(font);
        mParametersApplyButtonPtrs.append(new QPushButton("Apply", this));
        mpParametersOutputTextEditsLayout->addWidget(mParametersApplyButtonPtrs.last(), mParametersOutputLineEditPtrs.size(), 0);
        mpParametersOutputTextEditsLayout->addWidget(mParametersOutputLineEditPtrs.last(), mParametersOutputLineEditPtrs.size(), 1);
        mParametersApplyButtonPtrs.last()->setEnabled(false);
        connect(mParametersApplyButtonPtrs.last(), SIGNAL(clicked()), this, SLOT(applyParameters()));
    }
    while(mParametersOutputLineEditPtrs.size() > nPoints)
    {
        mpParametersOutputTextEditsLayout->removeWidget(mParametersOutputLineEditPtrs.last());
        mpParametersOutputTextEditsLayout->removeWidget(mParametersApplyButtonPtrs.last());
        delete mParametersOutputLineEditPtrs.last();
        delete mParametersApplyButtonPtrs.last();
        mParametersOutputLineEditPtrs.removeLast();
        mParametersApplyButtonPtrs.removeLast();
    }
}


//! @brief Slot that applies the parameters in a point to the original model. Index of the point is determined by the sender of the signal.
void OptimizationDialog::applyParameters()
{
    if(mParameterOutputIndexes.isEmpty())       //Just for safety, should not happen
        return;

    QPushButton *pSender = qobject_cast<QPushButton*>(QObject::sender());
    if(!pSender) return;
    int idx = mParametersApplyButtonPtrs.indexOf(pSender);
    idx = mParameterOutputIndexes.at(idx);

    if(gpModelHandler->count() == 0 || !gpModelHandler->getCurrentModel())
    {
        QMessageBox::critical(this, "Error", "No model is open.");
        return;
    }

     //Temporary switch optimization handler in global HCOM handler to this
    OptimizationHandler *pOrgOptHandler = gpTerminalWidget->mpHandler->mpOptHandler;
    gpTerminalWidget->mpHandler->mpOptHandler = mpTerminal->mpHandler->mpOptHandler;

    QStringList code;
    mpTerminal->mpHandler->getFunctionCode("setpars", code);
    bool abort;
    bool oldEchoState = gpTerminalWidget->mpHandler->mpConsole->getDontPrint();     //Remember old don't print setting (necessary in case the setpars function contains an "echo off" command)
    bool oldEchoStateError = gpTerminalWidget->mpHandler->mpConsole->getDontPrintErrors();
    gpTerminalWidget->mpHandler->setAcceptsOptimizationCommands(true);              //Temporarily allow optimization commands in main terminal
    gpTerminalWidget->mpHandler->runScriptCommands(QStringList() << "opt set evalid "+QString::number(idx), &abort);    //Set evalId corresponding to clicked button
    gpTerminalWidget->mpHandler->runScriptCommands(code, &abort);                   //Run the setpars function
    gpTerminalWidget->mpHandler->setAcceptsOptimizationCommands(false);             //Disable optimization commands again
    gpTerminalWidget->mpHandler->mpConsole->setDontPrint(oldEchoState, !oldEchoStateError);             //Reset old dont print setting

    //Switch back HCOM handler
    gpTerminalWidget->mpHandler->mpOptHandler = pOrgOptHandler;
}


//! @brief Checks if number of selected variables is correct. Gives error messages if they are too many or too low.
//! @param i Selected objective function
bool OptimizationDialog::verifyNumberOfVariables(int idx, int nSelVar)
{
    int nVar = mObjectiveFunctionNumberOfVariables.at(idx);

    if(nSelVar > nVar)
    {
        //This error makes no sense, since it is showed on the last page only, so user will see it too late
        //mpMessageHandler->addErrorMessage("Too many variables selected for this function.");
        return false;
    }
    else if(nSelVar < nVar)
    {
        //This error makes no sense, since it is showed on the last page only, so user will see it too late
        //mpMessageHandler->addErrorMessage("Too few variables selected for this function.");
        return false;
    }
    return true;
}


bool OptimizationDialog::loadObjectiveFunctions()
{
    mObjectiveFunctionDescriptions.clear();
    mObjectiveFunctionCalls.clear();
    mObjectiveFunctionDataLists.clear();
    mObjectiveFunctionNumberOfVariables.clear();
    mObjectiveFunctionUsesTimeVector.clear();

    // Look in both local and global scripts directory in case they are different

    //QDir scriptsDir(gDesktopHandler.getExecPath()+"../Scripts/HCOM/objFuncTemplates");
    QDir scriptsDir(gpDesktopHandler->getScriptsPath()+"HCOM/objFuncTemplates");
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
