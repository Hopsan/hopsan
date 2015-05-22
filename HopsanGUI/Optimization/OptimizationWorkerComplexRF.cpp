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
//! @file   OptimizationWorkerComplexRF.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Complex-RF algorithm
//!

//Hopsan includes
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "HcomHandler.h"
#include "OptimizationHandler.h"
#include "OptimizationWorkerComplexRF.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"

//C++ includes
#include <math.h>

OptimizationWorkerComplexRF::OptimizationWorkerComplexRF(OptimizationHandler *pHandler)
    : OptimizationWorkerComplex(pHandler)
{

}


//! @brief Initializes a Complex-RF optimization
void OptimizationWorkerComplexRF::init(const ModelWidget *pModel, const QString &modelPath)
{
    OptimizationWorkerComplex::init(pModel, modelPath);


    mLastWorstId = -1;
    mWorstCounter = 0;

    for(int p=0; p<mNumPoints; ++p)
    {
        mParameters[p].resize(mNumParameters);
        if(!mDontChangeStartValues)
        {
            for(int i=0; i<mNumParameters; ++i)
            {
                double r = (double)rand() / (double)RAND_MAX;
                mParameters[p][i] = mParMin[i] + r*(mParMax[i]-mParMin[i]);
                if(mpHandler->mParameterType == OptimizationHandler::Integer)
                {
                    mParameters[p][i] = round(mParameters[p][i]);
                }
            }
        }
    }
    mObjectives.resize(mNumPoints);

    mKf = 1.0-pow(mAlpha/2.0, mGamma/mNumPoints);

    LogDataHandler2 *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasVariable("WorstObjective"))
    {
        pHandler->removeVariable("WorstObjective", -1);
    }
    if (pHandler->hasVariable("BestObjective"))
    {
        pHandler->removeVariable("BestObjective", -1);
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
                pPlotTab->removeCurve(pPlotTab->getCurves().first());
            }
        }
    }
}



//! @brief Executes a Complex-RF optimization. optComplexInit() must be called before this one.
void OptimizationWorkerComplexRF::run()
{
    //Plot optimization points
    plotPoints();
    qDebug() << "Parnames: " << mParNames;

    mpHandler->mpHcomHandler->mpConsole->mpTerminal->setAbortButtonEnabled(true);

    //Reset convergence reason variable (0 = failed to converge)
    mConvergenceReason=0;

    //Verify that everything is ok
    if(!mpHandler->mpHcomHandler->hasFunction("evalall"))
    {
        printError("Function \"evalall\" not defined.","",false);
        return;
    }
    if(!mpHandler->mpHcomHandler->hasFunction("evalworst"))
    {
        printError("Function \"evalworst\" not defined.","",false);
        return;
    }

    print("Running optimization...", "", true);

    //Turn of terminal output during optimization
    execute("echo off -nonerrors");

    //Evaluate initial objective values
    execute("call evalall");
    logAllPoints();
    mEvaluations = mNumPoints;

    //Calculate best and worst id, and initialize last worst id
    calculateBestAndWorstId();
    mLastWorstId = mWorstId;

    //Store parameters for undo
    mParameters = mParameters;

    //Run optimization loop
    int i=0;
    for(; i<mMaxEvals && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        //Plot optimization points
        plotPoints();

        //Process UI events (required so that we don't lock up the program)
        qApp->processEvents();

        //Stop if user pressed abort button
        if(mpHandler->mpHcomHandler->isAborted())
        {
            print("Optimization aborted.");
            finalize();
            return;
        }

        //Print progress as percentage of maximum number of evaluations
        updateProgressBar(i);

        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        forget();

        //Calculate best and worst point
        calculateBestAndWorstId();
        int wid = mWorstId;

        //Plot best and worst objective values
        plotObjectiveFunctionValues();

        //Find geometrical center
        findCenter();

        //Reflect worst point
        QVector<double> newPoint;
        newPoint.resize(mNumParameters);
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[wid][j];
            mParameters[wid][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mParameters[wid][j] = mParameters[wid][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mParameters[wid][j] = qMin(mParameters[wid][j], mParMax[j]);
            mParameters[wid][j] = qMax(mParameters[wid][j], mParMin[j]);
        }
        newPoint = mParameters[wid]; //Remember the new point, in case we need to iterate below

        gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

        //Evaluate new point
        execute("call evalworst");

        ++mEvaluations;
        if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
        {
            execute("echo on");
            print("Optimization aborted.");
            finalize();
            return;
        }

        //Calculate best and worst points
        mLastWorstId=wid;
        calculateBestAndWorstId();
        wid = mWorstId;

        if(wid != mLastWorstId)
        {
            logPoint(mLastWorstId);
        }

        //Iterate until worst point is no longer the same
        mWorstCounter=0;
        while(mLastWorstId == wid)
        {
            plotPoints();

            qApp->processEvents();
            if(mpHandler->mpHcomHandler->isAborted())
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                mpHandler->mpHcomHandler->abortHCOM();
                return;
            }

            ++i;
            if(i>=mMaxEvals)
            {
                --i;    //Needed because for-loop will increase it by one anyway
                break;
            }

            double a1 = 1.0-exp(-double(mWorstCounter)/5.0);

            //Reflect worst point
            for(int j=0; j<mNumParameters; ++j)
            {
                double best = mParameters[mBestId][j];
                double maxDiff = getMaxParDiff();
                double r = (double)rand() / (double)RAND_MAX;
                mParameters[wid][j] = (mCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
                mParameters[wid][j] = qMin(mParameters[wid][j], mParMax[j]);
                mParameters[wid][j] = qMax(mParameters[wid][j], mParMin[j]);
            }
            newPoint = mParameters[wid];
            gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

            //Evaluate new point
            execute("call evalworst");

            ++mEvaluations;
            if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                return;
            }

            //Calculate best and worst points
            mLastWorstId=wid;
            calculateBestAndWorstId();
            wid = mWorstId;

            if(wid != mLastWorstId)
            {
                logPoint(mLastWorstId);
            }

            ++mWorstCounter;
            execute("echo off -nonerrors");

            updateProgressBar(i);
        }

        plotParameters();
        plotEntropy();
    }

    gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

    execute("echo on");

    switch(mConvergenceReason)
    {
    case 0:
        print("Optimization failed to converge after "+QString::number(i)+" iterations.");
        break;
    case 1:
        print("Optimization converged in function values after "+QString::number(i)+" iterations.");
        break;
    case 2:
        print("Optimization converged in parameter values after "+QString::number(i)+" iterations.");
        break;
    }

    mIterations = mEvaluations;

    print("\nBest point:");
    for(int i=0; i<mNumParameters; ++i)
    {
        if(mParNames.size() < i+1)
            print("par("+QString::number(i)+"): "+QString::number(mParameters[mBestId][i]));
        else
            print(mParNames[i]+": "+QString::number(mParameters[mBestId][i]));
    }

    // Clean up
    finalize();

    return;
}

void OptimizationWorkerComplexRF::finalize()
{
    OptimizationWorkerComplex::finalize();
}
