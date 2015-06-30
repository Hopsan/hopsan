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
    mCenter.resize(mNumParameters);

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


void OptimizationWorkerComplex::reflect(double distance)
{
    for(int j=0; j<mNumParameters; ++j)
    {
        //Reflect
        double worst = mParameters[mWorstId][j];
        mParameters[mWorstId][j] = mCenter[j] + (mCenter[j]-worst)*distance;

        //Add some random noise
        double maxDiff = getMaxParDiff();
        double r = (double)rand() / (double)RAND_MAX;
        mParameters[mWorstId][j] = mParameters[mWorstId][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
        mParameters[mWorstId][j] = qMin(mParameters[mWorstId][j], mParMax[j]);
        mParameters[mWorstId][j] = qMax(mParameters[mWorstId][j], mParMin[j]);
    }
}

void OptimizationWorkerComplex::reflectWorst()
{
    reflect(mAlpha);
}


void OptimizationWorkerComplex::expand()
{
    reflect(mGamma-mAlpha);
}


void OptimizationWorkerComplex::contract()
{
    reflect(mRho-mAlpha);
}


void OptimizationWorkerComplex::reduce()
{
    for(int i=0; i<mNumPoints; ++i)
    {
        if(i==mBestId) continue;

        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double best = mParameters[mBestId][j];
            mParameters[i][j] = best + mSigma*(mParameters[i][j] - best);
        }
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
    else if(var == "rho")
    {
        mRho = value.toDouble();
    }
    else if(var == "sigma")
    {
        mSigma = value.toDouble();
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
