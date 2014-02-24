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
//! @file   OptimizationWorkerParameterSweep.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-19
//! @version $Id: OptimizationHandler.cpp 6525 2014-01-30 15:58:59Z petno25 $
//!
//! @brief Contains an optimization worker object for parameter sweeps
//!

//Hopsan includes
#include "Configuration.h"
#include "DesktopHandler.h"
#include "global.h"
#include "HcomHandler.h"
#include "ModelHandler.h"
#include "OptimizationHandler.h"
#include "OptimizationWorker.h"
#include "OptimizationWorkerParameterSweep.h"

//Qt includes
#include <QDebug>
#include <QDateTime>

//C++ includes
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#endif

class OptimizationHandler;

OptimizationWorkerParameterSweep::OptimizationWorkerParameterSweep(OptimizationHandler *pHandler)
    : OptimizationWorker(pHandler)
{

}

void OptimizationWorkerParameterSweep::init()
{
    OptimizationWorker::init();

    mNumThreads = gpConfig->getNumberOfThreads();
    if(mNumThreads == 0)
    {
#ifdef WIN32
        std::string temp = getenv("NUMBER_OF_PROCESSORS");
        mNumThreads = atoi(temp.c_str());
#else
        mNumThreads = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    }

    mAllPoints.resize(pow(mLength, mNumParameters));
    mAllObjectives.reserve(pow(mLength, mNumParameters));
    for(int i=0; i<pow(mLength, mNumParameters); ++i)
    {
        for(int p=0; p<mNumParameters; ++p)
        {
            double x = floor(fmod(double(i)/double(pow(mLength,p)),mLength)) / double(mLength-1);
            mAllPoints[i].append(mParMin[p]+(mParMax[p]-mParMin[p])*x);
        }
    }

    mNumPoints = mNumThreads;
    mParameters.resize(mNumPoints);
    mObjectives.resize(mNumPoints);

    mMaxEvals = pow(mLength, mNumParameters);
}

void OptimizationWorkerParameterSweep::run()
{
    execute("echo off");

    //Verify that everything is ok
    if(!mpHandler->mpHcomHandler->hasFunction("evalall"))
    {
        printError("Function \"evalall\" not defined.");
        return;
    }

    if(mAllPoints.isEmpty())
    {
        printError("Length of parameter vectors undefined. Aborting.");
        return;
    }

    int bestIdx=0;

    for(int i=0; i<mAllPoints.size(); ++i)
    {
        updateProgressBar(i);

        mParameters[i%mNumThreads] = mAllPoints[i];

        if(i%mNumThreads == 3)
        {
            evaluateAllPoints();

            for(int o=0; o<mNumThreads; ++o)
            {
                mAllObjectives.append(mObjectives[o]);
                if(mObjectives[o] < mAllObjectives[bestIdx])
                {
                    bestIdx = mAllObjectives.size()-1;
                }
            }
        }
    }

    execute("echo on");

    print("Best objective:");
    print(QString::number(mAllObjectives[bestIdx]));
    print("At point:");
    for(int p=0; p<mNumParameters; ++p)
    {
        print(QString::number(mAllPoints[bestIdx][p]));
    }


    printOutput();
}

void OptimizationWorkerParameterSweep::finalize()
{
    mAllPoints.clear();
    mAllObjectives.clear();

    OptimizationWorker::finalize();
}

void OptimizationWorkerParameterSweep::setOptVar(const QString &var, const QString &value)
{
    OptimizationWorker::setOptVar(var, value);

    if(var == "length")
    {
        mLength = value.toInt();
    }
}

double OptimizationWorkerParameterSweep::getOptVar(const QString &var, bool &ok)
{
    double retval = OptimizationWorker::getOptVar(var, ok);
    if(ok)
    {
        return retval;
    }

    return 0;
}

void OptimizationWorkerParameterSweep::evaluateAllPoints()
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

void OptimizationWorkerParameterSweep::printOutput()
{
    QFile outputFile(gpDesktopHandler->getDocumentsPath()+"/parameter_sweep_"+QDateTime::currentDateTime().toString("yyyyMMdd")+".txt");
    outputFile.open(QFile::WriteOnly | QFile::Text);
    QString output;
    for(int i=0; i<mAllObjectives.size(); ++i)
    {
        for(int p=0; p<mNumParameters; ++p)
        {
            output.append(QString::number(mAllPoints[i][p])+",");
        }
        output.append(QString::number(mAllObjectives[i])+"\n");
    }
    outputFile.write(output.toUtf8());
    outputFile.close();
}
