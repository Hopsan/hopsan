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
#include <QToolBar>

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
#include "OptimizationScriptWizard.h"

class CentralTabWidget;


//! @brief Constructor for optimization dialog
OptimizationDialog::OptimizationDialog(QWidget *parent)
  : QDialog(parent)
{
    //Set the name and size of the main window
    this->resize(1024,768);
    this->setWindowTitle("Optimization");
    this->setPalette(gpConfig->getPalette());

    // Main layout
    QVBoxLayout *pLayout = new QVBoxLayout(this);

    // Toolbar
    QToolBar *pToolBar = createToolBar();
    pLayout->addWidget(pToolBar);

    // Tab widget
    mpTabWidget = new QTabWidget(this);
    pLayout->addWidget(mpTabWidget);

    // Script Tab
    QWidget *pScriptWidget = createScriptWidget();
    mpTabWidget->addTab(pScriptWidget, "Script");

    // Run Tab
    QWidget *pRunWidget = createRunWidget();
    mpTabWidget->addTab(pRunWidget, "Run");

    //Progress Bar Timer
    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), this, SLOT(updateCoreProgressBars()));
    mpTimer->setSingleShot(false);

    //Dialog button box
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(this);
    pButtonBox->addButton(QDialogButtonBox::Close);
    pLayout->addWidget(pButtonBox);
    connect(pButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

//! @brief Updates output boxes displaying the parameters
void OptimizationDialog::updateParameterOutputs(const std::vector<double> &objectives, const std::vector<std::vector<double> > &values, const int bestId, const int worstId)
{
    if(mOutputDisabled || !this->isVisible()) return;

    if(mParametersOutputLineEditPtrs.size() != (int)objectives.size())
    {
        recreateParameterOutputLineEdits();
    }

    mParameterOutputIndexes.clear();
    mParameterOutputIndexes.append(bestId);
    mParameterOutputIndexes.append(worstId);
    if(bestId == worstId)
    {
        mParameterOutputIndexes.remove(0);
    }
    for(size_t i=0; i<values.size(); ++i)
    {
        if(!mParameterOutputIndexes.contains(i))
        {
            mParameterOutputIndexes.append(i);
        }
    }

    for(int x=0; x<mParameterOutputIndexes.size(); ++x)
    {
        int i = mParameterOutputIndexes[x];
        if(i >= (int)objectives.size()) continue;

        QString output = "obj: ";
        QString objStr;
        //! @todo This is an error in the code, this must be solved, previous code assumed that i is in range of objects -> ASSERT failed in QVector
        qDebug() << "objectives size: " << objectives.size() << " " << i;
        if (i < (int)objectives.size())
        {
            objStr = QString::number(objectives[i], 'g', 8);
        }
        else
        {
            mpMessageHandler->addErrorMessage("In code: updateParameterOutputs objectives.size() < i");
            return;
        }

        while(objStr.size() < mObjectiveColumnWidth)
        {
            objStr.append(" ");
        }
        output.append(objStr);
        output.append(" [ ");
        for(size_t j=0; j<values[i].size(); ++j)
        {
            QString numStr = QString::number(values[i][j], 'g', 8);
            numStr.append(",");
            while(numStr.size() < mParameterColumnWidth)
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


//! @brief Updates total progress bar
//! @param progress Progress (between 0.0 and 1.0)
void OptimizationDialog::updateTotalProgressBar(double progress)
{
    if(mOutputDisabled || !this->isVisible()) return;

    mpTotalProgressBar->setValue(progress);
}


//! @brief Flags that optimization algorithm is finished
void OptimizationDialog::setOptimizationFinished()
{
    mpRunButton->setEnabled(true);

    for(int i=0; i<mParametersApplyButtonPtrs.size(); ++i)
    {
        mParametersApplyButtonPtrs[i]->setEnabled(true);
    }
}


//! @brief Puts specified code in Script tab
//! @param code New script code
void OptimizationDialog::setCode(const QString &code)
{
    mpTabWidget->setCurrentIndex(0);    //Go to script tab
    mpScriptBox->setPlainText(code);
    mpScriptFileLabel->setText("Script File: Generated Script");
}


//! @brief Opens and initializes the optimization dialog
void OptimizationDialog::open()
{
    mpSystem = gpModelHandler->getCurrentTopLevelSystem();
    connect(mpSystem, SIGNAL(destroyed()), this, SLOT(close()));

    //Set correct working directory for HCOM terminal
    mpTerminal->mpHandler->setWorkingDirectory(gpTerminalWidget->mpHandler->getWorkingDirectory());

    //Copy local variables from main HCOM terminal
    mpTerminal->mpHandler->setLocalVariables(gpTerminalWidget->mpHandler->getLocalVariables());

    mpTerminal->mpHandler->setModelPtr(gpModelHandler->getCurrentModel());

    recreateCoreProgressBars();
    recreateParameterOutputLineEdits();

    if(!mpTerminal->mpHandler->mpOptHandler->isRunning())
    {
        this->mpTerminal->mpHandler->mpOptHandler->clearModels();
    }

    mpModelNameLabel->setText("Model file: "+mpSystem->getModelFileInfo().fileName());

    OptimizationSettings optSettings;
    mpSystem->getOptimizationSettings(optSettings);
    if(!optSettings.mScriptFile.isEmpty())
    {
        mScriptTextChanged = false; //Needed to avoid "do you want to save" dialog
        loadScriptFile(optSettings.mScriptFile);
    }

    QDialog::show();
}


//! @brief Closes the optimization dialog
void OptimizationDialog::close()
{
    if(!askForSavingScript())
    {
        return;
    }

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


//! @brief Disables output by optimization dialog
void OptimizationDialog::setOutputDisabled(bool disabled)
{
    mOutputDisabled = disabled;
}

//! @brief Saves the generated script code to file and executes the script
void OptimizationDialog::run()
{
    if(!askForSavingScript())
    {
        return;
    }

    mpTabWidget->setCurrentIndex(1);    //Go to run tab

    saveAs(gpDesktopHandler->getBackupPath()+"optimization_script"+QDateTime::currentDateTime().toString("_yyyy-MM-dd_hh_mm")+".hcom");

    mCoreProgressBarsRecreated = false;

    recreateParameterOutputLineEdits();

    mpRunButton->setDisabled(true);

    QStringList commands = mpScriptBox->toPlainText().split("\n");
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


//! @brief Generates a script skeleton for custom scripts
void OptimizationDialog::generateScriptSkeleton()
{
    if(!askForSavingScript())
    {
        return;
    }

    QFile skeletonFile(gpDesktopHandler->getExecPath()+"/../Scripts/HCOM/optSkeleton.hcom");
    skeletonFile.open(QFile::ReadOnly | QFile::Text);
    QString skeletonCode = skeletonFile.readAll();
    skeletonFile.close();

    this->setCode(skeletonCode);

    mScriptFileInfo.setFile(QFile());
    mScriptTextChanged = true;
    mSavedScriptText = QString();
    updateModificationStatus();
}


//! @brief Opens the optimization script wizard
void OptimizationDialog::openScriptWizard()
{
    if(!askForSavingScript())
    {
        return;
    }

    OptimizationScriptWizard *pWizard = new OptimizationScriptWizard(mpSystem, this);
    pWizard->setAttribute(Qt::WA_DeleteOnClose, true);
    pWizard->open();
}


//! @brief Saves generated script to path specified by user
void OptimizationDialog::save()
{
    if(mScriptFileInfo.fileName().isEmpty())
    {
        saveAs();
    }
    else
    {
        saveAs(mScriptFileInfo.absoluteFilePath());
    }
}


//! @brief Saves generated script to file
//! @param[in] filePath File path, empty means open file dialog
void OptimizationDialog::saveAs(QString filePath)
{
    if(mpScriptBox->toPlainText().isEmpty())
    {
        return;
    }

    else if(filePath.isEmpty())
    {
        filePath = QFileDialog::getSaveFileName(this, tr("Save Script File"),
                                                gpConfig->getStringSetting(CFG_SCRIPTDIR),
                                                this->tr("HCOM Script (*.hcom)"));

        if(filePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }
        mScriptFileInfo.setFile(filePath);  //Save file info for "save as"
        mpTabWidget->setTabText(0, QString("Script (%1)").arg(mScriptFileInfo.fileName()));
        mpScriptFileLabel->setText(QString("Script file: %1").arg(mScriptFileInfo.fileName()));
        gpConfig->setStringSetting(CFG_SCRIPTDIR, mScriptFileInfo.absolutePath());
    }

    QFile file(filePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&file);
    out << mpScriptBox->toPlainText();
    file.close();

    mSavedScriptText = mpScriptBox->toPlainText();
    mScriptTextChanged = false;
}


//! @brief Loads an optimization script file
void OptimizationDialog::loadScriptFile(QString filePath)
{
    if(!askForSavingScript())
    {
        return;
    }

    if(filePath.isEmpty())
    {
        filePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Load Script File)"),
                                                        gpConfig->getStringSetting(CFG_SCRIPTDIR),
                                                        tr("HCOM Script (*.hcom)"));
        if(filePath.isEmpty())      //Canceled by user
            return;
    }

    gpConfig->setStringSetting(CFG_SCRIPTDIR, QFileInfo(filePath).absolutePath());

    QFile file(filePath);
    file.open(QFile::Text | QFile::ReadOnly);
    QString script = file.readAll();
    file.close();

    setCode(script);

    mScriptFileInfo.setFile(file);
    mSavedScriptText = script;
    updateModificationStatus();
    mpScriptFileLabel->setText(QString("Script file: %1").arg(mScriptFileInfo.fileName()));

    OptimizationSettings optSettings;
    mpSystem->getOptimizationSettings(optSettings);
    optSettings.mScriptFile = mScriptFileInfo.absoluteFilePath();
    mpSystem->setOptimizationSettings(optSettings);
}


//! @brief Updates all model progres bars
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


//! @brief Recreates progress bars depending on number of models
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
    case Ops::ComplexRF :    //Complex-RF
        mCoreProgressBarPtrs.append(new QProgressBar(this));
        mpCoreProgressBarsLayout->addWidget(new QLabel("Current simulation:", this),0,0);
        mpCoreProgressBarsLayout->addWidget(mCoreProgressBarPtrs.last(),0,1);
        break;
    case Ops::ComplexRFP :    //Complex-RFP
    case Ops::ParticleSwarm :    //Particle swarm
    case Ops::DifferentialEvolution :    //Differential Evolution
    case Ops::ParameterSweep :    //Parameter sweep
    case Ops::Genetic:    //Genetic algorithm
        if(showProgressPerParticle)
        {
            int nModels = mpTerminal->mpHandler->mpOptHandler->getOptVar("nmodels");
            for(int n=0; n<nModels; ++n)
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

//! @brief Create or destroy parameter line edits to match number of optimization points
void OptimizationDialog::recreateParameterOutputLineEdits()
{
    int nPoints = mpTerminal->mpHandler->mpOptHandler->getOptVar("npoints");

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    while(mParametersOutputLineEditPtrs.size() < nPoints)
    {
        mParametersOutputLineEditPtrs.append(new QLineEdit(this));
        mParametersOutputLineEditPtrs.last()->setFont(font);
        mParametersApplyButtonPtrs.append(new QPushButton("Apply", this));
        mpParametersOutputTextEditsLayout->addWidget(mParametersApplyButtonPtrs.last(), mParametersOutputLineEditPtrs.size(), 0);
        mpParametersOutputTextEditsLayout->addWidget(mParametersOutputLineEditPtrs.last(), mParametersOutputLineEditPtrs.size(), 1);
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


//! @brief Slot for flagging that script text has changed
void OptimizationDialog::updateModificationStatus()
{
    mScriptTextChanged = (mpScriptBox->toPlainText() != mSavedScriptText);

    QString asterisk = (mScriptTextChanged ? "*" : "");
    if(mScriptFileInfo.fileName().isEmpty())
    {
        mpTabWidget->setTabText(0,
                                QString("Script%1")
                                .arg(asterisk));
    }
    else
    {
        mpTabWidget->setTabText(0,
                                QString("Script (%1)%2")
                                .arg(mScriptFileInfo.fileName())
                                .arg(asterisk));
    }
}


//! @brief Asks user whether or not to save script if it has changed
//! @return True if user selects "yes"/"no", false if user selects "cancel"
bool OptimizationDialog::askForSavingScript()
{
    if(mScriptTextChanged)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, mScriptFileInfo.fileName(),
                                      "Do you want to save changes to current script?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(reply == QMessageBox::Cancel)
        {
            return false;
        }
        if(reply == QMessageBox::Yes)
        {
            if(mScriptFileInfo.fileName().isEmpty())
            {
                saveAs();
            }
            else
            {
                saveAs();
            }
        }
    }
    return true;
}


//! @brief Creates the toolbar and its connections
QToolBar*OptimizationDialog::createToolBar()
{
    QToolBar *pToolBar = new QToolBar(this);
    QToolButton *pNewScriptButton = new QToolButton(this);
    pNewScriptButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-New.png"));
    pNewScriptButton->setText("New Script Skeleton");
    pNewScriptButton->setToolTip("New Script Skeleton");
    pToolBar->addWidget(pNewScriptButton);

    QToolButton *pScriptWizardButton = new QToolButton(this);
    pScriptWizardButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Wizard.png"));
    pScriptWizardButton->setText("Script Wizard");
    pScriptWizardButton->setToolTip("Script Wizard");
    pToolBar->addWidget(pScriptWizardButton);

    QToolButton *pLoadScriptButton = new QToolButton(this);
    pLoadScriptButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Open.png"));
    pLoadScriptButton->setText("Load Script File");
    pLoadScriptButton->setToolTip("Load Script File");
    pToolBar->addWidget(pLoadScriptButton);

    QToolButton *pSaveScriptButton = new QToolButton(this);
    pSaveScriptButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Save.png"));
    pSaveScriptButton->setText("Save Script File");
    pSaveScriptButton->setToolTip("Save Script File");
    pToolBar->addWidget(pSaveScriptButton);

    QToolButton *pSaveAsScriptButton = new QToolButton(this);
    pSaveAsScriptButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SaveAs.png"));
    pSaveAsScriptButton->setText("Save Script File As");
    pSaveAsScriptButton->setToolTip("Save Script File As");
    pToolBar->addWidget(pSaveAsScriptButton);


    mpRunButton = new QToolButton(this);
    mpRunButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Optimize.png"));
    mpRunButton->setText("Start Optimization");
    mpRunButton->setToolTip("Start Optimization");
    pToolBar->addWidget(mpRunButton);

    QWidget *pSpacerWidget = new QWidget(this);
    pSpacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    pSpacerWidget->setVisible(true);
    pToolBar->addWidget(pSpacerWidget);

    QToolButton *pHelpButton = new QToolButton(this);
    pHelpButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Help.png"));
    pHelpButton->setText("Show Context Help");
    pHelpButton->setToolTip("Show Context Help");
    pHelpButton->setObjectName("optimizationHelpButton");
    pToolBar->addWidget(pHelpButton);

    connect(pNewScriptButton,       SIGNAL(clicked(bool)), this,               SLOT(generateScriptSkeleton()));
    connect(pScriptWizardButton,    SIGNAL(clicked(bool)), this,               SLOT(openScriptWizard()));
    connect(pLoadScriptButton,      SIGNAL(clicked(bool)), this,               SLOT(loadScriptFile()));
    connect(pSaveScriptButton,      SIGNAL(clicked(bool)), this,               SLOT(save()));
    connect(pSaveAsScriptButton,    SIGNAL(clicked(bool)), this,               SLOT(saveAs()));
    connect(mpRunButton,            SIGNAL(clicked(bool)), this,               SLOT(run()));
    connect(pHelpButton,            SIGNAL(clicked()),     gpHelpPopupWidget,  SLOT(openContextHelp()));

    return pToolBar;
}


//! @brief Creates widgets and connections for "Script" tab
QWidget*OptimizationDialog::createScriptWidget()
{
    QWidget *pScriptWidget = new QWidget(this);

    QVBoxLayout *pScriptLayout = new QVBoxLayout(pScriptWidget);


    mpScriptBox = new QTextEdit(this);
    HcomHighlighter *pHighligter = new HcomHighlighter(mpScriptBox->document());
    Q_UNUSED(pHighligter);
    QFont monoFont = mpScriptBox->font();
    monoFont.setFamily("Courier");
    monoFont.setPointSize(11);
    mpScriptBox->setFont(monoFont);
    mpScriptBox->setMinimumWidth(450);
    pScriptLayout->addWidget(mpScriptBox);

    connect(mpScriptBox, SIGNAL(textChanged()), this, SLOT(updateModificationStatus()));
    return pScriptWidget;
}


//! @brief Creates widgets and connections for "Run" tab
QWidget*OptimizationDialog::createRunWidget()
{
    QWidget *pRunWidget = new QWidget(this);
    pRunWidget->setPalette(gpConfig->getPalette());
    QGridLayout *pRunLayout = new QGridLayout(pRunWidget);

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

    pRunLayout->addWidget(mpModelNameLabel,               0,0,1,1);
    pRunLayout->addWidget(mpScriptFileLabel,              1,0,1,1);
    pRunLayout->addWidget(pParametersOutputScrollArea,    3,0,1,2);
    pRunLayout->addWidget(mpTerminal,                           4,0,1,2);
    pRunLayout->setRowStretch(3,2.4);
    pRunLayout->setRowStretch(4,2.6);
    pRunLayout->setColumnStretch(0,1);
    pRunLayout->setColumnMinimumWidth(1,400);
    pRunLayout->addWidget(mpTotalProgressBar,           5,1,1,2);

    return pRunWidget;
}
