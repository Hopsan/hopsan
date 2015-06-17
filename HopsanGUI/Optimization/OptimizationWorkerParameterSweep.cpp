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
//! @file   OptimizationWorkerParameterSweep.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-19
//! @version $Id$
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
#include "Widgets/ModelWidget.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"

//Qt includes
#include <QDebug>
#include <QDateTime>

//C++ includes
#include <math.h>
#ifndef _WIN32
#include <unistd.h>
#endif

class OptimizationHandler;

OptimizationWorkerParameterSweep::OptimizationWorkerParameterSweep(OptimizationHandler *pHandler)
    : OptimizationWorker(pHandler)
{

}

void OptimizationWorkerParameterSweep::init(const ModelWidget *pModel, const QString &modelPath)
{
    mNumModels = gpConfig->getIntegerSetting(CFG_NUMBEROFTHREADS);
    if(mNumModels == 0)
    {
#ifdef _WIN32
        std::string temp = getenv("NUMBER_OF_PROCESSORS");
        mNumModels = atoi(temp.c_str());
#else
        mNumModels = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    }

    OptimizationWorker::init(pModel, modelPath);

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

    mNumPoints = mNumModels;
    mParameters.resize(mNumPoints);
    mObjectives.resize(mNumPoints);

    mMaxEvals = pow(mLength, mNumParameters);
}

void OptimizationWorkerParameterSweep::run()
{
    execute("echo off -nonerrors");

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

        mParameters[i%mNumModels] = mAllPoints[i];

        if(i%mNumModels == mNumModels-1)
        {
            evaluateAllPoints();

            for(int o=0; o<mNumModels; ++o)
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


    printLogFile();
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
    logAllPoints();

    ++mIterations;
    mEvaluations += mNumPoints;
}

void OptimizationWorkerParameterSweep::printLogFile()
{
    QFile outputFile(gpDesktopHandler->getDocumentsPath()+"parameter_sweep_"+QDateTime::currentDateTime().toString("yyyyMMdd")+".txt");
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
