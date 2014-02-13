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
//! @file   OptimizationWorkerComplexRFP.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id: OptimizationHandler.cpp 6525 2014-01-30 15:58:59Z petno25 $
//!
//! @brief Contains an optimization worker object for the Complex-RFP algorithm
//!

//Hopsan includes
#include "Configuration.h"
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "HcomHandler.h"
#include "LogDataHandler.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "OptimizationHandler.h"
#include "OptimizationWorkerComplex.h"
#include "OptimizationWorkerComplexRFP.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"

//C++ includes
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#endif

OptimizationWorkerComplexRFP::OptimizationWorkerComplexRFP(OptimizationHandler *pHandler)
    : OptimizationWorkerComplex(pHandler)
{

}


//! @brief Initializes a Complex-RF optimization
void OptimizationWorkerComplexRFP::init()
{
    OptimizationWorkerComplex::init();


    mLastWorstId = -1;
    mWorstCounter = 0;

    for(int p=0; p<mNumPoints; ++p)
    {
        mParameters[p].resize(mNumParameters);
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

    mNumThreads = gpConfig->getNumberOfThreads();
    if(mNumThreads == 0)
    {
#ifdef WIN32
        std::string temp = getenv("NUMBER_OF_PROCESSORS");
        nThreads = atoi(temp.c_str());
#else
        mNumThreads = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    }

    mCandidateParticles.resize(mNumThreads);
    for(int i=0; i<mCandidateParticles.size(); ++i)
    {
        mCandidateParticles[i].resize(mNumParameters);
    }

    mObjectives.resize(mNumPoints+mNumThreads);

    mKf = 1.0-pow(mAlpha1/2.0, mGamma/mNumPoints);

    LogDataHandler *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasVariable("WorstObjective"))
    {
        pHandler->deleteVariable("WorstObjective");
    }
    if (pHandler->hasVariable("BestObjective"))
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
                pPlotTab->removeCurve(pPlotTab->getCurves().first());
            }
        }
    }
}



//! @brief Executes a Complex-RF optimization. optComplexInit() must be called before this one.
void OptimizationWorkerComplexRFP::run()
{
    //Plot optimization points
    plotPoints();

    mpHandler->mpConsole->mpTerminal->setEnabledAbortButton(true);

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

    print("Running optimization...");

    //Turn of terminal output during optimization
    execute("echo off");

    execute("call evalall");

    findCenter();

    calculateBestAndWorstId();

    pickCandidateParticles();

    //Evaluate initial objevtive values
    evaluateCandidateParticles();

    examineCandidateParticles();

    //Calculate best and worst id, and initialize last worst id
    calculateBestAndWorstId();
    mLastWorstId = mWorstId;

    //Store parameters for undo
    mParameters = mParameters;

    //Run optimization loop
    int i=0;
    int percent=-1;
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
        int dummy=int(100.0*double(i)/mMaxEvals);
        if(dummy != percent)    //Only update at whole numbers
        {
            percent = dummy;
            gpMainWindow->mpOptimizationDialog->updateTotalProgressBar(dummy);
        }

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
        pickCandidateParticles();

        gpMainWindow->mpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

        //Evaluate new point
        evaluateCandidateParticles();
        if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
        {
            execute("echo on");
            print("Optimization aborted.");
            finalize();
            return;
        }

        examineCandidateParticles();

        //Calculate best and worst points
        mLastWorstId=wid;
        calculateBestAndWorstId();
        wid = mWorstId;

        //Iterate until worst point is no longer the same
        //! @todo Implement (if necessary)
//        mWorstCounter=0;
//        while(mLastWorstId == wid)
//        {
//            plotPoints();

//            qApp->processEvents();
//            if(mpHandler->mpHcomHandler->isAborted())
//            {
//                execute("echo on");
//                print("Optimization aborted.");
//                finalize();
//                mpHandler->mpHcomHandler->abortHCOM();
//                return;
//            }

//            if(i>mMaxEvals) break;

//            double a1 = 1.0-exp(-double(mWorstCounter)/5.0);

//            //Reflect worst point
//            for(int j=0; j<mNumParameters; ++j)
//            {
//                double best = mParameters[mBestId][j];
//                double maxDiff = getMaxParDiff();
//                double r = (double)rand() / (double)RAND_MAX;
//                mParameters[wid][j] = (mCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
//                mParameters[wid][j] = min(mParameters[wid][j], mParMax[j]);
//                mParameters[wid][j] = max(mParameters[wid][j], mParMin[j]);
//            }
//            newPoint = mParameters[wid];
//            gpMainWindow->mpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

//            //Evaluate new point
//            execute("call evalworst");
//            if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
//            {
//                execute("echo on");
//                print("Optimization aborted.");
//                finalize();
//                return;
//            }

//            //Calculate best and worst points
//            mLastWorstId=wid;
//            calculateBestAndWorstId();
//            wid = mWorstId;

//            ++mWorstCounter;
//            ++i;
//            execute("echo off");
//        }

        plotParameters();
    }

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

    print("\nBest point:");
    for(int i=0; i<mNumParameters; ++i)
    {
        print("par("+QString::number(i)+"): "+QString::number(mParameters[mBestId][i]));
    }

    // Clean up
    finalize();

    return;
}

void OptimizationWorkerComplexRFP::finalize()
{
    OptimizationWorkerComplex::finalize();
}


void OptimizationWorkerComplexRFP::setOptVar(const QString &var, const QString &value)
{
    OptimizationWorkerComplex::setOptVar(var, value);

    if(var == "alpha1")
    {
        mAlpha1 = value.toDouble();
    }
    else if(var == "alpha2")
    {
        mAlpha2 = value.toDouble();
    }
    else if(var == "alpha3")
    {
        mAlpha3 = value.toDouble();
    }
}

double OptimizationWorkerComplexRFP::getOptVar(const QString &var, bool &ok)
{
    double retval = OptimizationWorkerComplex::getOptVar(var, ok);
    if(ok)
    {
        return retval;
    }

    ok = true;
    if(var == "alpha1")
    {
        return mAlpha1;
    }
    else if(var == "alpha2")
    {
        return mAlpha2;
    }
    else if(var == "alpha3")
    {
        return mAlpha3;
    }
    else
    {
        ok = false;
        return 0;
    }
}

double OptimizationWorkerComplexRFP::getParameter(const int pointIdx, const int parIdx) const
{
    if(mParameters.size()+mCandidateParticles.size() < pointIdx+1)
    {
        return 0;
    }
    else if(pointIdx < mParameters.size() && mParameters[pointIdx].size() >= parIdx)
    {
        return mParameters[pointIdx][parIdx];
    }
    else if(pointIdx >= mParameters.size() && mCandidateParticles[pointIdx-mNumPoints].size() >= parIdx)
    {
        return mCandidateParticles[pointIdx-mNumPoints][parIdx];
    }
    return 0;
}



void OptimizationWorkerComplexRFP::pickCandidateParticles()
{
    //Reflect first point
    for(int j=0; j<mNumParameters; ++j)
    {
        //Reflect
        double worst = mParameters[mWorstId][j];
        mCandidateParticles[0][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha1;

        //Add some random noise
        double maxDiff = getMaxParDiff();
        double r = (double)rand() / (double)RAND_MAX;
        mCandidateParticles[0][j] = mCandidateParticles[0][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
        mCandidateParticles[0][j] = min(mCandidateParticles[0][j], mParMax[j]);
        mCandidateParticles[0][j] = max(mCandidateParticles[0][j], mParMin[j]);
    }

    if(mNumThreads > 1)
    {
        //Reflect second point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mWorstId][j];
            mCandidateParticles[1][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha2;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[1][j] = mCandidateParticles[1][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[1][j] = min(mCandidateParticles[1][j], mParMax[j]);
            mCandidateParticles[1][j] = max(mCandidateParticles[1][j], mParMin[j]);
        }
    }

    if(mNumThreads > 2)
    {
        //Reflect second point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mWorstId][j];
            mCandidateParticles[2][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha3;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[2][j] = mCandidateParticles[2][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[2][j] = min(mCandidateParticles[2][j], mParMax[j]);
            mCandidateParticles[2][j] = max(mCandidateParticles[2][j], mParMin[j]);
        }
    }

    //Create random scout particles
    for(int i=3; i<mNumThreads; ++i)
    {
        generateRandomParticleWeightedToCenter(mCandidateParticles[i]);
    }
}


void OptimizationWorkerComplexRFP::evaluateCandidateParticles()
{
    //Multi-threading, we cannot use the "evalall" function
    for(int i=0; i<mCandidateParticles.size() && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[i]);
        execute("opt set evalid "+QString::number(i+mNumPoints));
        execute("call setpars");
    }
    gpModelHandler->simulateMultipleModels_blocking(mModelPtrs); //Ok to use global model handler for this, it does not use any member stuff
    for(int i=0; i<mCandidateParticles.size() && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[i]);
        execute("opt set evalid "+QString::number(i+mNumPoints));
        execute("call obj");
    }
    mpHandler->mpHcomHandler->setModelPtr(mModelPtrs.first());
}


void OptimizationWorkerComplexRFP::examineCandidateParticles()
{
    bool didSomething=false;        //! @todo Us
    while(true)
    {
        int worstParticleIdx = 0;
        double worstParticle = mObjectives[0];
        for(int i=1; i<mNumPoints; ++i)
        {
            if(mObjectives[i] > worstParticle)
            {
                worstParticleIdx = i;
                worstParticle = mObjectives[i];
            }
        }

        int bestCandidateIdx = 0;
        double bestCandidate = mObjectives[mNumPoints];
        for(int i=mNumPoints; i<mNumPoints+mNumThreads; ++i)
        {
            if(mObjectives[i] < bestCandidate)
            {
                bestCandidateIdx = i-mNumPoints;
                bestCandidate = mObjectives[i];
            }
        }

        if(bestCandidate < worstParticle)
        {
            didSomething=true;
            QVector<double> tempParameters = mParameters[worstParticleIdx];
            double tempObjective = mObjectives[worstParticleIdx];
            mParameters[worstParticleIdx] = mCandidateParticles[bestCandidateIdx];
            mObjectives[worstParticleIdx] = mObjectives[mNumPoints+bestCandidateIdx];
            mCandidateParticles[bestCandidateIdx] = tempParameters;
            mObjectives[mNumPoints+bestCandidateIdx] = tempObjective;
        }
        else
        {
            break;
        }
    }
}


double OptimizationWorkerComplexRFP::triangularDistribution(double min, double mid, double max)
{
    double r = (double)rand() / (double)RAND_MAX;
    if(min+r*(max-min) > mid)     //Generate number larger than midpoint
    {
        double r1 = (double)rand()/(double)RAND_MAX;
        double r2 = (double)rand()/(double)RAND_MAX;
        return r1*(max-mid)-r2*(max-mid);
    }
    else            //Generate number smaller than midpoint
    {
        double r1 = (double)rand()/(double)RAND_MAX;
        double r2 = (double)rand()/(double)RAND_MAX;
        return r1*(mid-min)+r2*(mid-min);
    }
}


void OptimizationWorkerComplexRFP::generateRandomParticle(QVector<double> &rParticle)
{
    for(int i=0; i<mNumParameters; ++i)
    {
        double r = (double)rand() / (double)RAND_MAX;
        rParticle[i] = mParMin[i] + r*(mParMax[i]-mParMin[i]);
        if(mpHandler->mParameterType == OptimizationHandler::Integer)
        {
            rParticle[i] = round(rParticle[i]);
        }
    }
}

void OptimizationWorkerComplexRFP::generateRandomParticleWeightedToCenter(QVector<double> &rParticle)
{
    for(int i=0; i<mNumParameters; ++i)
    {
        rParticle[i] = triangularDistribution(mParMin[i], mCenter[i], mParMax[i]);
        if(mpHandler->mParameterType == OptimizationHandler::Integer)
        {
            rParticle[i] = round(rParticle[i]);
        }
    }
}
