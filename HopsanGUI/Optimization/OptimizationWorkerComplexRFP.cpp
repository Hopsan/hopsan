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
//! @file   OptimizationWorkerComplexRFP.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Complex-RFP algorithm
//!

#define OPT_ROSENBROCK 1
//#define OPT_SPHERE 1

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
#include "GUIPort.h"
#include "GUIObjects/GUISystem.h"
#include "DesktopHandler.h"

//C++ includes
#include <math.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef USEZMQ
#include "RemoteSimulationUtils.h"
#endif




OptimizationWorkerComplexRFP::OptimizationWorkerComplexRFP(OptimizationHandler *pHandler)
    : OptimizationWorkerComplex(pHandler)
{
    mMethod = 0;
    mNumDir = 1;
    mNumStep = 1;
    mNumRet = 0;
    mNumDist = 1;
    mAlphaMin = 1.0;
    mAlphaMax = 2.0;
}


//! @brief Initializes a Complex-RF optimization
void OptimizationWorkerComplexRFP::init(const ModelWidget *pModel, const QString &modelPath)
{
    OptimizationWorkerComplex::init(pModel, modelPath);

    mModelPath = modelPath;
    mLastWorstId = -1;
    mWorstCounter = 0;
    mFirstReflectionFailed = false;

    print("Using method "+QString::number(mMethod));

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


    mCandidateParticles.resize(mNumModels);
    for(int i=0; i<mNumModels; ++i)
    {
        mCandidateParticles[i].resize(mNumParameters);
    }

    mObjectives.resize(mNumPoints);
    mCandidateObjectives.resize(mNumModels);

    //Limit number of models, in case worker has opened more models than necessary
    mUsedModelPtrs.clear();
    for(int i=0; i<mNumModels; ++i)
    {
        mUsedModelPtrs.append(mModelPtrs.at(i));
    }

    mKf = 1.0-pow(mAlpha/2.0, mGamma/mNumPoints);

    LogDataHandler2 *pHandler = mUsedModelPtrs[0]->getLogDataHandler();
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
            pPlotTab->removeAllCurves();
        }
    }

    if(mMethod == 3)
    {
//        if(mNumModels == 1) // 1
//        {
//            mNumDir = 1;
//            mNumDist = 1;
//        }
//        else if(mNumModels <= 3) // 2-3
//        {
//            mNumDir = 1;
//            mNumDist = 1;
//        }
//        else if(mNumModels <= 5) // 4-5
//        {
//            mNumDir = 2;
//            mNumDist = 1;
//        }
//        else if(mNumModels <= 7) // 6-7
//        {
//            mNumDir = 2;
//            mNumDist = 2;
//        }
//        else //8-
//        {
//            mNumDir = 2;
//            mNumDist = 3;
//        }


        qDebug() << "numModels = " << mNumModels << ", numDir = " << mNumDir << ", numDist = " << mNumDist;
    }
    mActionCounter.clear();

#ifdef USEZMQ
    // Setup parallell server queues
    if (gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION))
    {
        if (mMethod == 0 )
        {
            chooseRemoteModelSimulationQueuer(Crfp0_Homo_Reschedule);
        }
        else
        {
            chooseRemoteModelSimulationQueuer(Crfp1_Homo_Reschedule);
        }
        gpRemoteModelSimulationQueuer->benchmarkModel(mUsedModelPtrs.front());
        int pm, pa; double su;
        gpRemoteModelSimulationQueuer->determineBestSpeedup(-1, 8, pm, pa, su);
        reInit(pa);

        gpRemoteModelSimulationQueuer->setupModelQueues(mUsedModelPtrs, pm);
    }
#endif

}



//! @brief Executes a Complex-RF optimization. optComplexInit() must be called before this one.
void OptimizationWorkerComplexRFP::run()
{

    //if(mMethod == 1)
    //{
        mvAlpha.clear();
        if(mNumDist == 1)
        {
            mvAlpha.append(mAlpha);
        }
        else if(mNumDist == 2)
        {
            mvAlpha.append(mAlphaMin);
            mvAlpha.append(mAlphaMax);
        }
        else
        {
            for(int i=0; i<mNumDist; ++i)
            {
                mvAlpha.append(mAlphaMin + double(i+1.0)/(double(mNumDist)+1.0)*(mAlphaMax-mAlphaMin));
            }
        }
    //}

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
    evaluatePoints();
    logAllPoints();

    //Calculate best and worst id, and initialize last worst id
    calculateBestAndWorstId();
    mLastWorstId = mWorstId;

    //Run optimization loop
    mIterations=0;
    bool changed=false;
    bool needsReschedule = false;
    mNeedsIteration = false;
    int nReflections = 0;
    for(; mIterations<mMaxEvals && !mpHandler->mpHcomHandler->isAborted();)
    {
        mDirCount=0;
        mDistCount=0;
        mIterCount=0;

//        if(i>=20 && !changed)
//        {
//            reInit(3);
//            changed=true;
//        }
#ifdef USEZMQ
        if (needsReschedule)
        {
            int pm, pa;
            double su;
            gpRemoteModelSimulationQueuer->determineBestSpeedup(-1, 8, pm, pa, su);
            reInit(pa);
            // Setup parallell server queues
            if (gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION))
            {
                gpRemoteModelSimulationQueuer->setupModelQueues(mUsedModelPtrs, pm);
            }
            needsReschedule = false;
        }
#endif

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
        updateProgressBar(mIterations);

        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        //forget();

        //Calculate best and worst point
        calculateBestAndWorstId();

        //Plot best and worst objective values
        plotObjectiveFunctionValues();

        //Find geometrical center
        findCenter();

        //Reflect worst point
        pickCandidateParticles();

        //Plot points
        plotPoints();

        //Evaluate new point
        bool evalOK = evaluateCandidateParticles(needsReschedule, mIterations==0);
        if (needsReschedule)
        {
            --mIterations;
            continue;
        }
        else if (!evalOK)
        {
            execute("echo on");
            print("Simaultion failed during candidate evaluation.");
            print("Optimization aborted.");
            finalize();
            return;
        }

        if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
        {
            execute("echo on");
            print("Optimization aborted.");
            finalize();
            return;
        }

        //Examine candidate particles
        examineCandidateParticles();

        gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

        //Calculate best and worst points
        mLastWorstId=mWorstId;
        calculateBestAndWorstId();

        //Iterate until worst point is no longer the same
        mWorstCounter=0;

        //QVector<double> newPoint = mParameters[mWorstId];
        ++nReflections;
        if(mNeedsIteration)
        {
            QVector< QVector<double> > otherPoints = mParameters;
            otherPoints.remove(mWorstId);
            findCenter(otherPoints);
        }

        bool abort=false;
        while(mNeedsIteration)
        {
        //! @note Always iterate multiple steps (iterateSingle() is used only for statistics)
            if(mMethod == 0 || mMethod == 3 || mMethod == 4)
            {
                abort = iterate();
            }
            else
            {
                abort = iterateSingle();
            }

            if(abort)
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                mpHandler->mpHcomHandler->abortHCOM();
                return;
            }

            if(mIterations>mMaxEvals) break;
        }

        if(mIterCount > 0)
        {

            nReflections = 0;   //Must be reset after action counter, since it is used there
        }

        // Check if we need to reshedule from this internal while reiteration needed loop
        if (needsReschedule)
        {
            --mIterations;
            continue;
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
        print("Optimization failed to converge after "+QString::number(mIterations)+" iterations.");
        break;
    case 1:
        print("Optimization converged in function values after "+QString::number(mIterations)+" iterations.");
        break;
    case 2:
        print("Optimization converged in parameter values after "+QString::number(mIterations)+" iterations.");
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
    QFile actionsFile(gpDesktopHandler->getDocumentsPath()+"complexrfp_method3_actions.txt");
    actionsFile.open(QFile::WriteOnly | QFile::Text | QFile::Append);
    QMapIterator<QString, int> iter(mActionCounter);
    QString output;
    while(iter.hasNext())
    {
        iter.next();
        output.append(QString::number(iter.value())+","+iter.key()+"\n");
    }
    actionsFile.write(output.toUtf8());
    actionsFile.close();



    OptimizationWorkerComplex::finalize();

#ifdef USEZMQ
    // Clear and disconnect from parallell server queues
    if (gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION))
    {
        gpRemoteModelSimulationQueuer->clear();
    }
#endif
}

void OptimizationWorkerComplexRFP::reInit(int nModels)
{
    mNumModels = nModels;

    while(mModelPtrs.size() < mNumModels)
    {
        mpHandler->addModel(gpModelHandler->loadModel(mModelPath, true, true));

        //Make sure logging is disabled/enabled for same ports as in original model
        CoreSystemAccess *pCore = mModelPtrs.first()->getTopLevelSystemContainer()->getCoreSystemAccessPtr();
        foreach(const QString &compName, mModelPtrs.first()->getTopLevelSystemContainer()->getModelObjectNames())
        {
            foreach(const Port *port, mModelPtrs.first()->getTopLevelSystemContainer()->getModelObject(compName)->getPortListPtrs())
            {
                QString portName = port->getName();
                bool enabled = pCore->isLoggingEnabled(compName, portName);
                SystemContainer *pOptSystem = mModelPtrs.last()->getTopLevelSystemContainer();
                CoreSystemAccess *pOptCore = pOptSystem->getCoreSystemAccessPtr();
                pOptCore->setLoggingEnabled(compName, portName, enabled);
            }
        }
    }

    mCandidateParticles.clear();
    mCandidateParticles.resize(mNumModels);
    for(int i=0; i<mNumModels; ++i)
    {
        mCandidateParticles[i].resize(mNumParameters);
    }

    mCandidateObjectives.clear();
    mCandidateObjectives.resize(mNumModels);

    //Limit number of models, in case worker has opened more models than necessary
    mUsedModelPtrs.clear();
    for(int i=0; i<mNumModels; ++i)
    {
        mUsedModelPtrs.append(mModelPtrs.at(i));
    }
}


void OptimizationWorkerComplexRFP::setOptVar(const QString &var, const QString &value)
{
    OptimizationWorkerComplex::setOptVar(var, value);

    if(var == "nmodels")
    {
        mNumModels = value.toInt();
    }
    else if(var == "ndist")
    {
        mNumDist = value.toInt();
    }
    else if(var == "ndir")
    {
        mNumDir = value.toInt();
    }
    else if(var == "nstep")
    {
        mNumStep = value.toInt();
    }
    else if(var == "nret")
    {
        mNumRet = value.toInt();
    }
    else if(var == "alphamin")
    {
        mAlphaMin = value.toDouble();
    }
    else if(var == "alphamax")
    {
        mAlphaMax = value.toDouble();
    }
    else if(var == "method")
    {
        mMethod = value.toInt();
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
    if(var == "alphamin")
    {
        return mAlphaMin;
    }
    else if(var == "alphamax")
    {
        return mAlphaMax;
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

    for(int i=0; i<mTopLevelCandidates.size(); ++i)
    {
        delete(mTopLevelCandidates[i]);
    }
    mTopLevelCandidates.clear();


    if(mMethod == 3)    //Standard complex-rf, but calculating extra points for statistics
    {
        if(mNumModels < 20)
        {
            printError("Too few candidate particles for statistics method!");
            mpHandler->mpHcomHandler->abortHCOM();
            return;
        }

        mCandidateParticles[0] = reflect(mParameters[mWorstId], mCenter, mAlpha);

        //Calculate different distances
        mCandidateParticles[1] = reflect(mParameters[mWorstId], mCenter, 1.0);
        mCandidateParticles[2] = reflect(mParameters[mWorstId], mCenter, 1.1);
        mCandidateParticles[3] = reflect(mParameters[mWorstId], mCenter, 1.2);
        mCandidateParticles[4] = reflect(mParameters[mWorstId], mCenter, 1.4);
        mCandidateParticles[5] = reflect(mParameters[mWorstId], mCenter, 1.5);
        mCandidateParticles[6] = reflect(mParameters[mWorstId], mCenter, 1.6);
        mCandidateParticles[7] = reflect(mParameters[mWorstId], mCenter, 1.7);
        mCandidateParticles[8] = reflect(mParameters[mWorstId], mCenter, 1.8);
        mCandidateParticles[9] = reflect(mParameters[mWorstId], mCenter, 1.9);
        mCandidateParticles[10] = reflect(mParameters[mWorstId], mCenter, 2.0);

        //Calculate different directions
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

        mCandidateParticles[11] = reflect(mParameters[mvIdx[1]], mCenter, mAlpha);
        mCandidateParticles[12] = reflect(mParameters[mvIdx[2]], mCenter, mAlpha);
        mCandidateParticles[13] = reflect(mParameters[mvIdx[3]], mCenter, mAlpha);
        mCandidateParticles[14] = reflect(mParameters[mvIdx[4]], mCenter, mAlpha);
        mCandidateParticles[15] = reflect(mParameters[mvIdx[5]], mCenter, mAlpha);
        mCandidateParticles[16] = reflect(mParameters[mvIdx[6]], mCenter, mAlpha);
        mCandidateParticles[17] = reflect(mParameters[mvIdx[7]], mCenter, mAlpha);
        mCandidateParticles[18] = reflect(mParameters[mvIdx[8]], mCenter, mAlpha);
        mCandidateParticles[19] = reflect(mParameters[mvIdx[9]], mCenter, mAlpha);

    }
    else if(mMethod == 2)   //Multi-direction
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


        for(int i=0; i<qMin(mNumPoints, mNumModels); ++i)
        {
            mWorstId = mvIdx[i];
            findCenter();

            //Reflect first point
            mCandidateParticles[i] = reflect(mParameters[mvIdx[i]], mCenter, mAlpha);

            Candidate *pCandidate = new Candidate();
            pCandidate->mpPoint = &mCandidateParticles[i];
            pCandidate->mpObjective = &mCandidateObjectives[i];
            pCandidate->idx = mvIdx[0];
            mTopLevelCandidates.append(pCandidate);
        }
    }
    else if(mMethod == 1)     //Multi-distance
    {
        calculateBestAndWorstId();
        findCenter();

        for(int i=0; i<mNumDist; ++i)
        {
            mCandidateParticles[i] = reflect(mParameters[mWorstId], mCenter, mvAlpha[i]);

            Candidate *pCandidate = new Candidate();
            pCandidate->mpPoint = &mCandidateParticles[i];
            pCandidate->mpObjective = &mCandidateObjectives[i];
            pCandidate->idx = mWorstId;
            mTopLevelCandidates.append(pCandidate);
        }
    }
    else if(mMethod==0)     //Multi-step
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

        Candidate *pCandidate = new Candidate();
        mTopLevelCandidates.append(pCandidate);

        QVector< QVector<double> > otherPoints = mParameters;
        QVector< QVector<double> > centerPoints;
        for(int i=0; i<mNumStep; ++i)
        {
            if(i!=0)
            {
                pCandidate->subCandidates.append(new Candidate());
                pCandidate = pCandidate->subCandidates.first();
            }

            if(i<mNumPoints)
            {
                otherPoints.remove(mvIdx[i]);
                findCenter(otherPoints);
                centerPoints.append(mCenter);

                mCandidateParticles[i] = reflect(mParameters[mvIdx[i]], mCenter, mAlpha);

                pCandidate->mpPoint = &mCandidateParticles[i];
                pCandidate->mpObjective = &mCandidateObjectives[i];
                pCandidate->idx = mvIdx[i];

                otherPoints.insert(mvIdx[i], mCandidateParticles[i]);
            }
            else
            {
                QVector<double> worstPoint = otherPoints.at(mvIdx[i%mNumPoints]);
                otherPoints.remove(mvIdx[i%mNumPoints]);
                findCenter(otherPoints);
                centerPoints.append(mCenter);

                mCandidateParticles[i] = reflect(worstPoint, mCenter, mAlpha);

                pCandidate->mpPoint = &mCandidateParticles[i];
                pCandidate->mpObjective = &mCandidateObjectives[i];
                pCandidate->idx = mvIdx[i%mNumPoints];

                otherPoints.insert(mvIdx[i%mNumPoints], mCandidateParticles[i]);
            }
        }

        //Use additional threads to compute a few retraction steps from first candidate
        mWorstCounter=0;
        int extraSteps = mNumModels-mNumStep-mNumRet;

        QVector<double> newPoint = (*mTopLevelCandidates[0]->mpPoint);
        for(int t=mNumStep; t<mNumModels-extraSteps; ++t)
        {
            double a1 = 1.0-exp(-double(mWorstCounter)/5.0);
            findCenter();
            for(int j=0; j<mNumParameters; ++j)
            {
                double best = mParameters[mBestId][j];
                QVector<QVector<double> > points = mParameters;
                points[mWorstId] = newPoint;
                double maxDiff = getMaxParDiff(points);
                double r = (double)rand() / (double)RAND_MAX;
                mCandidateParticles[t][j] = (mCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
                mCandidateParticles[t][j] = qMin(mCandidateParticles[t][j], mParMax[j]);
                mCandidateParticles[t][j] = qMax(mCandidateParticles[t][j], mParMin[j]);
            }

            Candidate *pRetractionCandidate = new Candidate();
            pRetractionCandidate->mpPoint = &mCandidateParticles[t];
            pRetractionCandidate->mpObjective = &mCandidateObjectives[t];
            mTopLevelCandidates[0]->retractions.append(pRetractionCandidate);

            newPoint = mCandidateParticles[t];
            ++mWorstCounter;
        }


        //Also calculate first retraction for next extraSteps candidates
        int centerCounter = 0;
        if(!mTopLevelCandidates[0]->subCandidates.isEmpty())
        {
            Candidate *pCandidate = mTopLevelCandidates[0]->subCandidates[0];
            for(int i=0; i<extraSteps; ++i)
            {
                newPoint = (*pCandidate->mpPoint);
                int t=mNumModels-extraSteps+i;
                ++centerCounter;
                mCenter = centerPoints[centerCounter];
                for(int j=0; j<mNumParameters; ++j)
                {
                    QVector<QVector<double> > points = mParameters;
                    points[mWorstId] = newPoint;
                    double maxDiff = getMaxParDiff(points);
                    double r = (double)rand() / (double)RAND_MAX;
                    mCandidateParticles[t][j] = (mCenter[j] + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
                    mCandidateParticles[t][j] = qMin(mCandidateParticles[t][j], mParMax[j]);
                    mCandidateParticles[t][j] = qMax(mCandidateParticles[t][j], mParMin[j]);
                }
                Candidate *pRetractionCandidate = new Candidate();
                pRetractionCandidate->mpPoint = &mCandidateParticles[t];
                pRetractionCandidate->mpObjective = &mCandidateObjectives[t];
                pCandidate->retractions.append(pRetractionCandidate);

                if(!pCandidate->subCandidates.isEmpty())
                {
                    pCandidate = pCandidate->subCandidates[0];
                }
                else
                {
                    break;
                }
            }
        }
    }
}


bool OptimizationWorkerComplexRFP::evaluateCandidateParticles(bool &rNeedsRescheduling, bool firstTime)
{
#ifdef OPT_ROSENBROCK
    rNeedsRescheduling=false;
    for(int i=0; i<mNumModels && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        double x1 = mCandidateParticles[i][0];
        double x2 = mCandidateParticles[i][1];
        double x3 = mCandidateParticles[i][2];
        double x4 = mCandidateParticles[i][3];
        double x5 = mCandidateParticles[i][4];
        mCandidateObjectives[i] = (1.0-x1)*(1.0-x1) + 100.0*(x2-x1*x1)*(x2-x1*x1) +
                       (1.0-x2)*(1.0-x2) + 100.0*(x3-x2*x2)*(x3-x2*x2) +
                       (1.0-x3)*(1.0-x3) + 100.0*(x4-x3*x3)*(x4-x3*x3) +
                       (1.0-x4)*(1.0-x4) + 100.0*(x5-x4*x4)*(x5-x4*x4);
    }
    ++mIterations;
    mEvaluations += mNumModels;
    return true;
#endif
#ifdef OPT_SPHERE
    rNeedsRescheduling=false;
    for(int i=0; i<mNumModels && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        double x1 = mCandidateParticles[i][0];
        double x2 = mCandidateParticles[i][1];
        double x3 = mCandidateParticles[i][2];
        double x4 = mCandidateParticles[i][3];
        double x5 = mCandidateParticles[i][4];
        mCandidateObjectives[i] = x1*x1+x2*x2+x3*x3+x4*x4+x5*x5;
    }
    ++mIterations;
    mEvaluations += mNumModels;
    return true;
#endif

    rNeedsRescheduling = false;

    //Multi-threading, we cannot use the "evalall" function
    for(int i=0; i<mNumModels && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs[i]);
        execute("opt set evalid "+QString::number(mNumPoints+i));
        execute("call setpars");
    }

    bool simOK=false;
#ifdef USEZMQ
    if (gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION))
    {
        if (gpRemoteModelSimulationQueuer && gpRemoteModelSimulationQueuer->hasServers())
        {
            simOK = gpRemoteModelSimulationQueuer->simulateModels(rNeedsRescheduling);
        }
    }
    else
    {
        simOK = gpModelHandler->simulateMultipleModels_blocking(mUsedModelPtrs, !firstTime);
    }
#else
    simOK = gpModelHandler->simulateMultipleModels_blocking(mUsedModelPtrs, !firstTime);
#endif
    if (!simOK)
    {
        return false;
    }

    for(int i=0; i<mNumModels && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
//        double x1 = mCandidateParticles[i][0];
//        double x2 = mCandidateParticles[i][1];
//        double x3 = mCandidateParticles[i][2];
//        double x4 = mCandidateParticles[i][3];
//        double x5 = mCandidateParticles[i][4];
//        mCandidateObjectives[i] = x1*x1+x2*x2+x3*x3+x4*x4+x5*x5;
//        mCandidateObjectives[i] = (1.0-x1)*(1.0-x1) + 100.0*(x2-x1*x1)*(x2-x1*x1) +
//                       (1.0-x2)*(1.0-x2) + 100.0*(x3-x2*x2)*(x3-x2*x2) +
//                       (1.0-x3)*(1.0-x3) + 100.0*(x4-x3*x3)*(x4-x3*x3) +
//                       (1.0-x4)*(1.0-x4) + 100.0*(x5-x4*x4)*(x5-x4*x4);
        mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs[i]);
        execute("opt set evalid "+QString::number(mNumPoints+i));
        execute("call obj");
    }
    mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs.first());

    ++mIterations;
    mEvaluations += mNumModels;

    return true;
}

bool OptimizationWorkerComplexRFP::evaluatePoints(bool firstTime)
{
    #ifdef OPT_ROSENBROCK
        for(int i=0; i<mNumPoints && !mpHandler->mpHcomHandler->isAborted(); ++i)
        {
            double x1 = mParameters[i][0];
            double x2 = mParameters[i][1];
            double x3 = mParameters[i][2];
            double x4 = mParameters[i][3];
            double x5 = mParameters[i][4];
            mObjectives[i] = (1.0-x1)*(1.0-x1) + 100.0*(x2-x1*x1)*(x2-x1*x1) +
                           (1.0-x2)*(1.0-x2) + 100.0*(x3-x2*x2)*(x3-x2*x2) +
                           (1.0-x3)*(1.0-x3) + 100.0*(x4-x3*x3)*(x4-x3*x3) +
                           (1.0-x4)*(1.0-x4) + 100.0*(x5-x4*x4)*(x5-x4*x4);
        }
        ++mIterations;
        mEvaluations += mNumPoints;
        return true;
#endif
#ifdef OPT_SPHERE
        for(int i=0; i<mNumPoints && !mpHandler->mpHcomHandler->isAborted(); ++i)
        {
            double x1 = mParameters[i][0];
            double x2 = mParameters[i][1];
            double x3 = mParameters[i][2];
            double x4 = mParameters[i][3];
            double x5 = mParameters[i][4];
            mObjectives[i] = x1*x1+x2*x2+x3*x3+x4*x4+x5*x5;
        }
        ++mIterations;
        mEvaluations += mNumPoints;
        return true;
#endif

    // In case we have wefere models then points, we need to process the models sequentially
    // (all models in paralllel in ieach sequential step needed)
    int numEvaluatedPoints=0;
    int point_id=0;
    while ( (numEvaluatedPoints < mNumPoints) && !mpHandler->mpHcomHandler->isAborted() )
    {
        const int nMax = qMin(mNumPoints-numEvaluatedPoints, mNumModels);

        // Set parameters in models
        int candidate_id=0;
        for(int m=0; m<nMax && !mpHandler->mpHcomHandler->isAborted(); ++m)
        {
            mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs[m]);
            mCandidateParticles[candidate_id] = mParameters[point_id+m];
            execute("opt set evalid "+QString::number(mNumPoints+candidate_id));
            execute("call setpars");
            ++candidate_id;
        }

        // Simulate in parallele if possible
        bool simOK=false;
    #ifdef USEZMQ
        if (gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION))
        {
            if (gpRemoteModelSimulationQueuer && gpRemoteModelSimulationQueuer->hasServers())
            {
                bool dummy;
                simOK = gpRemoteModelSimulationQueuer->simulateModels(dummy);
            }
        }
        else
        {
            simOK = gpModelHandler->simulateMultipleModels_blocking(mUsedModelPtrs, !firstTime);
        }
    #else
        simOK = gpModelHandler->simulateMultipleModels_blocking(mUsedModelPtrs, !firstTime);
    #endif
        if (!simOK)
        {
            return false;
        }

        // Now calculate objective function value
        candidate_id=0;
        for(int m=0; m<nMax && !mpHandler->mpHcomHandler->isAborted(); ++m)
        {
            mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs[m]);
            execute("opt set evalid "+QString::number(mNumPoints+candidate_id));
            execute("call obj");
            mObjectives[point_id] = mCandidateObjectives[candidate_id];
            ++point_id;
            ++candidate_id;
        }


        // Reset model pointer
        mpHandler->mpHcomHandler->setModelPtr(mUsedModelPtrs.first());

        // Increment number of evaluated points
        numEvaluatedPoints += nMax;
    }

    ++mIterations;
    mEvaluations += numEvaluatedPoints;
    return true;
}



void OptimizationWorkerComplexRFP::examineCandidateParticles()
{
    if(mMethod == 3)
    {
        qDebug() << "Results: " << mCandidateObjectives;

        forget();

        int nWorsePoints=0;
        for(int ptId=0; ptId<mNumPoints; ++ptId)
        {
            if(mObjectives[ptId] > mCandidateObjectives[0])
            {
                ++nWorsePoints;
            }
        }

        mParameters[mWorstId] = mCandidateParticles[0];
        mObjectives[mWorstId] = mCandidateObjectives[0];

        if(nWorsePoints >= 2)
        {
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
    else
    {
        mNeedsIteration=false;

        Candidate *pCandidate = mTopLevelCandidates[0];
        for(int i=1; i<mTopLevelCandidates.size(); ++i)
        {
            if((*mTopLevelCandidates[i]->mpObjective) < (*pCandidate->mpObjective))
            {
                pCandidate = mTopLevelCandidates[i];
            }
        }


        mFirstReflectionFailed=true;
        while(true)
        {
            forget();

            int nWorsePoints=0;
            for(int ptId=0; ptId<mNumPoints; ++ptId)
            {
                if(mObjectives[ptId] > (*pCandidate->mpObjective))
                {
                    ++nWorsePoints;
                }
            }

            mParameters[pCandidate->idx] = (*pCandidate->mpPoint);
            mObjectives[pCandidate->idx] = (*pCandidate->mpObjective);
            if(nWorsePoints >= 2)   //New point is better, keep it
            {
                logPoint(pCandidate->idx);
                if(checkForConvergence()) return;   //Check convergence
            }
            else        //New point is not better, iterate
            {
                mpFailedCandidate = pCandidate;
                mNeedsIteration=true;
                return;
            }

            if(!pCandidate->subCandidates.isEmpty())
            {
                pCandidate = pCandidate->subCandidates[0];
                for(int i=1; i<pCandidate->subCandidates.size(); ++i)
                {
                    if((*pCandidate->subCandidates[i]->mpObjective) < (*pCandidate->mpObjective))
                    {
                        pCandidate = pCandidate->subCandidates[i];
                    }
                }
            }
            else
            {
                break;
            }
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
    for(int i=0; i<mNumParameters; ++i)
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
    for(int i=0; i<mNumParameters; ++i)
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
    for(int p=0; p<mNumModels; ++p)
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

void OptimizationWorkerComplexRFP::setOptimizationObjectiveValue(int idx, double value)
{
    if(idx>=0 && idx < mNumPoints)
    {
        mObjectives[idx] = value;
    }
    else if(idx < mCandidateObjectives.size()+mNumPoints)
    {
        mCandidateObjectives[idx-mNumPoints] = value;
    }
}


bool OptimizationWorkerComplexRFP::iterate()
{
    QVector<double> newPoint = mParameters[mWorstId];


    qApp->processEvents();
    if(mpHandler->mpHcomHandler->isAborted())
    {
        execute("echo on");
        print("Optimization aborted.");
        finalize();
        mpHandler->mpHcomHandler->abortHCOM();
        return true;
    }

    //Check the already evaluated iteration points (if any)
    if(mWorstCounter == 0)
    {
        for(int i=0; i<mpFailedCandidate->retractions.size(); ++i)
        {
            Candidate *pCandidate = mpFailedCandidate->retractions.at(i);

            newPoint = (*pCandidate->mpPoint);
            int nWorsePoints=0;
            for(int j=0; j<mNumPoints; ++j)
            {
                if(mObjectives[j] > (*pCandidate->mpObjective))
                {
                    ++nWorsePoints;
                }
            }

            if(nWorsePoints >= 2)
            {
                mParameters[mWorstId] = (*pCandidate->mpPoint);
                mObjectives[mWorstId] = (*pCandidate->mpObjective);
                logWorstPoint();
                mIterCount = 1;
                mNeedsIteration = false;
                return false;
            }
            ++mWorstCounter;
        }
    }

    if(!mNeedsIteration)
    {
        gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);
        plotParameters();
        plotEntropy();
        return false;
    }

    //Move first reflected point
    for(int t=0; t<mNumModels && mNeedsIteration; ++t)
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
        mParameters[mWorstId] = newPoint;
        ++mWorstCounter;
    }

    plotPoints();

    //Evaluate new point
    bool needsReschedule;
    bool evalOK2 = evaluateCandidateParticles(needsReschedule, false);
    if (needsReschedule)
    {
        return false;
    }
    else if (!evalOK2)
    {
        execute("echo on");
        print("Simaultion failed during candidate evaluation.");
        print("Optimization aborted.");
        finalize();
        return true;
    }


    //Replace worst point with first candidate point that is better, if any
    for(int o=0; o<mNumModels; ++o)
    {
        mParameters[mWorstId] = mCandidateParticles[o];
        mObjectives[mWorstId] = mCandidateObjectives[o];

        int prevWorst = mWorstId;
        calculateBestAndWorstId();
        if(prevWorst != mWorstId)
        {
            logWorstPoint();
            mNeedsIteration = false;

            mIterCount = mWorstCounter-mNumModels+o+1;

            return false;
        }
    }


    if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
    {
        execute("echo on");
        print("Optimization aborted.");
        finalize();
        return true;
    }

    //Calculate best and worst points
    mLastWorstId=mWorstId;
    calculateBestAndWorstId();

    execute("echo off -nonerrors");


//            if(mNeedsIteration)
//            {
//                //Replace worst point with last candidate if no success (for updating parameter outputs)
//                mParameters[mWorstId] = mCandidateParticles.last();
//                mObjectives[mWorstId] = mCandidateObjectives.last();
//            }

    gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

    plotParameters();
    plotEntropy();

    return false;
}



bool OptimizationWorkerComplexRFP::iterateSingle()
{
    QVector<double> newPoint = mParameters[mWorstId];

    plotPoints();

    qApp->processEvents();
    if(mpHandler->mpHcomHandler->isAborted())
    {
        execute("echo on");
        print("Optimization aborted.");
        finalize();
        mpHandler->mpHcomHandler->abortHCOM();
        return true;
    }


    //Move first reflected point
    double a1 = 1.0-exp(-double(mWorstCounter)/5.0);
    for(int j=0; j<mNumParameters; ++j)
    {
        double best = mParameters[mBestId][j];
        double maxDiff = getMaxParDiff()*10/(9.0+mWorstCounter);
        double r = (double)rand() / (double)RAND_MAX;
        mParameters[mWorstId][j] = (mCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
        mParameters[mWorstId][j] = qMin(mParameters[mWorstId][j], mParMax[j]);
        mParameters[mWorstId][j] = qMax(mParameters[mWorstId][j], mParMin[j]);
    }

    ++mWorstCounter;

    //Evaluate new point
    bool multicore = gpConfig->getUseMulticore();
    if(multicore)
        execute("set multicore off");   //Temporary hack, remove later?
#ifdef OPT_SPHERE
    double x1 = mParameters[mWorstId][0];
    double x2 = mParameters[mWorstId][1];
    double x3 = mParameters[mWorstId][2];
    double x4 = mParameters[mWorstId][3];
    double x5 = mParameters[mWorstId][4];
    mObjectives[mWorstId] = x1*x1+x2*x2+x3*x3+x4*x4+x5*x5;
#endif
#ifdef OPT_ROSENBROCK
    double x1 = mParameters[mWorstId][0];
    double x2 = mParameters[mWorstId][1];
    double x3 = mParameters[mWorstId][2];
    double x4 = mParameters[mWorstId][3];
    double x5 = mParameters[mWorstId][4];
    mObjectives[mWorstId] = (1.0-x1)*(1.0-x1) + 100.0*(x2-x1*x1)*(x2-x1*x1) +
            (1.0-x2)*(1.0-x2) + 100.0*(x3-x2*x2)*(x3-x2*x2) +
            (1.0-x3)*(1.0-x3) + 100.0*(x4-x3*x3)*(x4-x3*x3) +
            (1.0-x4)*(1.0-x4) + 100.0*(x5-x4*x4)*(x5-x4*x4);
#endif
#ifndef OPT_SPHERE
#ifndef OPT_ROSENBROCK
    execute("call evalworst");
#endif
#endif
    ++mEvaluations;
    ++mIterations;
    if(multicore)
        execute("set multicore on");

    //Replace worst point with first candidate point, if better
    int prevWorst = mWorstId;
    calculateBestAndWorstId();
    if(prevWorst != mWorstId)
    {
        logWorstPoint();
        mNeedsIteration = false;

        mIterCount = mWorstCounter+1;

        return false;
    }


    if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
    {
        execute("echo on");
        print("Optimization aborted.");
        finalize();
        return true;
    }

    execute("echo off -nonerrors");

    gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

    plotParameters();
    plotEntropy();

    return false;
}

//! @brief Reflects specified point through specified centroid and returns the reflected point
//! @param point Point to reflect
//! @param center Point to reflect through
//! @param alpha Reflection factor
QVector<double> OptimizationWorkerComplexRFP::reflect(QVector<double> point, QVector<double> center, double alpha)
{
    QVector<double> newPoint;
    newPoint.resize(mNumParameters);

    for(int j=0; j<mNumParameters; ++j)
    {
        //Reflect
        newPoint[j] = center[j] + (center[j]-point[j])*alpha;

        //Add some random noise
        double maxDiff = getMaxParDiff();
        double r = (double)rand() / (double)RAND_MAX;
        newPoint[j] = newPoint[j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
        newPoint[j] = qMin(newPoint[j], mParMax[j]);
        newPoint[j] = qMax(newPoint[j], mParMin[j]);
    }

    return newPoint;
}


Candidate::Candidate()
{
    this->idx = 0;
    this->mpObjective = 0;
    this->mpPoint = 0;
}

Candidate::~Candidate()
{
    for(int i=0; i<subCandidates.size(); ++i)
    {
        delete(subCandidates[i]);
    }
    subCandidates.clear();

    for(int i=0; i<retractions.size(); ++i)
    {
        delete(retractions[i]);
    }
    retractions.clear();
}
