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
//! @file   OptimizationWorkerComplex.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains a base class for optimization worker objects using constrained simplex (complex) algorithms
//!

#include "OptimizationWorkerComplex.h"
#include "OptimizationWorker.h"
#include "OptimizationHandler.h"

//! @brief Calculates center point for complex algorithm
OptimizationWorkerComplex::OptimizationWorkerComplex(OptimizationHandler *pHandler)
    : OptimizationWorker(pHandler)
{
    mDontChangeStartValues=false;
}

void OptimizationWorkerComplex::init(const ModelWidget *pModel, const QString &modelPath)
{
    OptimizationWorker::init(pModel, modelPath);
}

void OptimizationWorkerComplex::run()
{
    OptimizationWorker::run();
}

void OptimizationWorkerComplex::finalize()
{
    OptimizationWorker::finalize();
}


void OptimizationWorkerComplex::findCenter()
{
    mCenter.resize(mNumParameters);
    for(int i=0; i<mCenter.size(); ++i)
    {
        mCenter[i] = 0;
    }
    for(int p=0; p<mNumPoints; ++p)
    {
        if(p == mWorstId) continue;
        for(int i=0; i<mNumParameters; ++i)
        {
            mCenter[i] = mCenter[i]+mParameters[p][i];
        }
    }
    for(int i=0; i<mCenter.size(); ++i)
    {
        mCenter[i] = mCenter[i]/double(mNumPoints-1);
    }
}


//! @brief Applies the forgetting principle in complex algorithm
void OptimizationWorkerComplex::forget()
{
    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    for(int i=0; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    for(int i=0; i<mNumPoints; ++i)
    {
        mObjectives[i] = mObjectives[i]+(maxObj-minObj)*mKf;
    }
}


void OptimizationWorkerComplex::setOptVar(const QString &var, const QString &value)
{
    OptimizationWorker::setOptVar(var, value);

    if(var == "alpha")
    {
        mAlpha = value.toDouble();
    }
    else if(var == "rfak")
    {
        mRfak = value.toDouble();
    }
    else if(var == "gamma")
    {
        mGamma = value.toDouble();
    }
    else if(var == "dontchangestartvalues")
    {
        mDontChangeStartValues = (value=="on");
    }
    else if(var == "start")
    {
        QString value2 = value;
        value2.remove("str");
        int pointId = value2.section(",",0,0).toInt();
        int parId = value2.section(",",1,1).toInt();
        double val = value2.section(",",2,2).toDouble();

        if(mParameters.size() < pointId+1)
        {
            mParameters.resize(pointId+1);
        }
        if(mParameters[pointId].size() < parId+1)
        {
            mParameters[pointId].resize(parId+1);
        }

        mParameters[pointId][parId] = val;
    }
}

double OptimizationWorkerComplex::getOptVar(const QString &var, bool &ok)
{
    double retval = OptimizationWorker::getOptVar(var, ok);
    if(ok)
    {
        return retval;
    }

    ok = true;
    if(var == "alpha")
    {
        return mAlpha;
    }
    else if(var == "rfak")
    {
        return mRfak;
    }
    else if(var == "gamma")
    {
        return mGamma;
    }
    else
    {
        ok = false;
        return 0;
    }
}
