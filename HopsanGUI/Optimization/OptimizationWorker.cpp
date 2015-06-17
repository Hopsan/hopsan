/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   OptimizationWorker.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains a base class for optimization worker objects
//!

#include <QFile>

#include "OptimizationHandler.h"
#include "OptimizationWorker.h"
#include "Dialogs/OptimizationDialog.h"
#include "Widgets/HcomWidget.h"
#include "HcomHandler.h"
#include "ModelHandler.h"
#include "DesktopHandler.h"
#include "global.h"
#include "Widgets/ModelWidget.h"
#include "GUIObjects/GUIContainerObject.h"
#include "PlotHandler.h"
#include "PlotWindow.h"
#include "PlotArea.h"
#include "PlotCurve.h"
#include "Configuration.h"
#include "Utilities/GUIUtilities.h"
#include "CoreAccess.h"
#include "ComponentSystem.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"

//! @brief Checks for convergence (in either of the algorithms)
OptimizationWorker::OptimizationWorker(OptimizationHandler *pHandler)
{
    mpHandler = pHandler;

    mPlotPoints = false;
    mPlotObjectiveFunctionValues = false;
    mPlotParameters = false;
    mPlotEntropy = false;
    mDoLog = true;
    mFinalEval = true;
    mNumModels = 1;
}

OptimizationWorker::~OptimizationWorker()
{
    this->clearModels();
}


//! @brief Initialization function for optimization worker base class (should never be called directly)
void OptimizationWorker::init(const ModelWidget *pModel, const QString &modelPath)
{
    //Load model widgets
//    if(mModelPtrs.size() > mNumModels)
//    {
//        clearModels();
//    }
    //for(int i=0; i<mpOptHandler->getOptVar("npoints"); ++i)
    while(mModelPtrs.size() < mNumModels)
    {
        mpHandler->addModel(gpModelHandler->loadModel(modelPath, true, true));

        //Make sure logging is disabled/enabled for same ports as in original model
        CoreSystemAccess *pCore = pModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr();
        foreach(const QString &compName, pModel->getTopLevelSystemContainer()->getModelObjectNames())
        {
            foreach(const Port *port, pModel->getTopLevelSystemContainer()->getModelObject(compName)->getPortListPtrs())
            {
                QString portName = port->getName();
                bool enabled = pCore->isLoggingEnabled(compName, portName);
                SystemContainer *pOptSystem = mModelPtrs.last()->getTopLevelSystemContainer();
                CoreSystemAccess *pOptCore = pOptSystem->getCoreSystemAccessPtr();
                pOptCore->setLoggingEnabled(compName, portName, enabled);
            }
        }
    }

    mIterations = 0;
    mEvaluations = 0;
    mMetaModelEvaluations = 0;

    mDisconnectedFromModelHandler = disconnect(gpModelHandler, SIGNAL(modelChanged(ModelWidget*)), mpHandler->mpHcomHandler, SLOT(setModelPtr(ModelWidget*)));

    //Clear previous data in case models were re-used
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[i]);
        mpHandler->mpHcomHandler->executeCommand("rmvar *");
    }
    mpHandler->mpHcomHandler->setModelPtr(mModelPtrs.first());

    //Load default optimization functions
    QString oldPath = mpHandler->mpHcomHandler->getWorkingDirectory();
    mpHandler->mpHcomHandler->setWorkingDirectory(gpDesktopHandler->getExecPath());
    //executeCommand("exec ../Scripts/HCOM/optDefaultFunctions.hcom");
    QFile testFile1(gpDesktopHandler->getScriptsPath()+"HCOM/optDefaultFunctions.hcom");
    QFile testFile2(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optDefaultFunctions.hcom");
    if(testFile1.exists())
    {
        execute("exec \""+testFile1.fileName()+"\"");
    }
    else if(testFile2.exists())
    {
        execute("exec \""+testFile2.fileName()+"\"");
    }
    else
    {
        printError("Cannot find optimization default functions script file.","",false);
        return;
    }
    mpHandler->mpHcomHandler->setWorkingDirectory(oldPath);

    mPercent = -1;

    mpHandler->setIsRunning(true);

    mOrgProgressBarSetting = gpConfig->getBoolSetting(CFG_PROGRESSBAR);
    mOrgLimitDataGenerationsSetting = gpConfig->getBoolSetting(CFG_AUTOLIMITGENERATIONS);
    gpConfig->setBoolSetting(CFG_PROGRESSBAR, false);
    gpConfig->setBoolSetting(CFG_AUTOLIMITGENERATIONS, true);

    mLoggedParameters.clear();
    mLoggedParameters.resize(mNumParameters);

    mLogFile.setFileName(gpDesktopHandler->getDocumentsPath()+"OptLog"+QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".csv");

    if(mDoLog)
    {
        if(!mLogFile.open(QFile::WriteOnly | QFile::Text | QFile::Append))
        {
            mDoLog = false;
            printError("Cannot write to log file. Logging will be disabled.");
        }
    }
}


//! @brief Run function for optimization worker base class (should never be called directly)
void OptimizationWorker::run()
{
    //Nothing to do
}


//! @brief Finalize function for optimization worker base class (should never be called directly)
void OptimizationWorker::finalize()
{

    //Re-evaluate all points (to remove any effect from forgetting factor before logging)
    execute("echo off -nonerrors");
    if(mFinalEval && !mpHandler->mpHcomHandler->isAborted())
    {
        execute("call evalall");
    }
    calculateBestAndWorstId();
    gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);
    double secondBestObj = mObjectives[mWorstId];
    mSecondBestId = mWorstId;
    for(int i=0; i<mNumPoints; ++i)
    {
        if(i != mBestId && mObjectives[i] < secondBestObj)
        {
            secondBestObj = mObjectives[i];
            mSecondBestId = i;
        }
    }
    execute("echo on");


    LogDataHandler2 *pLogDataHandler = mpHandler->getModelPtrs()->first()->getLogDataHandler();
    for(int p=0; p<mNumParameters; ++p)
    {
        SharedVectorVariableT logVec = pLogDataHandler->createOrphanVariable("optpar"+QString::number(p));
        logVec->assignFrom(mLoggedParameters.at(p));
        mpHandler->getModelPtrs()->first()->getLogDataHandler()->insertNewVectorVariable(logVec);
    }

    print("Optimization finished!");
    updateProgressBar(mMaxEvals);
    mpHandler->setIsRunning(false);
    if(mDisconnectedFromModelHandler)
    {
        connect(gpModelHandler, SIGNAL(modelChanged(ModelWidget*)), mpHandler->mpHcomHandler, SLOT(setModelPtr(ModelWidget*)));
    }
    mpHandler->mpHcomHandler->mpConsole->mpTerminal->setAbortButtonEnabled(false);
    gpOptimizationDialog->setOptimizationFinished();



    QFile resultFile(gpDesktopHandler->getDocumentsPath()+"optimization_results_"+QDateTime::currentDateTime().toString("yyyyMMdd")+".txt");
    resultFile.open(QFile::WriteOnly | QFile::Text | QFile::Append);
    QString output = QString::number(mpHandler->getAlgorithm())+",";
    output.append(QString::number(mNumModels)+",");
    output.append(QString::number(mIterations)+",");
    output.append(QString::number(mEvaluations)+",");
    output.append(QString::number(mMetaModelEvaluations)+",");
    output.append(QString::number(mObjectives[mBestId])+",");
    for(int i=0; i<mNumParameters; ++i)
    {
        output.append(QString::number(mParameters[mBestId][i])+",");
    }
    output.append(QString::number(mObjectives[mSecondBestId])+",");
    for(int i=0; i<mNumParameters; ++i)
    {
        output.append(QString::number(mParameters[mSecondBestId][i])+",");
    }
    output.chop(1);
    output.append("\n");
    resultFile.write(output.toUtf8());
    resultFile.close();

    printLogFile();

    if(mDoLog)
    {
        mLogFile.close();
    }

    gpConfig->setBoolSetting(CFG_PROGRESSBAR, mOrgProgressBarSetting);
    gpConfig->setBoolSetting(CFG_AUTOLIMITGENERATIONS, mOrgLimitDataGenerationsSetting);
}

void OptimizationWorker::printLogFile()
{
    QString htmlCode;

    //Header
    htmlCode.append("<html>\n<head>\n<title>Hopsan Optimization Log</title>\n</head>\n<body>\n\n");

    //CSS style
    htmlCode.append("<style type=\"text/css\">\n  td {\n    width: 170pt\n  }\n</style>\n\n");

    //Title
    htmlCode.append("<h1>Hopsan Optimization Log</h1>\n");

    //Begin general information
    htmlCode.append("<h3>General Information:</h3>\n<table>\n");

    //Date & time
    htmlCode.append("<tr>\n<td><b>Date & time:</b></td>\n<td>"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"</td>\n</tr>\n");

    //Model name
    htmlCode.append("<tr>\n<td><b>Model:</b></td>\n<td>"+mModelPtrs.first()->getViewContainerObject()->getName()+"</td>\n</tr>\n");

    //Algorithm
    QString algStr;
    switch(mpHandler->getAlgorithm())
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
    case OptimizationHandler::ParticleSwarm:
        algStr = "Particle Swarm";
        break;
    case OptimizationHandler::ParameterSweep:
        algStr = "Parameter Sweep";
        break;
    case OptimizationHandler::Uninitialized:
        algStr = "Uninitialized";
    default:
        algStr = "Unknown";
    }
    htmlCode.append("<tr>\n<td><b>Algorithm:</b></td>\n<td>"+algStr+"</td>\n</tr>\n");

    //Iterations
    htmlCode.append("<tr>\n<td><b>Iterations:</b></td>\n<td>"+QString::number(mIterations)+"</td>\n</tr>\n");

    //Function Evaluations
    htmlCode.append("<tr>\n<td><b>Function Evaluations:</b></td>\n<td>"+QString::number(mEvaluations)+"</td>\n</tr>\n");

    //Meta model evaluations
    htmlCode.append("<tr>\n<td><b>Meta Model Evaluations:</b></td>\n<td>"+QString::number(mMetaModelEvaluations)+"</td>\n</tr>\n");

    //End general information
    htmlCode.append("</table>\n\n");

    //Begin general information
    htmlCode.append("<h3>Best Point:</h3>\n<table>\n");

    //Objective function value
    htmlCode.append("<tr>\n<td><b>f(x)</b></td>\n<td>"+QString::number(mObjectives[mBestId])+"</td>\n</tr>\n");

    //Parameter values
    for(int i=0; i<mNumParameters; ++i)
    {
        htmlCode.append("<tr>\n<td><b>x"+QString::number(i+1)+"</b></td>\n<td>"+QString::number(mParameters[mBestId][i])+"</td>\n</tr>\n");

    }

    //End general information
    htmlCode.append("</table>\n\n");

    //End body
    htmlCode.append("</body>\n");

    QFile logFile(gpDesktopHandler->getDocumentsPath()+"OptLog"+QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".html");
    logFile.open(QFile::WriteOnly | QFile::Text);
    logFile.write(htmlCode.toUtf8());
    logFile.close();
}


//! @brief Logs all parameters and the objective value of specified point to log variables
//! @param idx Index of point to save
void OptimizationWorker::logPoint(int idx)
{
    for(int p=0; p<mNumParameters; ++p)
    {
        mLoggedParameters[p].append(mParameters[idx][p]);
    }

    return;     //! @todo Make this work again (should probably store values in QVector<double> and only create a vector variable after optimization is finished)



    LogDataHandler2 *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();

    for(int p=0; p<mNumParameters; ++p)
    {
        QString name = "optpar"+QString::number(p);
        SharedVectorVariableT parVar = pHandler->getVectorVariable(name, -1);
        if(!parVar)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            parVar = pHandler->defineNewVectorVariable(name);
            parVar->preventAutoRemoval();

            parVar->assignFrom(mParameters[idx][p]);
        }
        else
        {
            parVar->append(mParameters[idx][p]);
        }
    }

    QString name = "optobj";
    SharedVectorVariableT objVar = pHandler->getVectorVariable(name, -1);
    if(!objVar)
    {
        //! @todo we should set name and unit and maybe description (in define variable)
        objVar = pHandler->defineNewVectorVariable(name);
        objVar->preventAutoRemoval();

        objVar->assignFrom(mObjectives[idx]);
    }
    else
    {
        objVar->append(mObjectives[idx]);
    }

    if(mDoLog)
    {
        QString logLine;
        logLine.append(QString::number(mObjectives[idx])+",");
        for(int p=0; p<mNumParameters; ++p)
        {
            logLine.append(QString::number(mParameters[idx][p])+",");
        }
        logLine.chop(1);
        logLine.append("\n");
        mLogFile.write(logLine.toUtf8());
    }
}



//! @brief Logs parameters and objective value of worst point to log variables
void OptimizationWorker::logWorstPoint()
{
    logPoint(mWorstId);
}


//! @brief Logs parameters and objective value of all points to log variables
void OptimizationWorker::logAllPoints()
{
    for(int i=0; i<mNumPoints; ++i)
    {
        logPoint(i);
    }
}


//! @brief Checks whether or not any of the convergence criteria has been fulfilled
bool OptimizationWorker::checkForConvergence()
{
    //Check objective function convergence
    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    for(int i=0; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    if(fabs(maxObj-minObj) <= mFuncTol)
    {
        mConvergenceReason=1;
        return true;
    }
    else if(minObj != 0.0 && fabs(maxObj-minObj)/fabs(minObj) <= mFuncTol)
    {
        mConvergenceReason=1;
        return true;
    }

    //Check parameter value convergence
    double maxDiff=getMaxParDiff();
    if(fabs(maxDiff) < mParTol)
    {
        mConvergenceReason=2;
        return true;
    }
    return false;
}


//! @brief Calculates indexes of best and worst point
void OptimizationWorker::calculateBestAndWorstId()
{
    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    mWorstId=0;
    mBestId=0;
    for(int i=1; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj)
        {
            maxObj = obj;
            mWorstId = i;
        }
        if(obj < minObj)
        {
            minObj = obj;
            mBestId = i;
        }
    }
    if(mWorstId == mBestId)
    {
        mWorstId = 0;
        mBestId = 1;
    }
}


//! @brief Plots the optimization points (if there are at least two parameters and the option is selected)
void OptimizationWorker::plotPoints()
{
    if(!mPlotPoints) { return; }

    if(mNumParameters < 2)
    {
        printError("Plotting points requires at least two parameters.");
        return;
    }

    for(int p=0; p<mNumPoints; ++p)
    {
        double x = mParameters[p][0];
        double y = mParameters[p][1];

        if(mPointVars_x.size() <= p)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            mPointVars_x.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));
            mPointVars_y.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));

            mPointVars_x.last()->assignFrom(x);
            mPointVars_y.last()->assignFrom(y);

            gpPlotHandler->plotDataToWindow("parplot", mPointVars_x.last(), mPointVars_y.last(), 0);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLimits(QwtPlot::xBottom, mParMin[0], mParMax[0]);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLimits(QwtPlot::yLeft, mParMin[1], mParMax[1]);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLabel(QwtPlot::xBottom, "Optimization Parameter 0");
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLabel(QwtPlot::yLeft, "Optimization Parameter 1");
        }
        else
        {
            //! @todo need to turn of auto refresh on plot and trigger it manually to avoid multiple redraws here
            mPointVars_x.at(p)->assignFrom(x);
            mPointVars_y.at(p)->assignFrom(y);
        }
    }

    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow("parplot");
    if(pPlotWindow)
    {
        PlotTab *pTab = pPlotWindow->getCurrentPlotTab();
        for(int c=0; c<pTab->getCurves(0).size(); ++c)
        {
            if(c==mBestId)
            {
                pTab->getCurves(0).at(c)->setLineSymbol("Star 1");
            }
            else
            {
                pTab->getCurves(0).at(c)->setLineSymbol("XCross");
            }
        }
        pTab->update();
    }
}


//! @brief Plots best and worst objective values (if option is selected)
void OptimizationWorker::plotObjectiveFunctionValues()
{
    if(!mPlotObjectiveFunctionValues) { return; }

    //Best objective value
    if(mBestVar.isNull())
    {
        mBestVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mBestVar->assignFrom(mObjectives[mBestId]);
    }
    else
    {
        mBestVar->append(mObjectives[mBestId]);
    }

    //Worst objective value
    if(mWorstVar.isNull())
    {
        mWorstVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mWorstVar->assignFrom(mObjectives[mWorstId]);
    }
    else
    {
        mWorstVar->append(mObjectives[mWorstId]);
    }

    //Newest objective value
    if(mNewestVar.isNull())
    {
        mNewestVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mNewestVar->assignFrom(mObjectives[mLastWorstId]);
    }
    else
    {
        mNewestVar->append(mObjectives[mLastWorstId]);
    }

    // If this is the first time, then recreate the plotwindows
    // Note! plots will auto update when new data is appended, so there is no need to call plotab->update()
    if(mBestVar->getDataSize() == 1)
    {
        PlotWindow *pPW = gpPlotHandler->createNewOrReplacePlotwindow("ObjectiveFunction");
        gpPlotHandler->plotDataToWindow(pPW, mBestVar, 0, true, QColor("Green"));
        gpPlotHandler->plotDataToWindow(pPW, mWorstVar, 0, true, QColor("Red"));
        gpPlotHandler->plotDataToWindow(pPW, mNewestVar, 0, true, QColor("Orange"));
    }
}


//! @brief Plots parameter values (if option is selected)
void OptimizationWorker::plotParameters()
{
    if(!mPlotParameters) { return; }

    for(int p=0; p<mNumParameters; ++p)
    {
        if(mParVars.size() <= p)
        {
            mParVars.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));
            mParVars.last()->assignFrom(mParameters[mLastWorstId][p]);
        }
        else
        {
            mParVars.at(p)->append(mParameters[mLastWorstId][p]);
        }

        // If this is the first time, then recreate the plotwindows
        // Note! plots will auto update when new data is appended, so there is no need to call plotab->update()
        if(mParVars.at(p)->getDataSize() == 1)
        {
            PlotWindow *pPW = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("ParameterValues");
            gpPlotHandler->plotDataToWindow(pPW, mParVars.at(p), 0, true);
        }
    }
}


//! @brief Plots the entropy of the points in the optimization
void OptimizationWorker::plotEntropy()
{
    if(!mPlotEntropy) { return; }

    double deltaX = getMaxParDiff();
    int n = mParameters.size();
    double entropy = -n*log2(deltaX);


    if(mEntropyVar.isNull())
    {
        mEntropyVar = createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription));
        mEntropyVar->assignFrom(entropy);
    }
    else
    {
        mEntropyVar->append(entropy);
    }

    // If this is the first time, then recreate the plotwindows
    if(mEntropyVar.data()->getDataSize() == 1)
    {
        PlotWindow *pPW = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("OptimizationEntropy");
        gpPlotHandler->plotDataToWindow(pPW, mEntropyVar, 0, true);
    }
}


//! @brief Set function for optimization variables
//! @param var Name of variable to set
//! @param value Value for variable
void OptimizationWorker::setOptVar(const QString &var, const QString &value)
{
    if(var == "plotpoints")
    {
        mPlotPoints = (value == "on");
    }
    else if(var == "plotbestworst")
    {
        mPlotObjectiveFunctionValues = (value == "on");
    }
    else if(var == "plotvariables")
    {
        mPlotVariables = (value == "on");
    }
    else if(var == "plotparameters")
    {
        mPlotParameters = (value == "on");
    }
    else if(var == "plotentropy")
    {
        mPlotEntropy = (value == "on");
    }
    else if(var == "npoints")
    {
        int n = value.toInt();
        mNumPoints = n;
        mParameters.resize(n);
    }
    else if(var == "nparams")
    {
        int n = value.toInt();
        mNumParameters = n;
        mParMin.resize(n);
        mParMax.resize(n);
    }
    else if(var == "parnames")
    {
        mParNames = value.split(",");
    }
    else if(var == "functol")
    {
        mFuncTol = value.toDouble();
    }
    else if(var == "partol")
    {
        mParTol = value.toDouble();
    }
    else if(var == "maxevals")
    {
        mMaxEvals = value.toInt();
    }

    else if(var == "evalid")
    {
        mEvalId = value.toDouble();
    }
    else if(var == "log")
    {
        mDoLog = (value == "on");
    }
    else if(var == "finaleval")
    {
        mFinalEval = (value == "on");
    }
}


//! @brief Returns value of specified optimization variable
//! @param var Name of variable
//! @param ok True if variable was found, else false
double OptimizationWorker::getOptVar(const QString &var, bool &ok)
{
    ok=true;
    if(var == "plotpoints")
    {
        if(mPlotPoints)
            return 1;
        else
            return 0;
    }
    else if(var == "plotbestworst")
    {
        if(mPlotObjectiveFunctionValues)
            return 1;
        else
            return 0;
    }
    else if(var == "plotparameters")
    {
        if(mPlotParameters)
            return 1;
        else
            return 0;
    }
    else if(var == "plotentropy")
    {
        if(mPlotEntropy)
            return 1;
        else
            return 0;
    }
    else if(var == "npoints")
    {
        return mNumPoints;
    }
    else if(var == "nparams")
    {
        return mNumParameters;
    }
    else if(var == "functol")
    {
        return mFuncTol;
    }
    else if(var == "partol")
    {
        return mParTol;
    }
    else if(var == "maxevals")
    {
        return mMaxEvals;
    }
    else if(var == "evalid")
    {
        return mEvalId;
    }
    else if(var == "bestid")
    {
        return mBestId;
    }
    else if(var == "worstid")
    {
        return mWorstId;
    }
    else if(var == "niter")
    {
        return mIterations;
    }
    else
    {
        ok=false;
        return 0;
    }
}


//! @brief Sets minimum parameter value for specified parameter
//! @param idx Index of parameter
//! @param value New minimum value for parameter
void OptimizationWorker::setParMin(int idx, double value)
{
    mParMin[idx] = value;
}


//! @brief Sets maximum parameter value for specified parameter
//! @param idx Index of parameter
//! @param value New maximum value for parameter
void OptimizationWorker::setParMax(int idx, double value)
{
    mParMax[idx] = value;
}


//! @brief Sets objective function value for specified point
//! @param idx Index of point
//! @param value New objective value
void OptimizationWorker::setOptimizationObjectiveValue(int idx, double value)
{
    if(idx<0 || idx > mObjectives.size()-1)
    {
        return;
    }
    mObjectives[idx] = value;
}


//! @brief Returns objective value of specified point
//! @param idx Index of point
double OptimizationWorker::getOptimizationObjectiveValue(int idx)
{
    if(idx<0 || idx > mObjectives.size()-1)
    {
        return 0;
    }
    return mObjectives[idx];
}


//! @brief Returns value in specified parameter in specified point
//! @param pointIdx Index of point
//! @param parIdx Index of parameter
double OptimizationWorker::getParameter(const int pointIdx, const int parIdx) const
{
    if(mParameters.size() < pointIdx+1)
    {
        return 0;
    }
    else if(mParameters[pointIdx].size() < parIdx)
    {
        return 0;
    }
    return mParameters[pointIdx][parIdx];
}


//! @brief Prints a message to the console
//! @param msg Message to print
void OptimizationWorker::print(const QString &msg)
{
    print(msg, "", false);
}


//! @brief Prints a message to the console with specified flags
//! @param msg Message to print
//! @param tag Tag of message (for grouping similar messages)
//! @param timeStamp Tells whether or not time stamps shall be shown
void OptimizationWorker::print(const QString &msg, const QString &tag, bool timeStamp)
{
    mpHandler->getMessageHandler()->addInfoMessage(msg, tag, timeStamp);
}


//! @brief Prints an error message to the console
//! @param msg Message to print
void OptimizationWorker::printError(const QString &msg)
{
    printError(msg, "", false);
}

void OptimizationWorker::printError(const QString &msg, const QString &tag, bool timeStamp)
{
    mpHandler->getMessageHandler()->addErrorMessage(msg, tag, timeStamp);
}

void OptimizationWorker::execute(const QString &cmd)
{
    mpHandler->mpHcomHandler->executeCommand(cmd);
}

void OptimizationWorker::updateProgressBar(int i)
{
    int dummy=int(100.0*double(i)/mMaxEvals);
    if(dummy != mPercent)    //Only update at whole numbers
    {
        mPercent = dummy;
        gpOptimizationDialog->updateTotalProgressBar(dummy);
    }
    qApp->processEvents();
}

//! @brief Returns the maximum difference between smallest and largest parameter
double OptimizationWorker::getMaxParDiff()
{
    double maxDiff = -1e100;
    for(int i=0; i<mNumParameters; ++i)
    {
        double maxPar = -1e100;
        double minPar = 1e100;
        for(int p=0; p<mNumPoints; ++p)
        {
            if(mParameters[p][i] > maxPar) maxPar = mParameters[p][i];
            if(mParameters[p][i] < minPar) minPar = mParameters[p][i];
        }
        if(mParMax[i] != mParMin[i] && (maxPar-minPar)/(mParMax[i]-mParMin[i]) > maxDiff)
        {
            maxDiff = (maxPar-minPar)/(mParMax[i]-mParMin[i]);
        }
    }
    return maxDiff;
}

double OptimizationWorker::getMaxParDiff(QVector<QVector<double> > &points)
{
    double maxDiff = -1e100;
    for(int i=0; i<mNumParameters; ++i)
    {
        double maxPar = -1e100;
        double minPar = 1e100;
        for(int p=0; p<points.size(); ++p)
        {
            if(points[p][i] > maxPar) maxPar = points[p][i];
            if(points[p][i] < minPar) minPar = points[p][i];
        }
        if(mParMax[i] != mParMin[i] && (maxPar-minPar)/(mParMax[i]-mParMin[i]) > maxDiff)
        {
            maxDiff = (maxPar-minPar)/(mParMax[i]-mParMin[i]);
        }
    }
    return maxDiff;
}

QStringList *OptimizationWorker::getParNamesPtr()
{
    return &mParNames;
}

void OptimizationWorker::clearModels()
{
    mpHandler->mpHcomHandler->setModelPtr(gpModelHandler->getCurrentModel());
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        mModelPtrs[i]->mpParentModelHandler->closeModel(mModelPtrs[i], true);
        delete(mModelPtrs[i]);
    }
    mModelPtrs.clear();
}
