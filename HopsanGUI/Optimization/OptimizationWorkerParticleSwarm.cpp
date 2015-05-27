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
//! @file   OptimizationWorkerParticleSwarm.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the particle swarm algorithm
//!

//Hopsan includes
#include "OptimizationWorkerParticleSwarm.h"
#include "OptimizationWorker.h"
#include "OptimizationHandler.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/HcomWidget.h"
#include "HcomHandler.h"
#include "Configuration.h"
#include "GUIObjects/GUIContainerObject.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "global.h"
#include "PlotWindow.h"
#include "Dialogs/OptimizationDialog.h"
#include "ModelHandler.h"

//C++ includes
#include <math.h>

#ifdef USEZMQ
#include "RemoteSimulationUtils.h"
#endif

//! @brief Initializes a particle swarm optimization
OptimizationWorkerParticleSwarm::OptimizationWorkerParticleSwarm(OptimizationHandler *pHandler)
    : OptimizationWorker(pHandler)
{
    mPrintLogOutput = true;  //! @todo Should be changeable by user
}

void OptimizationWorkerParticleSwarm::init(const ModelWidget *pModel, const QString &modelPath)
{
    mNumModels = mNumPoints;

    OptimizationWorker::init(pModel, modelPath);

    for(int p=0; p<mNumPoints; ++p)
    {
        mParameters[p].resize(mNumParameters);
        mVelocities[p].resize(mNumParameters);
        for(int i=0; i<mNumParameters; ++i)
        {
            //Initialize points
            double r = double(rand()) / double(RAND_MAX);
            mParameters[p][i] = mParMin[i] + r*(mParMax[i]-mParMin[i]);
            if(mpHandler->mParameterType == OptimizationHandler::Integer)
            {
                mParameters[p][i] = round(mParameters[p][i]);
            }

            //Initialize velocities
            double minVel = -fabs(mParMax[i]-mParMin[i]);
            double maxVel = fabs(mParMax[i]-mParMin[i]);
            r = double(rand()) / double(RAND_MAX);
            mVelocities[p][i] = minVel + r*(maxVel-minVel);
        }
    }
    mObjectives.resize(mNumPoints);

    LogDataHandler2 *pHandler = mModelPtrs[0]->getLogDataHandler();
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

#ifdef USEZMQ
    // Setup parallell server queues
    if (gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION))
    {
        chooseRemoteModelSimulationQueuer(Pso_Homo_Reschedule);
        gpRemoteModelSimulationQueuer->setup(mModelPtrs);
    }
#endif

}


//! @brief Executes a particle swarm algorithm. optParticleInit() must be called before this one.
void OptimizationWorkerParticleSwarm::run()
{
    plotPoints();

    mpHandler->mpHcomHandler->mpConsole->mpTerminal->setAbortButtonEnabled(true);

    mConvergenceReason=0;

   if(!mpHandler->mpHcomHandler->hasFunction("evalall"))
    {
        printError("Function \"evalall\" not defined.","",false);
        return;
    }

    print("Running optimization...", "", true);

    //Disable terminal output during optimization
    execute("echo off -nonerrors");

    //Evaluate initial objective values
    evaluateAllParticles();
    if(mpHandler->mpHcomHandler->getVar("ans") == -1 || mpHandler->mpHcomHandler->isAborted())    //This check is needed if abort key is pressed while evaluating
    {
        execute("echo on");
        print("Optimization aborted.");
        finalize();
        return;
    }

    logAllPoints();

    //Initialize best known point for each point
    for(int i=0; i<mNumPoints; ++i)
    {
        mBestKnowns[i] = mParameters[i];
        mBestObjectives[i] = mObjectives[i];
    }

    //Calculate best known global position
    calculateBestAndWorstId();
    mPsBestObj = mObjectives[mBestId];
    mBestPoint = mParameters[mBestId];

    bool asynchronous=false;
    if(asynchronous)
    {
        moveParticles();
        evaluateAllParticles();
        while(!checkForConvergence())
        {
            //Process events, to make sure GUI is updated
            qApp->processEvents();

            //Abort if abort key was pressed
            if(mpHandler->mpHcomHandler->isAborted())
            {
                print("Optimization aborted.");
                //gpModelHandler->setCurrentModel(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
                finalize();
                return;
            }

            //Print log output
            printLogOutput();

            //Update progress bar in dialog
            updateProgressBar(0);

            //Move particles
            for(int p=0; p<mNumPoints; ++p)
            {
                if(mModelPtrs.at(p)->getLastSimulationTime() > 0)
                {
                    mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[p]);
                    execute("opt set evalid "+QString::number(p));
                    execute("call obj");
                }



                //Evaluate objective values
                if(mpHandler->mpHcomHandler->getVar("ans") == -1 || mpHandler->mpHcomHandler->isAborted())    //This check is needed if abort key is pressed while evaluating
                {
                    execute("echo on");
                    print("Optimization aborted.");
                    //mpHcomHandler->setModelPtr(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
                    finalize();
                    return;
                }

                //Calculate best known positions
                if(mObjectives[p] < mBestObjectives[p])
                {
                    mBestKnowns[p] = mParameters[p];
                    mBestObjectives[p] = mObjectives[p];
                }

                //Calculate best known global position
                calculateBestAndWorstId();
                if(mObjectives[mBestId] < mPsBestObj)
                {
                    mPsBestObj = mObjectives[mBestId];
                    mBestPoint = mParameters[mBestId];
                }
                gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

                plotPoints();
                plotObjectiveFunctionValues();
                plotEntropy();

                moveParticle(p);
                evaluateParticleNonBlocking(p);
            }
        }
    }
    else
    {
        int i=0;
        for(; i<mMaxEvals && !mpHandler->mpHcomHandler->isAborted(); ++i)
        {
            //Process events, to make sure GUI is updated
            qApp->processEvents();

            //Abort if abort key was pressed
            if(mpHandler->mpHcomHandler->isAborted())
            {
                print("Optimization aborted.");
                //gpModelHandler->setCurrentModel(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
                finalize();
                return;
            }

            //Print log output
            printLogOutput();

            //Update progress bar in dialog
            updateProgressBar(i);

            //Move particles
            moveParticles();

            //Evaluate objective values
            evaluateAllParticles();
            if(mpHandler->mpHcomHandler->getVar("ans") == -1 || mpHandler->mpHcomHandler->isAborted())    //This check is needed if abort key is pressed while evaluating
            {
                execute("echo on");
                print("Optimization aborted.");
                //mpHcomHandler->setModelPtr(qobject_cast<ModelWidget*>(gpCentralTabWidget->currentWidget()));
                finalize();
                return;
            }

            //Calculate best known positions
            for(int p=0; p<mNumPoints; ++p)
            {
                if(mObjectives[p] < mBestObjectives[p])
                {
                    mBestKnowns[p] = mParameters[p];
                    mBestObjectives[p] = mObjectives[p];
                }
            }

            //Calculate best known global position
            calculateBestAndWorstId();
            if(mObjectives[mBestId] < mPsBestObj)
            {
                mPsBestObj = mObjectives[mBestId];
                mBestPoint = mParameters[mBestId];
            }
            gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

            plotPoints();
            plotObjectiveFunctionValues();
            plotEntropy();

            //Check convergence
            if(checkForConvergence()) break;      //Use complex method, it's the same principle
        }
    }

    execute("echo on");

    switch(mConvergenceReason)
    {
    case 0:
        print("Optimization failed to converge.");
        break;
    case 1:
        print("Optimization converged in function values.");
        break;
    case 2:
        print("Optimization converged in parameter values.");
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

    //Clean up
    finalize();
}

void OptimizationWorkerParticleSwarm::finalize()
{
    OptimizationWorker::finalize();

#ifdef USEZMQ
    // Clear and disconnect from parallell server queues
    if (gpConfig->getBoolSetting(CFG_USEREMOTEOPTIMIZATION))
    {
        gpRemoteModelSimulationQueuer->clear();
    }
#endif
}



//! @brief Moves the particles (for particle swarm optimization)
void OptimizationWorkerParticleSwarm::moveParticles()
{
    for (int p=0; p<mNumPoints; ++p)
    {
        moveParticle(p);
    }
}


//! @brief Moves specified particles (for particle swarm optimization)
void OptimizationWorkerParticleSwarm::moveParticle(int p)
{
    double r1 = double(rand())/double(RAND_MAX);
    double r2 = double(rand())/double(RAND_MAX);
    for(int j=0; j<mNumParameters; ++j)
    {
        mVelocities[p][j] = mPsOmega*mVelocities[p][j] + mPsC1*r1*(mBestKnowns[p][j]-mParameters[p][j]) + mPsC2*r2*(mBestPoint[j]-mParameters[p][j]);
        mParameters[p][j] = mParameters[p][j]+mVelocities[p][j];
        if(mParameters[p][j] <= mParMin[j])
        {
            mParameters[p][j] = mParMin[j];
            mVelocities[p][j] = 0.0;
        }
        if(mParameters[p][j] >= mParMax[j])
        {
            mParameters[p][j] = mParMax[j];
            mVelocities[p][j] = 0.0;
        }
    }
}


void OptimizationWorkerParticleSwarm::evaluateParticleNonBlocking(int p)
{
    mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[p]);
    execute("opt set evalid "+QString::number(p));
    execute("call setpars");

    mModelPtrs.at(p)->setLastSimulationTime(0);
    mModelPtrs.at(p)->simulate_nonblocking();
}


void OptimizationWorkerParticleSwarm::evaluateAllParticles()
{
    if(mpHandler->mpConfig->getUseMulticore())
    {
        //Multi-threading, we cannot use the "evalall" function
        for(int j=0; j<mNumPoints && !mpHandler->mpHcomHandler->isAborted(); ++j)
        {
            mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[j]);
            execute("opt set evalid "+QString::number(j));
            execute("call setpars");
        }
        gpModelHandler->simulateMultipleModels_blocking(mModelPtrs); //Ok to use global model handler for this, it does not use any member stuff
        for(int j=0; j<mNumPoints && !mpHandler->mpHcomHandler->isAborted(); ++j)
        {
            mpHandler->mpHcomHandler->setModelPtr(mModelPtrs[j]);
            execute("opt set evalid "+QString::number(j));
            execute("call obj");
        }
        mpHandler->mpHcomHandler->setModelPtr(mModelPtrs.first());
    }
    else
    {
        execute("call evalall");
    }

    ++mIterations;
    mEvaluations += mNumPoints;
}



//! @brief Prints logging output about particles (for use with particle swarm algorithm)
//! @todo Extent so it can also be used with complex algorithm
void OptimizationWorkerParticleSwarm::printLogOutput()
{
    if(mPrintLogOutput)
    {
        if(mLogOutput.isEmpty())
        {
            //Prepare logging output list
            for(int p=0; p<mNumPoints; ++p)
            {
                mLogOutput.append("Particle    "+QString::number(p)+":\n");
                mLogOutput[p].append("Position: \t\t\tVelocity: \t\t\tLocal best: \t\t\tGlobal best:\n");

            }
        }

        for(int p=0; p<mNumPoints; ++p)
        {
            for(int i=0; i<mParameters[p].size(); ++i)
            {
                mLogOutput[p].append(QString::number(mParameters[p][i])+",");
            }
            mLogOutput[p].chop(1);
            mLogOutput[p].append("\t\t");
            for(int i=0; i<mVelocities[p].size(); ++i)
            {
                mLogOutput[p].append(QString::number(mVelocities[p][i])+",");
            }
            mLogOutput[p].chop(1);
            mLogOutput[p].append("\t\t");
            for(int i=0; i<mBestKnowns[p].size(); ++i)
            {
                mLogOutput[p].append(QString::number(mBestKnowns[p][i])+",");
            }
            mLogOutput[p].chop(1);
            mLogOutput[p].append("\t\t");
            for(int i=0; i<mBestPoint.size(); ++i)
            {
                mLogOutput[p].append(QString::number(mBestPoint[i])+",");
            }
            mLogOutput[p].chop(1);
            mLogOutput[p].append("\n");
        }
    }
}


void OptimizationWorkerParticleSwarm::setOptVar(const QString &var, const QString &value)
{
    OptimizationWorker::setOptVar(var, value);

    if(var == "nmodels")
    {
        mNumModels = value.toInt();
    }
    else if(var == "npoints")
    {
        int n = value.toInt();
        mVelocities.resize(n);
        mBestKnowns.resize(n);
        mBestObjectives.resize(n);
    }
    else if(var == "nparams")
    {
        int n = value.toInt();
        mBestPoint.resize(n);
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
}

double OptimizationWorkerParticleSwarm::getOptVar(const QString &var, bool &ok)
{
    double retval = OptimizationWorker::getOptVar(var, ok);
    if(ok)
    {
        return retval;
    }

    ok = true;
    if(var == "omega")
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
    else
    {
        ok = false;
        return 0;
    }
}
