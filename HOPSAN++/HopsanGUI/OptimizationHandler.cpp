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
//! @file   HcomHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a handler for the HCOM scripting language
//!
//HopsanGUI includes
#include "common.h"
#include "global.h"
#include "Configuration.h"
#include "OptimizationHandler.h"
#include "HcomHandler.h"
#include "ModelHandler.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "GUIObjects/GUISystem.h"
#include "DesktopHandler.h"
#include "PlotHandler.h"
#include "Widgets/ProjectTabWidget.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "PlotCurve.h"
#include "Utilities/GUIUtilities.h"
#include "MainWindow.h"
#include "Dialogs/OptimizationDialog.h"


//! @brief Constructor for optimization  handler class
OptimizationHandler::OptimizationHandler(HcomHandler *pHandler)
{
    mpHcomHandler = pHandler;
    mpConsole = pHandler->mpConsole;
    mpConfig = new Configuration();
    mpConfig->loadFromXml();        //This should work, since changes are always saved to file immideately from gpConfig

    mAlgorithm = Uninitialized;
    mPlotPoints = false;
    mPlotObjectiveFunctionValues = false;
    mPlotParameters = false;
    mPsPrintLogOutput = true;  //! @todo Should be changeable by user
}


//! @brief Returns objective value with specified index
double OptimizationHandler::getOptimizationObjectiveValue(int idx)
{
    if(idx<0 || idx > mObjectives.size()-1)
    {
        return 0;
    }
    return mObjectives[idx];
}

double OptimizationHandler::getOptVar(QString &var, bool &ok) const
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
    else if(var == "algorithm")
    {
        return mAlgorithm;
    }
    else if(var == "datatype")
    {
        return mParameterType;
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
    else if(var == "alpha")
    {
        return mCrfAlpha;
    }
    else if(var == "rfak")
    {
        return mCrfRfak;
    }
    else if(var == "gamma")
    {
        return mCrfGamma;
    }
    else if(var == "evalid")
    {
        return mEvalId;
    }
    else if(var == "omega")
    {
        return mPsOmega;
    }
    else if(var == "c1")
    {
        return mPsC1;
    }
    else if(var == "c2")
    {
        return mPsC2;
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

void OptimizationHandler::setOptVar(const QString &var, const QString &value, bool &ok)
{
    ok=true;
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
    else if(var == "algorithm")
    {
        if(value == "complex")
            mAlgorithm = OptimizationHandler::Complex;
        else if(value == "particleswarm")
            mAlgorithm = OptimizationHandler::ParticleSwarm;
    }
    else if(var == "datatype")
    {
        if(value == "double")
            mParameterType = OptimizationHandler::Double;
        else if(value == "int")
            mParameterType = OptimizationHandler::Int;
    }
    else if(var == "npoints")
    {
        mNumPoints = value.toInt();
        mParameters.resize(mNumPoints);
    }
    else if(var == "nparams")
    {
        mNumParameters = value.toInt();
        mParMin.resize(mNumParameters);
        mParMax.resize(mNumParameters);
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
    else if(var == "alpha")
    {
        mCrfAlpha = value.toDouble();
    }
    else if(var == "rfak")
    {
        mCrfRfak = value.toDouble();
    }
    else if(var == "gamma")
    {
        mCrfGamma = value.toDouble();
    }
    else if(var == "evalid")
    {
        mEvalId = value.toDouble();
    }
    else if(var == "omega")
    {
        mPsOmega = value.toDouble();
    }
    else if(var == "c1")
    {
        mPsC1 = value.toDouble();
    }
    else if(var == "c2")
    {
        mPsC2 = value.toDouble();
    }
    else
    {
        ok=false;
    }
}

double OptimizationHandler::getParameter(const int pointIdx, const int parIdx) const
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


//! @brief Initializes a Complex-RF optimization
void OptimizationHandler::crfInit()
{
    mpHcomHandler->setModelPtr(mModelPtrs[0]);

    //Load default optimization functions
    QString oldPath = mpHcomHandler->getWorkingDirectory();
    mpHcomHandler->setWorkingDirectory(gpDesktopHandler->getExecPath());
    //executeCommand("exec ../Scripts/HCOM/optDefaultFunctions.hcom");
    QFile testFile1(gpDesktopHandler->getScriptsPath()+"/HCOM/optDefaultFunctions.hcom");
    QFile testFile2(gpDesktopHandler->getExecPath()+"../Scripts/HCOM/optDefaultFunctions.hcom");
    if(testFile1.exists())
    {
        mpHcomHandler->executeCommand("exec "+testFile1.fileName());
    }
    else if(testFile2.exists())
    {
        mpHcomHandler->executeCommand("exec "+testFile2.fileName());
    }
    else
    {
        mpConsole->printErrorMessage("Cannot find optimization default functions script file.","",false);
        return;
    }
    mpHcomHandler->setWorkingDirectory(oldPath);

    for(int p=0; p<mNumPoints; ++p)
    {
        mParameters[p].resize(mNumParameters);
        for(int i=0; i<mNumParameters; ++i)
        {
            double r = (double)rand() / (double)RAND_MAX;
            mParameters[p][i] = mParMin[i] + r*(mParMax[i]-mParMin[i]);
            if(mParameterType == Int)
            {
                mParameters[p][i] = round(mParameters[p][i]);
            }
        }
    }
    mObjectives.resize(mNumPoints);

    mCrfKf = 1.0-pow(mCrfAlpha/2.0, mCrfGamma/mNumPoints);

//    if(!gpModelHandler->getCurrentModel()->isSaved())
//    {
//        mpConsole->printErrorMessage("Current model is not saved. Please save it before running an optimization.", "", false);
//        return;
//    }

    LogDataHandler *pHandler = mpHcomHandler->getModelPtr()->getViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasLogVariableData("WorstObjective"))
    {
        pHandler->deleteVariable("WorstObjective");
    }
    if (pHandler->hasLogVariableData("BestObjective"))
    {
        pHandler->deleteVariable("BestObjective");
    }

    // Close these plotwindows before optimization to make sure old data is removed
    //! @todo should have define or const for this name "parplot"
    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow("parplot");
    if(pPlotWindow)
    {
        PlotTab *pPlotTab = pPlotWindow->getCurrentPlotTab();
        if(pPlotTab)
        {
            while(!pPlotTab->getCurves().isEmpty())
            {
                pPlotTab->removeCurve(pPlotTab->getCurves()[0]);
            }
        }
    }
}


//! @brief Executes a Complex-RF optimization. optComplexInit() must be called before this one.
void OptimizationHandler::crfRun()
{
    TicToc timer;
    //Plot optimization points
    plotPoints();
    timer.toc("PlotPoints");

    mpConsole->mpTerminal->setEnabledAbortButton(true);

    //Reset convergence reason variable (0 = failed to converge)
    mConvergenceReason=0;

    //Verify that everything is ok
    if(mAlgorithm == Uninitialized)
    {
        mpConsole->printErrorMessage("Optimization not initialized.", "", false);
        return;
    }
    if(!mpHcomHandler->hasFunction("evalall"))
    {
        mpConsole->printErrorMessage("Function \"evalall\" not defined.","",false);
        return;
    }
    if(!mpHcomHandler->hasFunction("evalworst"))
    {
        mpConsole->printErrorMessage("Function \"evalworst\" not defined.","",false);
        return;
    }

    mpConsole->print("Running optimization...");

    //Turn of terminal output during optimization
    mpHcomHandler->executeCommand("echo on");

    //Evaluate initial objevtive values
    timer.tic();
    mpHcomHandler->executeCommand("call evalall");
    timer.toc("call evalall");

    //Calculate best and worst id, and initialize last worst id
    timer.tic();
    calculatebestandworstid();
    mLastWorstId = mWorstId;
    timer.toc("optComplexCalculatebestandworstid");

    //Store parameters for undo
    timer.tic();
    mOldParameters = mParameters;
    timer.toc("Copy opt parameters");

    //Run optimization loop
    TicToc timer2;
    int i=0;
    int percent=-1;
    for(; i<mMaxEvals && !mpHcomHandler->isAborted(); ++i)
    {
        timer2.tic(QString("******************* Starting OptLoop %1").arg(i));

        //Plot optimization points
        plotPoints();

        //Process UI events (required so that we don't lock up the program)
        qApp->processEvents();

        //Stop if user pressed abort button
        if(mpHcomHandler->isAborted())
        {
            mpConsole->print("Optimization aborted.");
            //gpModelHandler->setCurrentModel(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
            finalize();
            return;
        }

        //Print progress as percentage of maximum number of evaluations
        int dummy=int(100.0*double(i)/mMaxEvals);
        if(dummy != percent)    //Only update at whole numbers
        {
//            mpConsole->setDontPrint(false);
//            mpConsole->print(QString::number(dummy)+"%");
//            mpConsole->setDontPrint(true);
            percent = dummy;
            gpMainWindow->mpOptimizationDialog->updateTotalProgressBar(dummy);
        }

        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        crfForget();

        //Calculate best and worst point
        calculatebestandworstid();
        int wid = mWorstId;

        //Plot best and worst objective values
        plotObjectiveFunctionValues();

        //Find geometrical center
        crfFindcenter();

        //Reflect worst point
        QVector<double> newPoint;
        newPoint.resize(mNumParameters);
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[wid][j];
            mParameters[wid][j] = mCrfCenter[j] + (mCrfCenter[j]-worst)*mCrfAlpha;

            //Add some random noise
            double maxDiff = crfMaxpardiff();
            double r = (double)rand() / (double)RAND_MAX;
            mParameters[wid][j] = mParameters[wid][j] + mCrfRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mParameters[wid][j] = min(mParameters[wid][j], mParMax[j]);
            mParameters[wid][j] = max(mParameters[wid][j], mParMin[j]);
        }
        newPoint = mParameters[wid]; //Remember the new point, in case we need to iterate below

        //Evaluate new point
        TicToc timer;
        timer.tic("+++++++++ Begin Evaluate new point");
        mpHcomHandler->executeCommand("call evalworst");
        if(mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
        {
            mpHcomHandler->executeCommand("echo on");
            mpConsole->print("Optimization aborted.");
            //gpModelHandler->setCurrentModel(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
            finalize();
            return;
        }
        timer.toc("+++++++++ End Evaluate new point");

        //Calculate best and worst points
        mLastWorstId=wid;
        calculatebestandworstid();
        wid = mWorstId;

        //Iterate until worst point is no longer the same
        timer.tic("--------- Begin Iterate until worst point is no longer the same");
        mCrfWorstCounter=0;
        while(mLastWorstId == wid)
        {
            //mpHcomHandler->executeCommand("echo on");
            plotPoints();

            qApp->processEvents();
            if(mpHcomHandler->isAborted())
            {
                mpHcomHandler->executeCommand("echo on");
                mpConsole->print("Optimization aborted.");
                //gpModelHandler->setCurrentModel(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
                finalize();
                mpHcomHandler->abortHCOM();
                return;
            }

            if(i>mMaxEvals) break;

            double a1 = 1.0-exp(-double(mCrfWorstCounter)/5.0);

            //Reflect worst point
            for(int j=0; j<mNumParameters; ++j)
            {
                double best = mParameters[mBestId][j];
                double maxDiff = crfMaxpardiff();
                double r = (double)rand() / (double)RAND_MAX;
                mParameters[wid][j] = (mCrfCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mCrfRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
                mParameters[wid][j] = min(mParameters[wid][j], mParMax[j]);
                mParameters[wid][j] = max(mParameters[wid][j], mParMin[j]);
            }
            newPoint = mParameters[wid];
            gpMainWindow->mpOptimizationDialog->updateParameterOutputs(mParameters, mBestId, mWorstId);

            //Evaluate new point
            mpHcomHandler->executeCommand("call evalworst");
            if(mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
            {
                mpHcomHandler->executeCommand("echo on");
                mpConsole->print("Optimization aborted.");
                //gpModelHandler->setCurrentModel(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
                finalize();
                return;
            }

            //Calculate best and worst points
            mLastWorstId=wid;
            calculatebestandworstid();
            wid = mWorstId;

            ++mCrfWorstCounter;
            ++i;
            //mpHcomHandler->executeCommand("echo off");
        }

        plotParameters();

        timer.toc("--------- End Iterate until worst point is no longer the same");
        timer2.toc(QString("******************* OptLoop %1").arg(i));
        qDebug() << "\n";
    }

    mpHcomHandler->executeCommand("echo on");

    switch(mConvergenceReason)
    {
    case 0:
        mpConsole->print("Optimization failed to converge after "+QString::number(i)+" iterations.");
        break;
    case 1:
        mpConsole->print("Optimization converged in function values after "+QString::number(i)+" iterations.");
        break;
    case 2:
        mpConsole->print("Optimization converged in parameter values after "+QString::number(i)+" iterations.");
        break;
    }

    mpConsole->print("\nBest point:");
    for(int i=0; i<mNumParameters; ++i)
    {
        mpConsole->print("par("+QString::number(i)+"): "+QString::number(mParameters[mBestId][i]));
    }

    // Clean up
    finalize();

    return;
}


//! @brief Applies the forgetting principle in complex algorithm
void OptimizationHandler::crfForget()
{
    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    for(int i=0; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    for(int i=0; i<mObjectives[0]; ++i)
    {
        mObjectives[0] = mObjectives[0]+(maxObj-minObj)*mCrfKf;
    }
}


//! @brief Calculates indexes of best and worst point
void OptimizationHandler::calculatebestandworstid()
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


//! @brief Calculates center point for complex algorithm
void OptimizationHandler::crfFindcenter()
{
    mCrfCenter.resize(mNumParameters);
    for(int i=0; i<mCrfCenter.size(); ++i)
    {
        mCrfCenter[i] = 0;
    }
    for(int p=0; p<mNumPoints; ++p)
    {
        for(int i=0; i<mNumParameters; ++i)
        {
            mCrfCenter[i] = mCrfCenter[i]+mParameters[p][i];
        }
    }
    for(int i=0; i<mCrfCenter.size(); ++i)
    {
        mCrfCenter[i] = mCrfCenter[i]/double(mNumPoints);
    }
}


//! @brief Checkes for convergence (in either of the algorithms)
bool OptimizationHandler::checkForConvergence()
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
    double maxDiff=crfMaxpardiff();
    if(fabs(maxDiff) < mParTol)
    {
        mConvergenceReason=2;
        return true;
    }
    return false;
}


//! @brief Returns the maximum difference between smallest and largest parameter
double OptimizationHandler::crfMaxpardiff()
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


//! @brief Initializes a particle swarm optimization
void OptimizationHandler::psInit()
{
    mpHcomHandler->setModelPtr(mModelPtrs.first());//gpModelHandler->setCurrentModel(mOptModelPtrs.first());

    //Load default optimization functions
    QString oldPath = mpHcomHandler->getWorkingDirectory();
    mpHcomHandler->setWorkingDirectory(gpDesktopHandler->getExecPath());
    mpHcomHandler->executeCommand("exec ../Scripts/HCOM/optDefaultFunctions.hcom");
    mpHcomHandler->setWorkingDirectory(oldPath);

    //if(mOptMulticore)
   // {
        //QString modelPath = mpHcomHandler->getModelPtr()->getViewContainerObject()->getModelFileInfo().filePath();
        //mpHcomHandler->getModelPtr()->save();
        //gpModelHandler->closeAllModels();
//        for(int i=0; i<mOptNumPoints; ++i)
//        {
//            mOptModelPtrs.append(gpModelHandler->loadModel(modelPath, true, true));
//        }
    //}

    for(int p=0; p<mNumPoints; ++p)
    {
        mParameters[p].resize(mNumParameters);
        mPsVelocities[p].resize(mNumParameters);
        for(int i=0; i<mNumParameters; ++i)
        {
            //Initialize points
            double r = double(rand()) / double(RAND_MAX);
            mParameters[p][i] = mParMin[i] + r*(mParMax[i]-mParMin[i]);
            if(mParameterType == Int)
            {
                mParameters[p][i] = round(mParameters[p][i]);
            }

            //Initialize velocities
            double minVel = -fabs(mParMax[i]-mParMin[i]);
            double maxVel = fabs(mParMax[i]-mParMin[i]);
            r = double(rand()) / double(RAND_MAX);
            mPsVelocities[p][i] = minVel + r*(maxVel-minVel);
        }
    }
    mObjectives.resize(mNumPoints);

    LogDataHandler *pHandler = mpHcomHandler->getModelPtr()->getViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasLogVariableData("WorstObjective"))
    {
        pHandler->deleteVariable("WorstObjective");
    }
    if (pHandler->hasLogVariableData("BestObjective"))
    {
        pHandler->deleteVariable("BestObjective");
    }

    // Close these plotwindows before optimization to make sure old data is removed
    //! @todo should have define or const for this name "parplot"
    //gpPlotHandler->closeWindow("parplot");
    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow("parplot");
    if(pPlotWindow)
    {
        PlotTab *pPlotTab = pPlotWindow->getCurrentPlotTab();
        if(pPlotTab)
        {
            while(!pPlotTab->getCurves().isEmpty())
            {
                pPlotTab->removeCurve(pPlotTab->getCurves()[0]);
            }
        }
    }
}


//! @brief Executes a particle swarm algorithm. optParticleInit() must be called before this one.
void OptimizationHandler::psRun()
{
    plotPoints();

    mpConsole->mpTerminal->setEnabledAbortButton(true);

    mConvergenceReason=0;

    if(mAlgorithm == Uninitialized)
    {
        mpConsole->printErrorMessage("Optimization not initialized.", "", false);
        return;
    }
    if(!mpHcomHandler->hasFunction("evalall"))
    {
        mpConsole->printErrorMessage("Function \"evalall\" not defined.","",false);
        return;
    }

    mpConsole->print("Running optimization...");

    //Disable terminal output during optimization
    mpHcomHandler->executeCommand("echo off");

    //Evaluate initial objevtive values
    mpHcomHandler->executeCommand("call evalall");
    if(mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
    {
        mpHcomHandler->executeCommand("echo on");
        mpConsole->print("Optimization aborted.");
        //mpHcomHandler->setModelPtr(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
        finalize();
        return;
    }

    //Initialize best known point for each point
    for(int i=0; i<mNumPoints; ++i)
    {
        mPsBestKnowns[i] = mParameters[i];
        mPsBestObjectives[i] = mObjectives[i];
    }

    //Calculate best known global position
    calculatebestandworstid();
    mPsBestObj = mObjectives[mBestId];
    mPsBestPoint = mParameters[mBestId];

    int i=0;
    int percent=-1;
    for(; i<mMaxEvals && !mpHcomHandler->isAborted(); ++i)
    {
        //Process events, to make sure GUI is updated
        qApp->processEvents();

        //Abort if abort key was pressed
        if(mpHcomHandler->isAborted())
        {
            mpConsole->print("Optimization aborted.");
            //gpModelHandler->setCurrentModel(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
            finalize();
            return;
        }

        //Print log output
        psPrintLogOutput();

        //Print progress as percentage of maximum number of evaluations
        int dummy=int(100.0*double(i)/mMaxEvals);
        if(dummy != percent)    //Only update at whole numbers
        {
//            mpConsole->setDontPrint(false);
//            mpConsole->print(QString::number(dummy)+"%");
//            mpConsole->setDontPrint(true);
            percent = dummy;
            gpMainWindow->mpOptimizationDialog->updateTotalProgressBar(dummy);
        }

        //Move particles
        psMoveParticles();

        //Evaluate objevtive values
        if(mpConfig->getUseMulticore())
        {
            //Multi-threading, we cannot use the "evalall" function
            for(int i=0; i<mNumPoints && !mpHcomHandler->isAborted(); ++i)
            {
                mpHcomHandler->setModelPtr(mModelPtrs[i]);
                mpHcomHandler->executeCommand("opt set evalid "+QString::number(i));
                mpHcomHandler->executeCommand("call setpars");
            }
            gpModelHandler->simulateMultipleModels_blocking(mModelPtrs); //Ok to use global model handler for this, it does not use any member stuff
            for(int i=0; i<mNumPoints && !mpHcomHandler->isAborted(); ++i)
            {
                mpHcomHandler->setModelPtr(mModelPtrs[i]);
                mpHcomHandler->executeCommand("opt set evalid "+QString::number(i));
                mpHcomHandler->executeCommand("call obj");
            }
            mpHcomHandler->setModelPtr(mModelPtrs.first());
        }
        else
        {
            mpHcomHandler->executeCommand("call evalall");
        }
        if(mpHcomHandler->getVar("ans") == -1 || mpHcomHandler->isAborted())    //This check is needed if abort key is pressed while evaluating
        {
            mpHcomHandler->executeCommand("echo on");
            mpConsole->print("Optimization aborted.");
           //mpHcomHandler->setModelPtr(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
            finalize();
            return;
        }

        //Calculate best known positions
        for(int p=0; p<mNumPoints; ++p)
        {
            if(mObjectives[p] < mPsBestObjectives[p])
            {
                mPsBestKnowns[p] = mParameters[p];
                mPsBestObjectives[p] = mObjectives[p];
            }
        }

        //Calculate best known global position
        calculatebestandworstid();
        if(mObjectives[mBestId] < mPsBestObj)
        {
            mPsBestObj = mObjectives[mBestId];
            mPsBestPoint = mParameters[mBestId];
        }
        gpMainWindow->mpOptimizationDialog->updateParameterOutputs(mParameters, mBestId, mWorstId);

        plotPoints();
        plotObjectiveFunctionValues();

        //Check convergence
        if(checkForConvergence()) break;      //Use complex method, it's the same principle
    }

    mpHcomHandler->executeCommand("echo on");

    switch(mConvergenceReason)
    {
    case 0:
        mpConsole->print("Optimization failed to converge after "+QString::number(i)+" iterations.");
        break;
    case 1:
        mpConsole->print("Optimization converged in function values after "+QString::number(i)+" iterations.");
        break;
    case 2:
        mpConsole->print("Optimization converged in parameter values after "+QString::number(i)+" iterations.");
        break;
    }

    mpConsole->print("\nBest point:");
    for(int i=0; i<mNumParameters; ++i)
    {
        mpConsole->print("par("+QString::number(i)+"): "+QString::number(mParameters[mBestId][i]));
    }

    //Clean up
    finalize();
}


//! @brief Prints logging output about particles (for use with particle swarm algorithm)
//! @todo Extent so it can also be used with complex algorithm
void OptimizationHandler::psPrintLogOutput()
{
    if(mPsPrintLogOutput)
    {
        if(mPsLogOutput.isEmpty())
        {
            //Prepare logging output list
            for(int p=0; p<mNumPoints; ++p)
            {
                mPsLogOutput.append("Particle    "+QString::number(p)+":\n");
                mPsLogOutput[p].append("Position: \t\t\tVelocity: \t\t\tLocal best: \t\t\tGlobal best:\n");

            }
        }

        for(int p=0; p<mNumPoints; ++p)
        {
            for(int i=0; i<mParameters[p].size(); ++i)
            {
                mPsLogOutput[p].append(QString::number(mParameters[p][i])+",");
            }
            mPsLogOutput[p].chop(1);
            mPsLogOutput[p].append("\t\t");
            for(int i=0; i<mPsVelocities[p].size(); ++i)
            {
                mPsLogOutput[p].append(QString::number(mPsVelocities[p][i])+",");
            }
            mPsLogOutput[p].chop(1);
            mPsLogOutput[p].append("\t\t");
            for(int i=0; i<mPsBestKnowns[p].size(); ++i)
            {
                mPsLogOutput[p].append(QString::number(mPsBestKnowns[p][i])+",");
            }
            mPsLogOutput[p].chop(1);
            mPsLogOutput[p].append("\t\t");
            for(int i=0; i<mPsBestPoint.size(); ++i)
            {
                mPsLogOutput[p].append(QString::number(mPsBestPoint[i])+",");
            }
            mPsLogOutput[p].chop(1);
            mPsLogOutput[p].append("\n");
        }
    }
}


//! @brief Plots the optimization points (if there are at least two parameters and the option is selected)
void OptimizationHandler::plotPoints()
{
    if(!mPlotPoints) { return; }

    if(mNumParameters < 2)
    {
        mpConsole->printErrorMessage("Plotting points requires at least two parameters.");
        return;
    }

    LogDataHandler *pHandler = mpHcomHandler->getModelPtr()->getViewContainerObject()->getLogDataHandler();
    for(int p=0; p<mNumPoints; ++p)
    {
        QString namex = "par"+QString::number(p)+"x";
        QString namey = "par"+QString::number(p)+"y";
        double x = mParameters[p][0];
        double y = mParameters[p][1];
        SharedLogVariableDataPtrT parVar_x = pHandler->getLogVariableDataPtr(namex, -1);
        SharedLogVariableDataPtrT parVar_y = pHandler->getLogVariableDataPtr(namey, -1);
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
        for(int c=0; c<pTab->getCurves(FirstPlot).size(); ++c)
        {
            if(c==mBestId)
            {
                pTab->getCurves(FirstPlot).at(c)->setLineSymbol("Star 1");
            }
            else
            {
                pTab->getCurves(FirstPlot).at(c)->setLineSymbol("XCross");
            }
        }
        pTab->update();
    }
}


//! @brief Plots best and worst objective values (if option is selected)
void OptimizationHandler::plotObjectiveFunctionValues()
{
    if(!mPlotObjectiveFunctionValues) { return; }

    LogDataHandler *pHandler = mpHcomHandler->getModelPtr()->getViewContainerObject()->getLogDataHandler();
    SharedLogVariableDataPtrT bestVar = pHandler->getLogVariableDataPtr("BestObjective", -1);
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
    SharedLogVariableDataPtrT worstVar = pHandler->getLogVariableDataPtr("WorstObjective", -1);
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
void OptimizationHandler::plotParameters()
{
    if(!mPlotParameters) { return; }

    LogDataHandler *pHandler = mpHcomHandler->getModelPtr()->getViewContainerObject()->getLogDataHandler();
    for(int p=0; p<mNumParameters; ++p)
    {
        SharedLogVariableDataPtrT par = pHandler->getLogVariableDataPtr("NewPar"+QString::number(p), -1);
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
            gpPlotHandler->plotDataToWindow(pPW, par, 0);
        }
    }
}

void OptimizationHandler::finalize()
{
//    while(!mModelPtrs.isEmpty())
//    {
//        mModelPtrs[0]->close();
//        delete mModelPtrs[0];
//        mModelPtrs.remove(0);
//    }

    mpConsole->mpTerminal->setEnabledAbortButton(false);

    gpMainWindow->mpOptimizationDialog->setOptimizationFinished();

    emit optimizationFinished();
}



//! @brief Moves the particles (for particle swarm optimization)
void OptimizationHandler::psMoveParticles()
{
    for (int p=0; p<mNumPoints; ++p)
    {
        double r1 = double(rand())/double(RAND_MAX);
        double r2 = double(rand())/double(RAND_MAX);
        for(int j=0; j<mNumParameters; ++j)
        {
            mPsVelocities[p][j] = mPsOmega*mPsVelocities[p][j] + mPsC1*r1*(mPsBestKnowns[p][j]-mParameters[p][j]) + mPsC2*r2*(mPsBestPoint[j]-mParameters[p][j]);
            mParameters[p][j] = mParameters[p][j]+mPsVelocities[p][j];
            if(mParameters[p][j] <= mParMin[j])
            {
                mParameters[p][j] = mParMin[j];
                mPsVelocities[p][j] = 0.0;
            }
            if(mParameters[p][j] >= mParMax[j])
            {
                mParameters[p][j] = mParMax[j];
                mPsVelocities[p][j] = 0.0;
            }
        }
    }
}





