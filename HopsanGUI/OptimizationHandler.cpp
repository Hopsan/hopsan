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
//! @file   OptimizationHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-08-02
//! @version $Id$
//!
//! @brief Contains a handler for optimizations
//!

//HopsanGUI includes
#include "Configuration.h"
#include "HcomHandler.h"
#include "OptimizationHandler.h"
#include "Dialogs/OptimizationDialog.h"
#include "OpsWorkerParameterSweep.h"
#include "OpsWorkerComplexRF.h"
#include "OpsWorkerComplexRFP.h"
#include "OpsWorkerNelderMead.h"
#include "OpsWorkerParticleSwarm.h"
#include "OpsWorkerDifferentialEvolution.h"
#include "OpsWorkerControlledRandomSearch.h"
#include "OpsWorkerComplexBurmen.h"
#include "OpsWorkerGenetic.h"
#include "MessageHandler.h"
#include "ModelHandler.h"
#include "Widgets/ModelWidget.h"
#include "global.h"
#include "PlotHandler.h"
#include "PlotWindow.h"
#include "PlotArea.h"
#include "PlotCurve.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "DesktopHandler.h"
#include "Widgets/HcomWidget.h"
#include "LogVariable.h"

#ifdef USEZMQ
#include "RemoteSimulationUtils.h"
#endif

//! @brief Constructor for optimization  handler class
OptimizationHandler::OptimizationHandler(HcomHandler *pHandler)
{
    mpHcomHandler = pHandler;
    mpMessageHandler = new GUIMessageHandler(this);
    mpMessageHandler->startPublish();
    mpConfig = new Configuration(); //!< @todo memory leak, never deleted
    mpConfig->loadFromXml();        //This should work, since changes are always saved to file immediately from gpConfig

    mpWorker = 0;
    mIsRunning = false;

    mPlotPoints = false;

    mpEvaluator = new OptimizationEvaluator(this);
    mpOpsMessageHandler = new OptimizationMessageHandler(this);

    mPrintLogFile = false;
    mPrintResultFile = true;
    mPrintDebugFile = false;
}

void OptimizationHandler::startOptimization(ModelWidget *pModel, QString &modelPath)
{
    if(mpWorker)
    {
        if(mPlotPoints) {
            PlotWindow *pParPlotWindow = gpPlotHandler->getPlotWindow("parplot");
            if(pParPlotWindow) {
                pParPlotWindow->closeAllTabs();
            }
        }

        clearPlotVariables();

        mModelPath = modelPath;

        mEvaluations = 0;
        mCurrentProgressBarPercent=0;
        mLoggedParameters.clear();
//        connect(mpWorker, SIGNAL(stepCompleted(int)), this, SLOT(updateProgressBar(int)));
        //gpOptimizationDialog->setOutputDisabled(true);

        connect(mpHcomHandler, SIGNAL(aborted()), mpOpsMessageHandler, SLOT(abort()));

//        connect(mpWorker, SIGNAL(objectiveChanged(int)),    this,               SLOT(updateOutputs()));
//        connect(mpWorker, SIGNAL(objectivesChanged()),      this,               SLOT(updateOutputs()));
//        connect(mpWorker, SIGNAL(pointChanged(int)),        this,               SLOT(updateOutputs()));
//        connect(mpWorker, SIGNAL(pointsChanged()),          this,               SLOT(updateOutputs()));
//        connect(mpWorker, SIGNAL(message(QString)),         mpMessageHandler,   SLOT(addInfoMessage(QString)));
//        connect(mpWorker, SIGNAL(pointsChanged()),          this,               SLOT(plotPoints()));
//        connect(mpWorker, SIGNAL(pointsChanged()),          this,               SLOT(plotParameters()));
//        connect(mpWorker, SIGNAL(pointsChanged()),          this,               SLOT(plotEntropy()));
//        connect(mpWorker, SIGNAL(pointChanged(int)),        this,               SLOT(plotPoints()));
//        connect(mpWorker, SIGNAL(pointChanged(int)),        this,               SLOT(plotParameters()));
//        connect(mpWorker, SIGNAL(pointChanged(int)),        this,               SLOT(plotEntropy()));
//        connect(mpWorker, SIGNAL(candidateChanged(int)),    this,               SLOT(plotPoints()));
//        connect(mpWorker, SIGNAL(candidatesChanged()),      this,               SLOT(plotPoints()));
//        connect(mpWorker, SIGNAL(objectivesChanged()),      this,               SLOT(plotObjectiveValues()));
//        connect(mpWorker, SIGNAL(objectiveChanged(int)),    this,               SLOT(plotObjectiveValues()));
//        connect(mpWorker, SIGNAL(pointChanged(int)),        this,               SLOT(logPoint(int)));
//        connect(mpWorker, SIGNAL(pointsChanged()),          this,               SLOT(logAllPoints()));
//        connect(mpWorker, SIGNAL(pointChanged(int)),        this,               SLOT(logPoint(int)));
//        connect(mpWorker, SIGNAL(stepCompleted(int)),       this,               SLOT(checkIfRescheduleIsNeeded()));

        mOrgSetPwdToMwdSetting = gpConfig->getBoolSetting(cfg::setpwdtomwd);
        mOrgProgressBarSetting = gpConfig->getBoolSetting(cfg::progressbar);
        mOrgLimitDataGenerationsSetting = gpConfig->getBoolSetting(cfg::autolimitgenerations);
        gpConfig->setBoolSetting(cfg::setpwdtomwd, false);
        gpConfig->setBoolSetting(cfg::progressbar, false);
        gpConfig->setBoolSetting(cfg::autolimitgenerations, true);

        int nModels = mpWorker->getNumberOfCandidates();
        this->initModels(pModel, nModels, modelPath);


#ifdef USEZMQ
        // Setup parallel server queues
        if (gpConfig->getBoolSetting(cfg::useremoteoptimization))
        {
            int pm, pa; double su;
            rescheduleForBestSpeedup(pm,pa,su,true);
            mpRemoteSimulationQueueHandler->setupModelQueues(mModelPtrs.mid(0, mpWorker->getNumberOfCandidates()), pm);
        }
#endif

        mStartTime = QDateTime::currentDateTime();

        mpWorker->initialize();
        mpWorker->run();

        mpMessageHandler->addInfoMessage("Optimization finished!");
        gpOptimizationDialog->updateTotalProgressBar(/*mpWorker->getMaxNumberOfIterations()*/100);
        this->setIsRunning(false);
        if(mDisconnectedFromModelHandler)
        {
            connect(gpModelHandler, SIGNAL(modelChanged(ModelWidget*)), mpHcomHandler, SLOT(setModelPtr(ModelWidget*)));
        }
        mpHcomHandler->mpConsole->mpTerminal->setAbortButtonEnabled(false);
        gpOptimizationDialog->setOptimizationFinished();
        mpHcomHandler->setModelPtr(gpModelHandler->getCurrentModel());

        printResultFile();
        printLogFile();
        printDebugFile();

        gpConfig->setBoolSetting(cfg::setpwdtomwd, mOrgSetPwdToMwdSetting);
        gpConfig->setBoolSetting(cfg::progressbar, mOrgProgressBarSetting);
        gpConfig->setBoolSetting(cfg::autolimitgenerations, mOrgLimitDataGenerationsSetting);
    }
    else
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
    }

#ifdef USEZMQ
    // Clear and disconnect from parallel server queues
    if (gpConfig->getBoolSetting(cfg::useremoteoptimization))
    {
        mpRemoteSimulationQueueHandler->clear();
    }
#endif
}

void OptimizationHandler::initModels(ModelWidget *pModel, int nModels, QString &modelPath)
{
    QString originalModelBasePath = pModel->getTopLevelSystemContainer()->getAppearanceData()->getBasePath();
    mModelPtrs.clear();
    while(mModelPtrs.size() < nModels)
    {
        auto pNewModel = gpModelHandler->loadModel(modelPath, ModelHandler::IgnoreAlreadyOpen | ModelHandler::Detatched);
        // Add base path from original model as search path, for components that load files with relative paths
        pNewModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->addSearchPath(originalModelBasePath);
        addModel(pNewModel);

        // Make sure logging is disabled/enabled for same ports as in original model
        //! @todo This code only deals with top-level components and ports and ignores contents of subsystems /Peter
        CoreSystemAccess *pCore = pModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr();
        for(const QString &compName : pModel->getTopLevelSystemContainer()->getModelObjectNames()) {
            for(const Port *port : pModel->getTopLevelSystemContainer()->getModelObject(compName)->getPortListPtrs()) {
                QString portName = port->getName();
                if (portName.contains(' '))
                {
                    gpMessageHandler->addWarningMessage("portname has invalid space: "+portName, "bug");
                }
                bool enabled = pCore->isLoggingEnabled(compName, portName);
                SystemObject *pOptSystem = mModelPtrs.last()->getTopLevelSystemContainer();
                CoreSystemAccess *pOptCore = pOptSystem->getCoreSystemAccessPtr();
                pOptCore->setLoggingEnabled(compName, portName, enabled);
            }
        }
    }

    mDisconnectedFromModelHandler = disconnect(gpModelHandler, SIGNAL(modelChanged(ModelWidget*)), mpHcomHandler, SLOT(setModelPtr(ModelWidget*)));

    //Clear previous data in case models were re-used
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        mpHcomHandler->setModelPtr(mModelPtrs[i]);
        mpHcomHandler->executeCommand("rmvar *");
    }
    mpHcomHandler->setModelPtr(mModelPtrs.first());

    //Load default optimization functions
    QString oldPath = mpHcomHandler->getWorkingDirectory();
    mpHcomHandler->setWorkingDirectory(gpDesktopHandler->getExecPath());
    //executeCommand("exec ../Scripts/HCOM/optDefaultFunctions.hcom");
    QFile testFile1(gpDesktopHandler->getScriptsPath()+"HCOM/optDefaultFunctions.hcom");
    QFile testFile2(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optDefaultFunctions.hcom");
    if(testFile1.exists())
    {
        mpHcomHandler->executeCommand("exec \""+testFile1.fileName()+"\"");
    }
    else if(testFile2.exists())
    {
        mpHcomHandler->executeCommand("exec \""+testFile2.fileName()+"\"");
    }
    else
    {
        QString msg = "Cannot find optimization default functions script file.";
        getMessageHandler()->addErrorMessage(msg, "", false);
        return;
    }
    mpHcomHandler->setWorkingDirectory(oldPath);

    setIsRunning(true);
}

void OptimizationHandler::setCandidateObjectiveValue(int idx, double value)
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setCandidateObjectiveValue(idx, value);
}

void OptimizationHandler::setParameterLimits(int idx, double min, double max)
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setParameterLimits(idx, min, max);
}


//! @brief Returns objective value with specified index
double OptimizationHandler::getObjectiveValue(int idx)
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return mpWorker->getObjectiveValue(idx);
}

double OptimizationHandler::getOptVar(const QString &var)
{
    bool ok;
    return getOptVar(var, ok);
}

double OptimizationHandler::getOptVar(const QString &var, bool &ok) const
{
    ok = true;
    if(var == "algorithm")
    {
        if(!mpWorker) return 0;
        return (int)mpWorker->getAlgorithm();
    }
    else if(var == "nparams")
    {
        if(!mpWorker) return 0;
        return mpWorker->getNumberOfParameters();
    }
    else if(var == "npoints")
    {
        if(!mpWorker) return 0;
        return mpWorker->getNumberOfPoints();
    }
    else if(var == "nmodels")
    {
        return mpWorker->getNumberOfCandidates();
    }
    else if(var == "evalid")
    {
        return mEvalId;
    }

    ok = false;

    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return 0;
}


//! @todo "ok" parameter should be used
void OptimizationHandler::setOptVar(const QString &var, const QString &value, bool &ok)
{
    Q_UNUSED(ok);

    if(var == "printlogfile" || var == "log")   //Use both for backwards compatibility
    {
        if(var == "log") mpMessageHandler->addWarningMessage("\"opt log\" command is deprecated. Use \"opt printlogfile\" instead.");
        mPrintLogFile = (value == "on");
    }
    else if(var == "printresultfile")
    {
        mPrintResultFile = (value == "on");
    }
    else if(var == "printdebugfile")
    {
        mPrintDebugFile = (value == "on");
    }
    else if(var == "algorithm")
    {
        if(value == "neldermead")
        {
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerNelderMead(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "complexrf" || value == "complex") //Use both for backwards compatibility
        {
            if(value == "complex") mpMessageHandler->addWarningMessage("Algorithm \"complex\" is deprecated. Use \"complexrf\" instead.");
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerComplexRF(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "complexrfp")
        {
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerComplexRFP(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "pso" || value == "particleswarm") //Use both for backwards compatibility
        {
            if(value == "particleswarm") mpMessageHandler->addWarningMessage("Algorithm \"particleswarm\" is deprecated. Use \"pso\" instead.");
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerParticleSwarm(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "parametersweep")
        {
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerParameterSweep(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "differentialevolution")
        {
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerDifferentialEvolution(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "controlledrandomsearch")
        {
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerControlledRandomSearch(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "complexburmen")
        {
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerComplexBurmen(mpEvaluator, mpOpsMessageHandler);
        }
        else if(value == "ga" || value == "genetic")
        {
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new Ops::WorkerGenetic(mpEvaluator, mpOpsMessageHandler);
        }
        return;
    }
    else if(var == "evalid")
    {
        mEvalId = value.toInt();
    }
    else if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No algorithm selected.");
        return;
    }
    else if(var == "nmodels")
    {
        mpWorker->setNumberOfCandidates(value.toDouble());
    }
    else if(var == "plotpoints")
    {
        mPlotPoints = (value == "on");
    }
    else if(var == "plotparameters")
    {
        mPlotParameters = (value == "on");
    }
    else if(var == "plotobjectives" || var == "plotbestworst")
    {
        mPlotObjectiveValues = (value == "on");
    }
    else if(var == "plotentropy")
    {
        mPlotEntropy = (value == "on");
    }
    else if(var == "npoints")
    {
        mpWorker->setNumberOfPoints(value.toInt());
    }
    else if(var == "nparams")
    {
        mpWorker->setNumberOfParameters(value.toInt());
    }
    else if(var == "parnames")
    {
        std::vector<const char*> cNames;
        QStringList names = value.split(",");
        foreach (QString name, names)
        {
            cNames.push_back(name.toUtf8());
        }
        mpWorker->setParameterNames(cNames);
    }
    else if(var == "partol")
    {
        mpWorker->setTolerance(value.toDouble());
    }
    else if(var == "maxevals")
    {
        mpWorker->setMaxNumberOfIterations(value.toInt());
    }
    else if(var == "sampling")
    {
        if(value == "random")
        {
            mpWorker->setSamplingMethod(Ops::SamplingRandom);
        }
        else if(value == "latinhypercube")
        {
            mpWorker->setSamplingMethod(Ops::SamplingLatinHypercube);
        }
    }
    else if(var == "surrogatemodel")
    {
        mpWorker->setUseSurrogateModel(value.toUInt());
    }

    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    if(mpWorker->getAlgorithm() == Ops::ComplexRF || mpWorker->getAlgorithm() == Ops::ComplexRFP || mpWorker->getAlgorithm() == Ops::ComplexBurmen)
    {
        Ops::WorkerComplexRF *pWorker = dynamic_cast<Ops::WorkerComplexRF*>(mpWorker);
        if(var == "alpha")
        {
            pWorker->setReflectionFactor(value.toDouble());
        }
        else if(var == "beta" || var == "rfak") //Use both for backwards compatibility
        {
            if(var == "rfak") mpMessageHandler->addWarningMessage("\"opt set rfak\" is deprecated. Use \"opt set beta\" instead.");
            pWorker->setRandomFactor(value.toDouble());
        }
        else if(var == "gamma")
        {
            pWorker->setForgettingFactor(value.toDouble());
        }
    }
    if(mpWorker->getAlgorithm() == Ops::ComplexRFP)
    {
        Ops::WorkerComplexRFP *pWorker = dynamic_cast<Ops::WorkerComplexRFP*>(mpWorker);
        if(var == "parallelmethod")
        {
            if(value == "taskprediction")
            {
                pWorker->setParallelMethod(Ops::TaskPrediction);
            }
            else if(value == "multidirection")
            {
                pWorker->setParallelMethod(Ops::MultiDirection);
            }
            else if(value == "multidistance")
            {
                pWorker->setParallelMethod(Ops::MultiDistance);
            }
        }
        else if(var == "npredictions")
        {
            pWorker->setNumberOfPredictions(value.toInt());
        }
        else if(var == "nretractions")
        {
            pWorker->setNumberOfRetractions(value.toInt());
        }
        else if(var == "alphamin")
        {
            pWorker->setMinimumReflectionFactor(value.toDouble());
        }
        else if(var == "alphamax")
        {
            pWorker->setMaximumReflectionFactor(value.toDouble());
        }
    }
    else if(mpWorker->getAlgorithm() == Ops::NelderMead)
    {
        Ops::WorkerNelderMead *pWorker = dynamic_cast<Ops::WorkerNelderMead*>(mpWorker);
        if(var == "alpha")
        {
            pWorker->setReflectionFactor(value.toDouble());
        }
        else if(var == "gamma")
        {
            pWorker->setExpansionFactor(value.toDouble());
        }
        else if(var == "rho")
        {
            pWorker->setContractionFactor(value.toDouble());
        }
        else if(var == "sigma")
        {
            pWorker->setReductionFactor(value.toDouble());
        }
    }
    else if(mpWorker->getAlgorithm() == Ops::ParticleSwarm)
    {
        Ops::WorkerParticleSwarm *pWorker = dynamic_cast<Ops::WorkerParticleSwarm*>(mpWorker);
        if(var == "omega1")
        {
            pWorker->setOmega1(value.toDouble());
        }
        else if(var == "omega2")
        {
            pWorker->setOmega2(value.toDouble());
        }
        else if(var == "c1")
        {
            pWorker->setC1(value.toDouble());
        }
        else if(var == "c2")
        {
            pWorker->setC2(value.toDouble());
        }
        else if(var == "vmax")
        {
            pWorker->setVmax(value.toDouble());
        }
        else if(var == "inertiastrategy")
        {
            if(value == "constant")
            {
                pWorker->setInertiaStrategy(Ops::InertiaConstant);
            }
            else if(value == "lineardecreasing")
            {
                pWorker->setInertiaStrategy(Ops::InertiaLinearDecreasing);
            }
        }
    }
    else if(mpWorker->getAlgorithm() == Ops::DifferentialEvolution)
    {
        Ops::WorkerDifferentialEvolution *pWorker = dynamic_cast<Ops::WorkerDifferentialEvolution*>(mpWorker);
        if(var == "F")
        {
            pWorker->setDifferentialWeight(value.toDouble());
        }
        else if(var == "CR")
        {
            pWorker->setCrossoverProbability(value.toDouble());
        }
    }
    else if(mpWorker->getAlgorithm() == Ops::Genetic)
    {
        Ops::WorkerGenetic *pWorker = dynamic_cast<Ops::WorkerGenetic*>(mpWorker);
        if(var == "CP")
        {
           pWorker->setCrossoverProbability(value.toDouble());
        }
        else if(var == "MP")
        {
            pWorker->setMutationProbability(value.toDouble());
        }
        else if(var == "elites")
        {
            pWorker->setNumberOfElites(value.toInt());
        }
    }
}

double OptimizationHandler::getCandidateParameter(const int pointIdx, const int parIdx) const
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return mpWorker->getCandidateParameter(pointIdx, parIdx);
}

double OptimizationHandler::getParameter(const int pointIdx, const int parIdx) const
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return mpWorker->getParameter(pointIdx, parIdx);
}


void OptimizationHandler::setIsRunning(bool value)
{
    mIsRunning = value;
}

bool OptimizationHandler::isRunning()
{
    return mIsRunning;
}

QStringList OptimizationHandler::getOptParNamesPtr()
{
    QStringList ret;
    std::vector<const char*> cNames;
    for(const char* &name : cNames)
    {
        ret.append(QString(name));
    }
    return ret;
}

void OptimizationHandler::setStartValue(const int pointIdx, const int parIdx, const double value)
{
    mpWorker->setParameter(pointIdx, parIdx, value);
    mpWorker->setIgnoreParameterWhenSampling(pointIdx, parIdx);
}

bool OptimizationHandler::evaluateCandidate(int idx)
{
    mEvalId = idx;
    mpHcomHandler->setModelPtr(mModelPtrs.at(idx));
    mpHcomHandler->executeCommand("opt set evalid "+QString::number(idx));

    if(mpHcomHandler->hasFunction("evalexternal")) {
        mpHcomHandler->executeCommand("call evalexternal");
    }
    else {
        mpHcomHandler->executeCommand("call setpars");
        bool simOK=false;
        simOK = mModelPtrs.at(idx)->simulate_blocking();

        if (!simOK)
        {
            return false;
        }
    }

    mpHcomHandler->executeCommand("opt set evalid "+QString::number(idx));
    mpHcomHandler->executeCommand("call obj");

    mpHcomHandler->setModelPtr(mModelPtrs.at(0));

    ++mEvaluations;

    return true;
}

bool OptimizationHandler::evaluateAllCandidates()
{
    mNeedsRescheduling = false;

    //Multi-threading, we cannot use the "evalall" function
    for(size_t i=0; i<mpWorker->getNumberOfCandidates() && !mpWorker->aborted(); ++i)
    {
        mpHcomHandler->setModelPtr(mModelPtrs[i]);
        mpHcomHandler->executeCommand("opt set evalid "+QString::number(i));
        mpHcomHandler->executeCommand("call setpars");
    }

    bool simOK=false;
#ifdef USEZMQ
    if (gpConfig->getBoolSetting(cfg::useremoteoptimization))
    {
        if (mpRemoteSimulationQueueHandler && mpRemoteSimulationQueueHandler->hasServers())
        {
            simOK = mpRemoteSimulationQueueHandler->simulateModels(mNeedsRescheduling);
        }
    }
    else
    {
        simOK = gpModelHandler->simulateMultipleModels_blocking(mModelPtrs.mid(0,mpWorker->getNumberOfCandidates()));
    }
#else
    //! @note The "mid()" function are used to make sure that the number of models simulated equals number of candidates (in case more models are opened)
    simOK = gpModelHandler->simulateMultipleModels_blocking(mModelPtrs.mid(0,mpWorker->getNumberOfCandidates())/*, !firstTime*/);
#endif
    if (!simOK)
    {
        return false;
    }

    for(size_t i=0; i<mpWorker->getNumberOfCandidates(); ++i)
    {
        mpHcomHandler->setModelPtr(mModelPtrs[i]);
        mpHcomHandler->executeCommand("opt set evalid "+QString::number(i));
        mpHcomHandler->executeCommand("call obj");
    }
    mpHcomHandler->setModelPtr(mModelPtrs.at(0));

    mEvaluations += mpWorker->getNumberOfCandidates();

    return true;
}


void OptimizationHandler::plotPoints(PointPlotContentT content)
{
    if(!mPlotPoints || mpWorker->getNumberOfParameters() < 2)
    {
        return;
    }

    for(size_t p=0; p<mpWorker->getNumberOfPoints() && (content == AllPoints); ++p)
    {

        double x = mpWorker->getParameter(p,0);
        double y = mpWorker->getParameter(p,1);

        if(mPointVars_x.size() <= (int)p)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            mPointVars_x.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));
            mPointVars_y.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));

            auto varDesc_x = mPointVars_x.last()->getVariableDescription();
            varDesc_x->mCustomLabel = QString("x[%1]").arg(p);
            varDesc_x->mDataQuantity = "parameter 0";
            varDesc_x->mDataUnit = "-";

            auto varDesc_y = mPointVars_y.last()->getVariableDescription();
            varDesc_y->mCustomLabel = QString("x[%1]").arg(p);
            varDesc_y->mDataQuantity = "parameter 1";
            varDesc_y->mDataUnit = "-";

            mPointVars_x.last()->assignFrom(x);
            mPointVars_y.last()->assignFrom(y);

            double min0, max0, min1, max1;
            mpWorker->getParameterLimits(0, min0, max0);
            mpWorker->getParameterLimits(1, min1, max1);

            gpPlotHandler->plotDataToWindow("parplot", mPointVars_x.last(), mPointVars_y.last(), 0, QColor("blue"));
            PlotArea *pPlotArea = gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea();
            pPlotArea->setAxisLimits(QwtPlot::xBottom, min0, max0);
            pPlotArea->setAxisLimits(QwtPlot::yLeft, min1, max1);
            pPlotArea->setAxisLabel(QwtPlot::xBottom, "Optimization Parameter 0");
            pPlotArea->setAxisLabel(QwtPlot::yLeft, "Optimization Parameter 1");
            pPlotArea->getCurves().last()->setLineSymbol("XCross");
        }
        else
        {
            //! @todo need to turn of auto refresh on plot and trigger it manually to avoid multiple redraws here
            mPointVars_x.at(p)->assignFrom(x);
            mPointVars_y.at(p)->assignFrom(y);
        }
    }

    for(size_t p=0; p<mpWorker->getNumberOfCandidates(); ++p)
    {

        double x = mpWorker->getCandidateParameter(p,0);
        double y = mpWorker->getCandidateParameter(p,1);

        if(mPointVars_x.size() <= (int)mpWorker->getNumberOfPoints()+(int)p)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            mPointVars_x.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));
            mPointVars_y.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));

            auto varDesc_x = mPointVars_x.last()->getVariableDescription();
            varDesc_x->mCustomLabel = QString("c[%1]").arg(p);
            varDesc_x->mDataQuantity = "parameter 0";
            varDesc_x->mDataUnit = "-";

            auto varDesc_y = mPointVars_y.last()->getVariableDescription();
            varDesc_y->mCustomLabel = QString("c[%1]").arg(p);
            varDesc_y->mDataQuantity = "parameter 1";
            varDesc_y->mDataUnit = "-";

            mPointVars_x.last()->assignFrom(x);
            mPointVars_y.last()->assignFrom(y);

            double min0, max0, min1, max1;
            mpWorker->getParameterLimits(0, min0, max0);
            mpWorker->getParameterLimits(1, min1, max1);

            auto pPlotWindow = gpPlotHandler->plotDataToWindow("parplot", mPointVars_x.last(), mPointVars_y.last(), 0, QColor("black"));
            auto pPlotArea = gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea();
            pPlotArea->setAxisLimits(QwtPlot::xBottom, min0, max0);
            pPlotArea->setAxisLimits(QwtPlot::yLeft, min1, max1);
            pPlotArea->setAxisLabel(QwtPlot::xBottom, "parameter 0");
            pPlotArea->setAxisLabel(QwtPlot::yLeft, "parameter 1");
            pPlotArea->setLegendsVisible(false);
            for(auto pPlotCurve : pPlotWindow->getCurrentPlotTab()->getCurves())
            {
                pPlotCurve->setIncludeGenerationInTitle(false);
                pPlotCurve->refreshCurveTitle();
                pPlotCurve->setLineSymbol("XCross");
            }
        }
        else
        {
            //! @todo need to turn of auto refresh on plot and trigger it manually to avoid multiple redraws here
            mPointVars_x.at(mpWorker->getNumberOfPoints()+p)->assignFrom(x);
            mPointVars_y.at(mpWorker->getNumberOfPoints()+p)->assignFrom(y);
        }
    }


    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow("parplot");
    if(pPlotWindow)
    {
        PlotTab *pTab = pPlotWindow->getCurrentPlotTab();
        for(int c=0; c<mpWorker->getNumberOfPoints(); ++c)
        {
            if(c == (int)mpWorker->getBestId())
            {
                pTab->getCurves(0).at(c)->setLineColor(QColor("green"));
            }
            else if(c == (int)mpWorker->getWorstId())
            {
                pTab->getCurves(0).at(c)->setLineColor(QColor("red"));
            }
            else
            {
                pTab->getCurves(0).at(c)->setLineColor(QColor("blue"));
            }
        }
        pTab->update();
    }
}

//! @brief Plots parameter values (if option is selected)
void OptimizationHandler::plotParameters()
{
    if(!mPlotParameters) { return; }

    for(int p=0; p<(int)mpWorker->getNumberOfParameters(); ++p)
    {
        if(mParVars.size() <= p)
        {
            mParVars.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));
            mParVars.last()->assignFrom(mpWorker->getParameter(mpWorker->getWorstId(),p));
            auto varDesc = mParVars.last()->getVariableDescription();
            varDesc->mCustomLabel = QString("parameter %1").arg(p+1);
            varDesc->mDataQuantity = "parameter value";
            varDesc->mDataUnit = "-";
        }
        else
        {
            mParVars.at(p)->append(mpWorker->getParameter(mpWorker->getWorstId(),p));
        }

        // If this is the first time, then recreate the plotwindows
        // Note! plots will auto update when new data is appended, so there is no need to call plotab->update()
        if(mParVars.at(p)->getDataSize() == 1)
        {
            PlotWindow *pPW = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("ParameterValues");
            gpPlotHandler->plotDataToWindow(pPW, mParVars.at(p), 0, true);
            auto pPlotCurve = pPW->getCurrentPlotTab()->getCurves().last();
            pPlotCurve->setIncludeGenerationInTitle(false);
            pPlotCurve->refreshCurveTitle();
            auto pPlotArea = pPW->getCurrentPlotTab()->getPlotArea();
            pPlotArea->setAxisLabel(QwtPlot::xBottom, "evaluations");
        }
    }
}

void OptimizationHandler::plotObjectiveValues()
{
    if(!mPlotObjectiveValues) { return; }

    double best = mpWorker->getObjectiveValue(mpWorker->getBestId());
    double worst = mpWorker->getObjectiveValue(mpWorker->getWorstId());
    double lastworst = mpWorker->getObjectiveValue(mpWorker->getLastWorstId());

    //Best objective value
    if(mBestVar.isNull())
    {
        mBestVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mBestVar->assignFrom(best);
        auto varDesc = mBestVar->getVariableDescription();
        varDesc->mCustomLabel = "best";
        varDesc->mDataQuantity = "fitness";
        varDesc->mDataUnit = "-";
    }
    else
    {
        mBestVar->append(best);
    }

    //Worst objective value
    if(mWorstVar.isNull())
    {
        mWorstVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mWorstVar->assignFrom(worst);
        auto varDesc = mWorstVar->getVariableDescription();
        varDesc->mCustomLabel = "worst";
        varDesc->mDataQuantity = "fitness";
        varDesc->mDataUnit = "-";
    }
    else
    {
        mWorstVar->append(worst);
    }

    //Newest objective value
    if(mNewestVar.isNull())
    {
        mNewestVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mNewestVar->assignFrom(lastworst);
        auto varDesc = mNewestVar->getVariableDescription();
        varDesc->mCustomLabel = "latest";
        varDesc->mDataQuantity = "fitness";
        varDesc->mDataUnit = "-";
    }
    else
    {
        mNewestVar->append(lastworst);
    }

    // If this is the first time, then recreate the plotwindows
    // Note! plots will auto update when new data is appended, so there is no need to call plotab->update()
    if(mBestVar->getDataSize() == 1)
    {
        PlotWindow *pPW = gpPlotHandler->createNewOrReplacePlotwindow("ObjectiveFunction");
        gpPlotHandler->plotDataToWindow(pPW, mBestVar, 0, true, QColor("Green"));
        gpPlotHandler->plotDataToWindow(pPW, mWorstVar, 0, true, QColor("Red"));
        gpPlotHandler->plotDataToWindow(pPW, mNewestVar, 0, true, QColor("Orange"));

        for(auto pPlotCurve : pPW->getCurrentPlotTab()->getCurves())
        {
            pPlotCurve->setIncludeGenerationInTitle(false);
            pPlotCurve->refreshCurveTitle();
        }
        auto pPlotArea = pPW->getCurrentPlotTab()->getPlotArea();
        pPlotArea->setAxisLabel(QwtPlot::xBottom, "evaluations");
    }
}

void OptimizationHandler::plotEntropy()
{
    if(!mPlotEntropy) { return; }

    double deltaX = mpWorker->getMaxPercentalParameterDiff();
    int n = mpWorker->getNumberOfParameters();
    double entropy = -n*log2(deltaX);

    if(mEntropyVar.isNull())
    {
        mEntropy.clear();
        mEntropy.append(entropy);
        mEntropyVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mEntropyVar->assignFrom(entropy);
        auto varDesc = mEntropyVar->getVariableDescription();
        varDesc->mCustomLabel = "entropy";
        varDesc->mDataQuantity = "entropy";
        varDesc->mDataUnit = "bits";
    }
    else
    {
        mEntropy.append(entropy);
        mEntropyVar->append(entropy);
    }

    // If this is the first time, then recreate the plotwindows
    if(mEntropyVar.data()->getDataSize() == 1)
    {
        auto pPlotWindow = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("OptimizationEntropy");
        gpPlotHandler->plotDataToWindow(pPlotWindow, mEntropyVar, 0, true);
        auto pPlotCurve = pPlotWindow->getCurrentPlotTab()->getCurves().last();
        pPlotCurve->setIncludeGenerationInTitle(false);
        pPlotCurve->refreshCurveTitle();
        auto pPlotArea = pPlotWindow->getCurrentPlotTab()->getPlotArea();
        pPlotArea->setAxisLabel(QwtPlot::xBottom, "evaluations");
    }
}

void OptimizationHandler::updateProgressBar(int i)
{
    int maxEvals = mpWorker->getMaxNumberOfIterations();
    int dummy=int(100.0*double(i)/maxEvals);

    if(dummy != mCurrentProgressBarPercent)    //Only update at whole numbers
    {
        mCurrentProgressBarPercent = dummy;
        gpOptimizationDialog->updateTotalProgressBar(dummy);
    }
    qApp->processEvents();
}

void OptimizationHandler::updateOutputs()
{
    gpOptimizationDialog->updateParameterOutputs(mpWorker->getObjectiveValues(), mpWorker->getPoints(),
                                                 mpWorker->getBestId(), mpWorker->getWorstId());
    qApp->processEvents();
}


const QVector<ModelWidget *> *OptimizationHandler::getModelPtrs() const
{
    if(mpWorker)
    {
        return (&mModelPtrs);
    }

    //! @todo this is a memory leak
    return new QVector<ModelWidget *>();
}

void OptimizationHandler::clearModels()
{
    mpHcomHandler->setModelPtr(gpModelHandler->getCurrentModel());
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        //mModelPtrs[i]->mpParentModelHandler->closeModel(mModelPtrs[i], true);
        delete mModelPtrs[i];
        //! @todo we delete her since the models are detached, but it would be better if we could use the closeModel function and let the handler close and delete the models
    }
    mModelPtrs.clear();
}


void OptimizationHandler::addModel(ModelWidget *pModel)
{
    if (mpWorker)
    {
        mModelPtrs.append(pModel);
        pModel->setMessageHandler(mpMessageHandler);
    }
    else
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
    }
}


//! @brief Clears the contents from all optimization plot variables
void OptimizationHandler::clearPlotVariables()
{
    mEntropyVar.clear();
    mBestVar.clear();
    mWorstVar.clear();
    mNewestVar.clear();
    for(auto &var : mParVars)
        var.clear();
    for(auto &var : mPointVars_x)
        var.clear();
    for(auto &var : mPointVars_y)
        var.clear();
    mParVars.clear();
    mPointVars_x.clear();
    mPointVars_y.clear();
}

Ops::AlgorithmT OptimizationHandler::getAlgorithm() const
{
    if(mpWorker)
    {
        return mpWorker->getAlgorithm();
    }
    else
    {
        return Ops::Undefined;
    }
}

GUIMessageHandler *OptimizationHandler::getMessageHandler()
{
    return mpMessageHandler;
}




OptimizationEvaluator::OptimizationEvaluator(OptimizationHandler *pHandler)
{
    mpHandler = pHandler;
}

void OptimizationEvaluator::evaluateCandidate(size_t idx)
{
    mpHandler->evaluateCandidate(idx);
}

void OptimizationEvaluator::evaluateAllCandidates()
{
    mpHandler->evaluateAllCandidates();
}





void OptimizationHandler::printResultFile()
{
    if(!mPrintResultFile) return;

    QString htmlCode;

    //Header
    htmlCode.append("<html>\n<head>\n<title>Hopsan Optimization Log</title>\n</head>\n<body>\n\n");

    //CSS style
    htmlCode.append("<style type=\"text/css\">\n  td {\n    width: 170pt\n  }\n</style>\n\n");

    //Title
    htmlCode.append("<h1>Hopsan Optimization Log</h1>\n");

    //Begin general information
    htmlCode.append("<h3>General Information:</h3>\n<table>\n");

    //Start time
    htmlCode.append("<tr>\n<td><b>Start time:</b></td>\n<td>"+mStartTime.toString("yyyy-MM-dd hh:mm:ss")+"</td>\n</tr>\n");

    //Duration
    htmlCode.append("<tr>\n<td><b>Duration:</b></td>\n<td>"+QString::number(mStartTime.secsTo(QDateTime::currentDateTime()))+" seconds</td>\n</tr>\n");

    //Model name
    htmlCode.append("<tr>\n<td><b>Model:</b></td>\n<td>"+mModelPtrs.first()->getViewContainerObject()->getName()+"</td>\n</tr>\n");

    //Algorithm
    QString algStr;
    switch(getAlgorithm())
    {
    case OptimizationHandler::ComplexRF:
        algStr = "Complex-RF";
        break;
    case OptimizationHandler::ComplexRFM:
        algStr = "Complex-RFM";
        break;
    case OptimizationHandler::ComplexRFP:
        algStr = "Complex-RFP";
        break;
    case OptimizationHandler::PSO:
        algStr = "Particle Swarm";
        break;
    case OptimizationHandler::ParameterSweep:
        algStr = "Parameter Sweep";
        break;
    case OptimizationHandler::Uninitialized:
        algStr = "Uninitialized";
        break;
    default:
        algStr = "Unknown";
    }
    htmlCode.append("<tr>\n<td><b>Algorithm:</b></td>\n<td>"+algStr+"</td>\n</tr>\n");

    //Iterations
    htmlCode.append("<tr>\n<td><b>Iterations:</b></td>\n<td>"+QString::number(mpWorker->getCurrentNumberOfIterations())+"</td>\n</tr>\n");

    //Function Evaluations
    htmlCode.append("<tr>\n<td><b>Function Evaluations:</b></td>\n<td>"+QString::number(mEvaluations)+"</td>\n</tr>\n");

    //Meta model evaluations
    htmlCode.append("<tr>\n<td><b>Surrogate Model Evaluations:</b></td>\n<td>"+QString::number(0)+"</td>\n</tr>\n");

    //End general information
    htmlCode.append("</table>\n\n");

    //Begin general information
    htmlCode.append("<h3>Best Point:</h3>\n<table>\n");

    //Objective function value
    htmlCode.append("<tr>\n<td><b>f(x)</b></td>\n<td>"+QString::number(mpWorker->getObjectiveValue(mpWorker->getBestId()))+"</td>\n</tr>\n");

    //Parameter values
    for(int i=0; i<(int)mpWorker->getNumberOfParameters(); ++i)
    {
        htmlCode.append("<tr>\n<td><b>x"+QString::number(i+1)+"</b></td>\n<td>"+QString::number(mpWorker->getParameter(mpWorker->getBestId(),i))+"</td>\n</tr>\n");

    }

    //End general information
    htmlCode.append("</table>\n\n");

    //End body
    htmlCode.append("</body>\n");

    QFile resultFile(gpDesktopHandler->getDocumentsPath()+"OptimizationResultFile_"+QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".html");
    resultFile.open(QFile::WriteOnly | QFile::Text);
    resultFile.write(htmlCode.toUtf8());
    resultFile.close();
}




//! @brief Logs parameters and objective value of all points to log variables
void OptimizationHandler::logAllPoints()
{
    for(int i=0; i<(int)mpWorker->getNumberOfPoints(); ++i)
    {
        logPoint(i);
    }
}



//! @brief Logs all parameters and the objective value of specified point to log variables
//! @param idx Index of point to save
void OptimizationHandler::logPoint(int idx)
{
    if(!mPrintLogFile) return;

    mLoggedParameters.append(QVector<double>());
    mLoggedParameters.last().append(mpWorker->getObjectiveValue(idx));
    for(int p=0; p<(int)mpWorker->getNumberOfParameters(); ++p)
    {
        mLoggedParameters.last().append(mpWorker->getParameter(idx,p));
    }
}


void OptimizationHandler::printLogFile()
{
    if(!mPrintLogFile) return;


    QFile logFile(gpDesktopHandler->getDocumentsPath()+"OptimizationLogFile_"+QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".csv");
    logFile.open(QFile::WriteOnly | QFile::Text);
    for(int i=0; i<mLoggedParameters.size(); ++i)
    {
        QString line;
        for(int j=0; j<mLoggedParameters[i].size(); ++j)
        {
            line.append(QString::number(mLoggedParameters[i][j])+",");
        }
        line.chop(1);
        line.append("\n");
        logFile.write(line.toUtf8());
    }
    logFile.close();
}

void OptimizationHandler::printDebugFile()
{
    if(!mPrintDebugFile) return;

    QFile debugFile(gpDesktopHandler->getDocumentsPath()+"OptimizationDebugFile_"+QDateTime::currentDateTime().toString("yyyyMMdd")+".txt");
    debugFile.open(QFile::WriteOnly | QFile::Text | QFile::Append);
    QString output = QString::number(getAlgorithm())+",";
    output.append(QString::number(mpWorker->getNumberOfCandidates())+",");
    output.append(QString::number(mpWorker->getCurrentNumberOfIterations())+",");
    output.append(QString::number(mEvaluations)+",");
    output.append(QString::number(0)+",");  //Surrogate models, not currently implemented
    output.append(QString::number(mpWorker->getObjectiveValue(mpWorker->getBestId()))+",");
    for(int i=0; i<(int)mpWorker->getNumberOfParameters(); ++i)
    {
        output.append(QString::number(mpWorker->getParameter(mpWorker->getBestId(),i))+",");
    }
    output.chop(1);
    output.append("\n");
    debugFile.write(output.toUtf8());
    debugFile.close();
}

void OptimizationHandler::checkIfRescheduleIsNeeded()
{
#ifdef USEZMQ
        if (mNeedsRescheduling)
        {
            int pm, pa; double su;
            rescheduleForBestSpeedup(pm,pa,su);
            // Setup parallel server queues
            if (gpConfig->getBoolSetting(cfg::useremoteoptimization))
            {
                mpRemoteSimulationQueueHandler->setupModelQueues(mModelPtrs.mid(0, mpWorker->getNumberOfCandidates()), pm);
            }
            mNeedsRescheduling = false;
        }
#endif
}

void OptimizationHandler::reInitialize(int nModels)
{
    mpWorker->setNumberOfCandidates(nModels);   //! @todo This needs more error checking in algorithms, so they don't try to use no longer existing candidates

    while(mModelPtrs.size() < nModels)
    {
        addModel(gpModelHandler->loadModel(mModelPath, ModelHandler::IgnoreAlreadyOpen | ModelHandler::Detatched));

        //Make sure logging is disabled/enabled for same ports as in original model
        CoreSystemAccess *pCore = mModelPtrs.first()->getTopLevelSystemContainer()->getCoreSystemAccessPtr();
        for(const QString &compName : mModelPtrs.first()->getTopLevelSystemContainer()->getModelObjectNames()) {
            for(const Port *port : mModelPtrs.first()->getTopLevelSystemContainer()->getModelObject(compName)->getPortListPtrs()) {
                QString portName = port->getName();
                bool enabled = pCore->isLoggingEnabled(compName, portName);
                SystemObject *pOptSystem = mModelPtrs.last()->getTopLevelSystemContainer();
                CoreSystemAccess *pOptCore = pOptSystem->getCoreSystemAccessPtr();
                pOptCore->setLoggingEnabled(compName, portName, enabled);
            }
        }
    }
}

void OptimizationHandler::rescheduleForBestSpeedup(int &pm, int &pa, double &su, bool doBenchmark)
{
#ifdef USEZMQ
    // Check algorithm
    Ops::AlgorithmT algo = mpWorker->getAlgorithm();
    if (algo == Ops::ComplexRFP)
    {
        Ops::WorkerComplexRFP *pCRFPWorker = dynamic_cast<Ops::WorkerComplexRFP*>(mpWorker);
        if (pCRFPWorker && pCRFPWorker->getParallelMethod() == Ops::TaskPrediction)
        {
            //! @todo this Crfp0... method needs replacing with new data
            removeRemoteSimulationQueueHandler(mpRemoteSimulationQueueHandler);
            mpRemoteSimulationQueueHandler = createRemoteSimulationQueueHandler(Crfp0_Homo_Reschedule);
        }
        else if (pCRFPWorker && pCRFPWorker->getParallelMethod() == Ops::MultiDistance)
        {
            //! @todo this Crfp0... method needs replacing with new data
            removeRemoteSimulationQueueHandler(mpRemoteSimulationQueueHandler);
            mpRemoteSimulationQueueHandler = createRemoteSimulationQueueHandler(Crfp1_Homo_Reschedule);
        }
        // Fall-back
        else
        {
            removeRemoteSimulationQueueHandler(mpRemoteSimulationQueueHandler);
            mpRemoteSimulationQueueHandler = createRemoteSimulationQueueHandler(Basic);
            mNeedsRescheduling = false;
        }

        if (doBenchmark)
        {
            mpRemoteSimulationQueueHandler->benchmarkModel(mModelPtrs.front());
        }
        mpRemoteSimulationQueueHandler->determineBestSpeedup(-1, 8, pm, pa, su);
        reInitialize(pa);
    }
    else if (algo == Ops::ParticleSwarm)
    {
        removeRemoteSimulationQueueHandler(mpRemoteSimulationQueueHandler);
        mpRemoteSimulationQueueHandler = createRemoteSimulationQueueHandler(Pso_Homo_Reschedule);

        if (doBenchmark)
        {
            mpRemoteSimulationQueueHandler->benchmarkModel(mModelPtrs.front());
        }
        mpRemoteSimulationQueueHandler->determineBestSpeedup(-1, mModelPtrs.size(), pm, pa, su);
        mNeedsRescheduling = false; //! @todo maybe returnode
    }
    else
    {
        removeRemoteSimulationQueueHandler(mpRemoteSimulationQueueHandler);
        mpRemoteSimulationQueueHandler = createRemoteSimulationQueueHandler(Basic);
        mpRemoteSimulationQueueHandler->determineBestSpeedup(-1, mModelPtrs.size(), pm, pa, su);
        mNeedsRescheduling = false;
    }
#endif
}

OptimizationMessageHandler::OptimizationMessageHandler(OptimizationHandler *pHandler)
{
    mpHandler = pHandler;
}

void OptimizationMessageHandler::printMessage(char* str)
{
    mpHandler->getMessageHandler()->addInfoMessage(str);
}

void OptimizationMessageHandler::pointsChanged()
{
    mpHandler->updateOutputs();
    mpHandler->plotPoints();
    mpHandler->plotParameters();
    mpHandler->plotEntropy();
    mpHandler->logAllPoints();
}

void OptimizationMessageHandler::pointChanged(size_t idx)
{
    mpHandler->updateOutputs();
    mpHandler->plotPoints();
    mpHandler->plotParameters();
    mpHandler->plotEntropy();
    mpHandler->logPoint(idx);
}

void OptimizationMessageHandler::objectivesChanged()
{
    mpHandler->plotObjectiveValues();
    mpHandler->updateOutputs();
}

void OptimizationMessageHandler::objectiveChanged(size_t idx)
{
    Q_UNUSED(idx);
    mpHandler->plotObjectiveValues();
    mpHandler->updateOutputs();
}

void OptimizationMessageHandler::candidatesChanged()
{
    mpHandler->plotPoints(OptimizationHandler::Candidates);
}

void OptimizationMessageHandler::candidateChanged(size_t idx)
{
    Q_UNUSED(idx);
    mpHandler->plotPoints(OptimizationHandler::Candidates);
}

void OptimizationMessageHandler::abort()
{
    mIsAborted = true;
}

void OptimizationMessageHandler::stepCompleted(size_t steps)
{
    mpHandler->updateProgressBar(steps);
}
