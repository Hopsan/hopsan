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
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Complex-RFP algorithm
//!

//Hopsan includes
#include "Configuration.h"
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "HcomHandler.h"
#include "ModelHandler.h"
#include "OptimizationHandler.h"
#include "OptimizationWorkerComplexRFP.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "PlotArea.h"
#include "PlotCurve.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "Utilities/GUIUtilities.h"

//C++ includes
#include <math.h>
#ifndef _WIN32
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

    mMethod = 1;

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

    mNumThreads = gpConfig->getIntegerSetting(CFG_NUMBEROFTHREADS);
    if(mNumThreads == 0)
    {
#ifdef _WIN32
        std::string temp = getenv("NUMBER_OF_PROCESSORS");
        mNumThreads = atoi(temp.c_str());
#else
        mNumThreads = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    }

//    if(mMethod == 1)
//    {
//        mNumThreads = max(mNumThreads, mNumPoints-2);
//    }

    mCandidateParticles.resize(mNumThreads);
    for(int i=0; i<mCandidateParticles.size(); ++i)
    {
        mCandidateParticles[i].resize(mNumParameters);
    }

    mObjectives.resize(mNumPoints+mNumThreads);

    //Limit number of models, in case worker has opened more models than necessary
    mUsedModelPtrs.clear();
    for(int i=0; i<mNumThreads; ++i)
    {
        mUsedModelPtrs.append(mModelPtrs.at(i));
    }

    mKf = 1.0-pow(mAlpha/2.0, mGamma/mNumPoints);

    LogDataHandler2 *pHandler = mUsedModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
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
void OptimizationWorkerComplexRFP::run()
{
    //Plot optimization points
    plotPoints();

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

    //Evaluate all points
    execute("call evalall");

//    for(int i=0; i<mNumPoints; ++i)
//    {
//        mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[i]);
//        execute("opt set evalid "+QString::number(i));
//        execute("call setpars");
//    }
//    gpModelHandler->simulateMultipleModels_blocking(mModelPtrs); //Ok to use global model handler for this, it does not use any member stuff
//    for(int i=0; i<mNumPoints && !mpHandler->mpHcomHandler->isAborted(); ++i)
//    {
//        mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[i]);
//        execute("opt set evalid "+QString::number(i));
//        execute("call obj");
//    }
    mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs.first());
    ++mIterations;
    logAllPoints();
    mEvaluations += mNumPoints;

    //Calculate best and worst id, and initialize last worst id
    calculateBestAndWorstId();
    mLastWorstId = mWorstId;

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
        //forget();

        //Calculate best and worst point
        calculateBestAndWorstId();
        int wid = mWorstId;

        //Plot best and worst objective values
        plotObjectiveFunctionValues();

        //Find geometrical center
        findCenter();

        //Reflect worst point
        pickCandidateParticles();

        //Evaluate new point
        evaluateCandidateParticles(i==0);
        if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
        {
            execute("echo on");
            print("Optimization aborted.");
            finalize();
            return;
        }

        examineCandidateParticles();

        gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

        //Calculate best and worst points
        mLastWorstId=wid;
        calculateBestAndWorstId();
        wid = mWorstId;

        //Iterate until worst point is no longer the same
        mWorstCounter=0;

        QVector<double> newPoint = mParameters[mWorstId];
        if(mNeedsIteration)
        {
            QVector< QVector<double> > otherPoints = mParameters;
            otherPoints.remove(mWorstId);
            findCenter(otherPoints);
        }
        while(mNeedsIteration)
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

            if(i>mMaxEvals) break;



            //Move reflected points
            for(int t=0; t<mNumThreads; ++t)
            {
                double a1 = 1.0-exp(-double(mWorstCounter)/5.0);
                for(int j=0; j<mNumParameters; ++j)
                {
                    double best = mParameters[mBestId][j];
                    double maxDiff = getMaxParDiff();
                    double r = (double)rand() / (double)RAND_MAX;
                    mCandidateParticles[t][j] = (mCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
                    mCandidateParticles[t][j] = qMin(mCandidateParticles[t][j], mParMax[j]);
                    mCandidateParticles[t][j] = qMax(mCandidateParticles[t][j], mParMin[j]);
                }

                newPoint = mCandidateParticles[t];
                ++mWorstCounter;
            }

            newPoint = mCandidateParticles.last();

            //Evaluate new point
            evaluateCandidateParticles();

            //Replace worst point with first candidate point that is better, if any
            for(int o=0; o<mNumThreads; ++o)
            {
                int nWorsePoints=0;
                for(int j=0; j<mNumPoints; ++j)
                {
                    if(mObjectives[j] > mObjectives[mNumPoints+o])
                    {
                        ++nWorsePoints;
                    }
                }

                if(nWorsePoints >= 2)
                {
                    mParameters[mWorstId] = mCandidateParticles[o];
                    mObjectives[mWorstId] = mObjectives[mNumPoints+o];
                    logWorstPoint();
                    mNeedsIteration = false;
                    break;
                }
            }


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

            ++i;
            execute("echo off -nonerrors");
        }

        gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

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
    if(mMethod==1)
    {
        //Sort ids by objective value (worst to best)
        mvIdx.clear();
        while(mvIdx.size() != mNumPoints)
        {
            int worstId = 0;
            double worstObjective = -1000000000;
            for(int i=0; i<mNumPoints; ++i)
            {
                if(mvIdx.contains(i)) continue;  //Ignore already added indexes

                if(mObjectives[i] > worstObjective)
                {
                    worstObjective = mObjectives[i];
                    worstId = i;
                }
            }
            mvIdx.append(worstId);
        }

        int i=0;
        findCenter();

//        double alpha1 = mAlpha;                 //1.3
//        double alpha2 = 1;                      //1
//        double alpha3 = (1-(mAlpha-1));         //0.7
//        double alpha4 = (1+(mAlpha-1)*2);       //1.6
//        double alpha5 = (1+(mAlpha-1)*3);       //1.9
//        double alpha6 = (1+(mAlpha-1)*4);       //2.2
//        double alpha7 = (1+(mAlpha-1)*5);       //2.5
//        double alpha8 = (1+(mAlpha-1)*6);       //2.8

        double alpha1 = mAlpha;                 //1.3
        double alpha2 = (1+(mAlpha-1)*2);       //1.6
        double alpha3 = (1+(mAlpha-1)*3);       //1.9
        double alpha4 = (1+(mAlpha-1)*4);       //2.2
        double alpha5 = (1+(mAlpha-1)*5);       //2.5
        double alpha6 = (1+(mAlpha-1)*6);       //2.8
        double alpha7 = (1+(mAlpha-1)*7);       //3.1
        double alpha8 = (1+(mAlpha-1)*8);       //3.4


        //Reflect first point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha1;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        ++i;

        if(i >= mNumThreads) return;

        //Reflect second point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha2;

            //Add some random noise
            double maxDiff = getMaxParDiff();   //! @todo Use correct min and max (including previous candidates)
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        ++i;


        if(i >= mNumThreads) return;

        //Reflect third point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha3;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        ++i;

        if(i >= mNumThreads) return;

        //Reflect forth point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha4;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        ++i;

        if(i >= mNumThreads) return;

        //Reflect fifth point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha5;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        ++i;

        if(i >= mNumThreads) return;

        //Reflect sixth point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha6;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        ++i;

        if(i >= mNumThreads) return;

        //Reflect seventh point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha7;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        ++i;

        if(i >= mNumThreads) return;

        //Reflect eigth point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[0]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*alpha8;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }
    }
    else if(mMethod==0)
    {
        //Sort ids by objective value (worst to best)
        mvIdx.clear();
        while(mvIdx.size() != mNumPoints)
        {
            int worstId = 0;
            double worstObjective = -1000000000;
            for(int i=0; i<mNumPoints; ++i)
            {
                if(mvIdx.contains(i)) continue;  //Ignore already added indexes

                if(mObjectives[i] > worstObjective)
                {
                    worstObjective = mObjectives[i];
                    worstId = i;
                }
            }
            mvIdx.append(worstId);
        }

        QList<int> nTests;
        int i=0;
        QVector< QVector<double> > otherPoints = mParameters;
        otherPoints.remove(mvIdx[i]);
        findCenter(otherPoints);

        //Reflect first point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[i]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        otherPoints.insert(mvIdx[i], mCandidateParticles[i]);

        ++i;
        otherPoints.remove(mvIdx[i]);
        findCenter(otherPoints);

        if(i >= mNumThreads) return;

        //Reflect second point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[i]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha;

            //Add some random noise
            double maxDiff = getMaxParDiff();   //! @todo Use correct min and max (including previous candidates)
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        otherPoints.insert(mvIdx[i], mCandidateParticles[i]);

        ++i;
        otherPoints.remove(mvIdx[i]);
        findCenter(otherPoints);

        if(i >= mNumThreads) return;

        //Reflect third point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[i]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        otherPoints.insert(mvIdx[i], mCandidateParticles[i]);

        ++i;
        otherPoints.remove(mvIdx[i]);
        findCenter(otherPoints);

        if(i >= mNumThreads) return;

        //Reflect forth point
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[mvIdx[i]][j];
            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha;

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
        }

        //! @todo Maybe reflect more points if more processors are available?

//        i=0;
//        if(i >= mNumThreads) return;

//        //Reflect third
//        for(int j=0; j<mNumParameters; ++j)
//        {
//            //Reflect
//            double worst = mParameters[mvIdx[i]][j];
//            mCandidateParticles[i][j] = mCenter[j] + (mCenter[j]-worst)*mAlpha;

//            //Add some random noise
//            double maxDiff = getMaxParDiff();
//            double r = (double)rand() / (double)RAND_MAX;
//            mCandidateParticles[i][j] = mCandidateParticles[i][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
//            mCandidateParticles[i][j] = qMin(mCandidateParticles[i][j], mParMax[j]);
//            mCandidateParticles[i][j] = qMax(mCandidateParticles[i][j], mParMin[j]);
//        }
    }
}


void OptimizationWorkerComplexRFP::evaluateCandidateParticles(bool firstTime)
{
    //Multi-threading, we cannot use the "evalall" function
    for(int i=0; i<mCandidateParticles.size() && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs[i]);
        execute("opt set evalid "+QString::number(i+mNumPoints));
        execute("call setpars");
    }
    gpModelHandler->simulateMultipleModels_blocking(mUsedModelPtrs, !firstTime); //Ok to use global model handler for this, it does not use any member stuff

    for(int i=0; i<mCandidateParticles.size() && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs[i]);
        execute("opt set evalid "+QString::number(i+mNumPoints));
        execute("call obj");
    }
    mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs.first());

    ++mIterations;
    mEvaluations += mNumThreads;
}



void OptimizationWorkerComplexRFP::examineCandidateParticles()
{
    if(mMethod == 0)
    {
        mNeedsIteration=false;

        int i=0;
        forget();

        int nWorsePoints=0;
        for(int j=0; j<mNumPoints; ++j)
        {
            if(mObjectives[j] > mObjectives[mNumPoints+i])
            {
                ++nWorsePoints;
            }
        }

        mParameters[mvIdx[i]] = mCandidateParticles[i];
        mObjectives[mvIdx[i]] = mObjectives[mNumPoints+i];
        if(nWorsePoints >= 2)   //New point is better, keep it
        {
            logPoint(mvIdx[i]);
            if(checkForConvergence()) return;   //Check convergence
        }
        else        //New point is not better, iterate
        {
            mNeedsIteration=true;
            return;
        }

        ++i;
        forget();

        if(i>=mNumThreads)
            return;

        nWorsePoints=0;
        for(int j=0; j<mNumPoints; ++j)
        {
            if(mObjectives[j] > mObjectives[mNumPoints+i])
            {
                ++nWorsePoints;
            }
        }

        mParameters[mvIdx[i]] = mCandidateParticles[i];
        mObjectives[mvIdx[i]] = mObjectives[mNumPoints+i];
        if(nWorsePoints >= 2)   //New point is better, keep it
        {
            logPoint(mvIdx[i]);
            if(checkForConvergence()) return;   //Check convergence
        }
        else        //New point is not better, iterate
        {
            mNeedsIteration=true;
            return;
        }

        ++i;
        forget();

        if(i>=mNumThreads)
            return;

        calculateBestAndWorstId();
        if(mWorstId != mvIdx[i]) return;

        nWorsePoints=0;
        for(int j=0; j<mNumPoints; ++j)
        {
            if(mObjectives[j] > mObjectives[mNumPoints+i])
            {
                ++nWorsePoints;
            }
        }

        mParameters[mvIdx[i]] = mCandidateParticles[i];
        mObjectives[mvIdx[i]] = mObjectives[mNumPoints+i];
        if(nWorsePoints >= 2)   //New point is better, keep it
        {
            logPoint(mvIdx[i]);
            if(checkForConvergence()) return;   //Check convergence
        }
        else
        {
            mNeedsIteration=true;
            return;
        }

        ++i;
        forget();

        if(i>=mNumThreads)
            return;

        calculateBestAndWorstId();
        if(mWorstId != mvIdx[i]) return;

        nWorsePoints=0;
        for(int j=0; j<mNumPoints; ++j)
        {
            if(mObjectives[j] > mObjectives[mNumPoints+i])
            {
                ++nWorsePoints;
            }
        }

        mParameters[mvIdx[i]] = mCandidateParticles[i];
        mObjectives[mvIdx[i]] = mObjectives[mNumPoints+i];
        if(nWorsePoints >= 2)   //New point is better, keep it
        {
            logPoint(mvIdx[i]);
            if(checkForConvergence()) return;   //Check convergence
        }
        else
        {
            mNeedsIteration=true;
            return;
        }
    }
    else
    {
        calculateBestAndWorstId();
        int minIdx = 0;
        for(int i=1; i<mNumThreads; ++i)
        {
            if(mObjectives[mNumPoints+i] < mObjectives[mNumPoints+minIdx])
            {
                minIdx = i;
            }
        }

        if(mObjectives[mNumPoints+minIdx] < mObjectives[mWorstId])
        {
            mParameters[mWorstId] = mCandidateParticles[minIdx];
            mObjectives[mWorstId] = mObjectives[mNumPoints+minIdx];
            logPoint(mWorstId);
            calculateBestAndWorstId();
            if(checkForConvergence()) return;   //Check convergence
        }
        else
        {
            mNeedsIteration=true;
            return;
        }
    }
}


double OptimizationWorkerComplexRFP::triangularDistribution(double min, double mid, double max)
{
    if(mid > max+min/2.0)
    {
        min = max-2.0*mid;
    }
    else
    {
        max = min+2.0*mid;
    }

    double r1 = (double)rand()/(double)RAND_MAX;
    double r2 = (double)rand()/(double)RAND_MAX;
    double r3 = (double)rand()/(double)RAND_MAX;
    double r4 = (double)rand()/(double)RAND_MAX;
    double temp1 = abs(1.0-(r1-r2)*(r3-r4));
    double temp2 = temp1*(max+min)/2.0;
    return temp2;
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

void OptimizationWorkerComplexRFP::findCenter()
{
    OptimizationWorkerComplex::findCenter();
}

void OptimizationWorkerComplexRFP::findCenter(QVector<QVector<double> > &particles)
{
    mCenter.resize(mNumParameters);
    for(int i=0; i<mCenter.size(); ++i)
    {
        mCenter[i] = 0;
    }
    for(int p=0; p<particles.size(); ++p)
    {
        for(int i=0; i<mNumParameters; ++i)
        {
            mCenter[i] = mCenter[i]+particles[p][i];
        }
    }
    for(int i=0; i<mCenter.size(); ++i)
    {
        mCenter[i] = mCenter[i]/double(particles.size());
    }
}


//! @brief Plots the optimization points (if there are at least two parameters and the option is selected)
void OptimizationWorkerComplexRFP::plotPoints()
{
    if(!mPlotPoints) { return; }

    if(mNumParameters < 2)
    {
        printError("Plotting points requires at least two parameters.");
        return;
    }

    for(int p=0; p<mNumPoints; ++p)
    {
        //QString namex = "par"+QString::number(p)+"x";
        //QString namey = "par"+QString::number(p)+"y";
        double x = mParameters[p][0];
        double y = mParameters[p][1];

        if(mPointVars_x.size() <= p)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            mPointVars_x.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));
            mPointVars_y.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));

            mPointVars_x.last()->assignFrom(x);
            mPointVars_y.last()->assignFrom(y);

            gpPlotHandler->plotDataToWindow("parplot", mPointVars_x.last(), mPointVars_y.last(), 0, QColor("blue"));
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
    for(int p=0; p<mNumThreads; ++p)
    {
        double x = mCandidateParticles[p][0];
        double y = mCandidateParticles[p][1];

        if(mCandidateVars_x.size() <= p)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            mCandidateVars_x.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));
            mCandidateVars_y.append(createFreeVectorVariable(QVector<double>(), SharedVariableDescriptionT(new VariableDescription)));

            mCandidateVars_x.last()->assignFrom(x);
            mCandidateVars_y.last()->assignFrom(y);

            gpPlotHandler->plotDataToWindow("parplot", mCandidateVars_x.last(), mCandidateVars_y.last(), 0, QColor("red"));
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLimits(QwtPlot::xBottom, mParMin[0], mParMax[0]);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLimits(QwtPlot::yLeft, mParMin[1], mParMax[1]);
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLabel(QwtPlot::xBottom, "Optimization Parameter 0");
            gpPlotHandler->getPlotWindow("parplot")->getCurrentPlotTab()->getPlotArea()->setAxisLabel(QwtPlot::yLeft, "Optimization Parameter 1");
        }
        else
        {
            //! @todo need to turn of auto refresh on plot and trigger it manually to avoid multiple redraws here
            mCandidateVars_x.at(p)->assignFrom(x);
            mCandidateVars_y.at(p)->assignFrom(y);
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
