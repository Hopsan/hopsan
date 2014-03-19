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
//! @file   OptimizationWorker.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id: OptimizationHandler.cpp 6525 2014-01-30 15:58:59Z petno25 $
//!
//! @brief Contains a base class for optimization worker objects
//!

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

//! @brief Checkes for convergence (in either of the algorithms)
OptimizationWorker::OptimizationWorker(OptimizationHandler *pHandler)
{
    mpHandler = pHandler;

    mPlotPoints = false;
    mPlotObjectiveFunctionValues = false;
    mPlotParameters = false;
    mDoLog = true;
}


//! @brief Initialization function for optimization worker base class (should never be called directly)
void OptimizationWorker::init()
{
    mIterations = 0;
    mEvaluations = 0;
    mMetaModelEvaluations = 0;

    mDisconnectedFromModelHandler = disconnect(gpModelHandler, SIGNAL(modelChanged(ModelWidget*)), mpHandler->mpHcomHandler, SLOT(setModelPtr(ModelWidget*)));

    mpHandler->mpHcomHandler->setModelPtr(mModelPtrs.first());

    //Load default optimization functions
    QString oldPath = mpHandler->mpHcomHandler->getWorkingDirectory();
    mpHandler->mpHcomHandler->setWorkingDirectory(gpDesktopHandler->getExecPath());
    //executeCommand("exec ../Scripts/HCOM/optDefaultFunctions.hcom");
    QFile testFile1(gpDesktopHandler->getScriptsPath()+"/HCOM/optDefaultFunctions.hcom");
    QFile testFile2(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optDefaultFunctions.hcom");
    if(testFile1.exists())
    {
        execute("exec "+testFile1.fileName());
    }
    else if(testFile2.exists())
    {
        execute("exec "+testFile2.fileName());
    }
    else
    {
        printError("Cannot find optimization default functions script file.","",false);
        return;
    }
    mpHandler->mpHcomHandler->setWorkingDirectory(oldPath);

    mPercent = -1;

    mpHandler->setIsRunning(true);
}


//! @brief Run function for optimization worker base class (should never be called directly)
void OptimizationWorker::run()
{
    //Nothing to do
}


//! @brief Finalie function for optimization worker base class (should never be called directly)
void OptimizationWorker::finalize()
{
    //Re-evaluate all points (to remove any effect from forgetting factor before logging)
    execute("echo off");
    execute("call evalall");
    calculateBestAndWorstId();
    double secondBestObj = mObjectives[mWorstId];
    mSecondBestId = mWorstId;
    for(int i=0; i<mNumParameters; ++i)
    {
        if(i != mBestId && mObjectives[i] < secondBestObj)
        {
            secondBestObj = mObjectives[i];
            mSecondBestId = i;
        }
    }
    execute("echo on");

    mpHandler->setIsRunning(false);

    if(mDisconnectedFromModelHandler)
    {
        connect(gpModelHandler, SIGNAL(modelChanged(ModelWidget*)), mpHandler->mpHcomHandler, SLOT(setModelPtr(ModelWidget*)));
    }

    mpHandler->mpHcomHandler->mpConsole->mpTerminal->setAbortButtonEnabled(false);

    gpOptimizationDialog->setOptimizationFinished();



    QFile resultFile(gpDesktopHandler->getDocumentsPath()+"/optimization_results_"+QDateTime::currentDateTime().toString("yyyyMMdd")+".txt");
    resultFile.open(QFile::WriteOnly | QFile::Text | QFile::Append);
    QString output = QString::number(mpHandler->getAlgorithm())+",";
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

    if(mDoLog)
    {
        printLogFile();
    }
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

    QFile logFile(gpDesktopHandler->getDocumentsPath()+"/OptLog"+QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".html");
    logFile.open(QFile::WriteOnly | QFile::Text);
    logFile.write(htmlCode.toUtf8());
    logFile.close();
}


//! @brief Logs all parameters and the objective value of specified point to log variables
//! @param idx Index of point to save
void OptimizationWorker::logPoint(int idx)
{
    LogDataHandler *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();

    for(int p=0; p<mNumParameters; ++p)
    {
        QString name = "optpar"+QString::number(p);
        SharedVectorVariableT parVar = pHandler->getVectorVariable(name, -1);
        if(!parVar)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            parVar = pHandler->defineNewVariable(name);
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
        objVar = pHandler->defineNewVariable(name);
        objVar->preventAutoRemoval();

        objVar->assignFrom(mObjectives[idx]);
    }
    else
    {
        objVar->append(mObjectives[idx]);
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


//! @brief Checks whether or not any of the convergence cricterias has been fulfilled
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

    LogDataHandler *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
    for(int p=0; p<mNumPoints; ++p)
    {
        QString namex = "par"+QString::number(p)+"x";
        QString namey = "par"+QString::number(p)+"y";
        double x = mParameters[p][0];
        double y = mParameters[p][1];
        SharedVectorVariableT parVar_x = pHandler->getVectorVariable(namex, -1);
        SharedVectorVariableT parVar_y = pHandler->getVectorVariable(namey, -1);
        if(!parVar_x)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            parVar_x = pHandler->defineNewVariable(namex);
            parVar_y = pHandler->defineNewVariable(namey);
            parVar_x->preventAutoRemoval();
            parVar_y->preventAutoRemoval();

            parVar_x->assignFrom(x);
            parVar_y->assignFrom(y);

            gpPlotHandler->plotDataToWindow("parplot", parVar_x, parVar_y, 0);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLimits(QwtPlot::xBottom, mParMin[0], mParMax[0]);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLimits(QwtPlot::yLeft, mParMin[1], mParMax[1]);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLabel(QwtPlot::xBottom, "Optimization Parameter 0");
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLabel(QwtPlot::yLeft, "Optimization Parameter 1");
        }
        else
        {
            //! @todo need to turn of auto refresh on plot and trygger it manually to avoid multiple redraws here
            parVar_x->assignFrom(x);
            parVar_y->assignFrom(y);
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

    LogDataHandler *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
    SharedVectorVariableT bestVar = pHandler->getVectorVariable("BestObjective", -1);
    if(bestVar.isNull())
    {
        //! @todo unit and description
        bestVar = pHandler->defineNewVariable("BestObjective");
        bestVar->preventAutoRemoval();
        bestVar->assignFrom(mObjectives[mBestId]);
        bestVar->setCacheDataToDisk(false);
    }
    else
    {
        bestVar->append(mObjectives[mBestId]);
    }
    SharedVectorVariableT worstVar = pHandler->getVectorVariable("WorstObjective", -1);
    if(worstVar.isNull())
    {
        worstVar = pHandler->defineNewVariable("WorstObjective");
        worstVar->preventAutoRemoval();
        worstVar->assignFrom(mObjectives[mWorstId]);
        worstVar->setCacheDataToDisk(false);
    }
    else
    {
        worstVar->append(mObjectives[mWorstId]);
    }

    // If this is the first time, then recreate the plotwindows
    // Note! plots will autoupdate when new data is appended, so there is no need to call plotab->update()
    if(bestVar.data()->getDataSize() == 1)
    {
        PlotWindow *pPW = gpPlotHandler->createNewOrReplacePlotwindow("ObjectiveFunction");
        gpPlotHandler->plotDataToWindow(pPW, bestVar, 0, QColor("Green"));
        gpPlotHandler->plotDataToWindow(pPW, worstVar, 0, QColor("Red"));
    }
}


//! @brief Plots best and worst objective values (if option is selected)
void OptimizationWorker::plotParameters()
{
    if(!mPlotParameters) { return; }

    LogDataHandler *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
    for(int p=0; p<mNumParameters; ++p)
    {
        SharedVectorVariableT par = pHandler->getVectorVariable("NewPar"+QString::number(p), -1);
        if(par.isNull())
        {
            par = pHandler->defineNewVariable("NewPar"+QString::number(p));
            par->preventAutoRemoval();
            par->assignFrom(mParameters[mLastWorstId][p]);
            par->setCacheDataToDisk(false);
        }
        else
        {
            par->append(mParameters[mLastWorstId][p]);
        }

        // If this is the first time, then recreate the plotwindows
        // Note! plots will autoupdate when new data is appended, so there is no need to call plotab->update()
        if(par.data()->getDataSize() == 1)
        {
            PlotWindow *pPW = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("ParameterValues");
            gpPlotHandler->plotDataToWindow(pPW, par, 0, QColor("blue"));
        }
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
        if((maxPar-minPar)/(mParMax[i]-mParMin[i]) > maxDiff)
        {
            maxDiff = (maxPar-minPar)/(mParMax[i]-mParMin[i]);
        }
    }
    return maxDiff;
}

double OptimizationWorker::getMaxParDiff(QVector<QVector<double> > &points)
{
    double maxDiff = -1e100;
    for(int i=0; i<points.size(); ++i)
    {
        double maxPar = -1e100;
        double minPar = 1e100;
        for(int p=0; p<points.size(); ++p)
        {
            if(points[p][i] > maxPar) maxPar = points[p][i];
            if(points[p][i] < minPar) minPar = points[p][i];
        }
        if((maxPar-minPar)/(mParMax[i]-mParMin[i]) > maxDiff)
        {
            maxDiff = (maxPar-minPar)/(mParMax[i]-mParMin[i]);
        }
    }
    return maxDiff;
}
