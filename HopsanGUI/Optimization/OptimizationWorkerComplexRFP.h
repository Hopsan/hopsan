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
//! @file   OptimizationWorkerComplexRFP.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Complex-RFP algorithm
//!

#ifndef OPTIMIZATIONWORKERCOMPLEXRFP_H
#define OPTIMIZATIONWORKERCOMPLEXRFP_H

#include "OptimizationWorkerComplex.h"

class OptimizationHandler;

class OptimizationWorkerComplexRFP : public OptimizationWorkerComplex
{
public:
    OptimizationWorkerComplexRFP(OptimizationHandler *pHandler);

    virtual void init(const ModelWidget *pModel, const QString &modelPath);
    virtual void run();
    virtual void finalize();

    void reInit(int nModels);

    virtual void setOptVar(const QString &var, const QString &value);
    virtual double getOptVar(const QString &var, bool &ok);

    double getParameter(const int pointIdx, const int parIdx) const;

protected:
    virtual void pickCandidateParticles();
    virtual bool evaluateCandidateParticles(bool &rNeedsRescheduling, bool firstTime=true);
    virtual bool evaluatePoints(bool firstTime=true);
    virtual void examineCandidateParticles();
    double triangularDistribution(double min, double mid, double max);
    void generateRandomParticle(QVector<double> &rParticle);
    void generateRandomParticleWeightedToCenter(QVector<double> &rParticle);
    void findCenter();
    void findCenter(QVector< QVector<double> > &particles);
    void plotPoints();
    void setOptimizationObjectiveValue(int idx, double value);

    QVector<ModelWidget *> mUsedModelPtrs;
    QString mModelPath;

    QVector< QVector<double> > mCandidateParticles;
    QVector<double> mCandidateObjectives;
    bool mNeedsIteration;
    bool mFirstReflectionFailed;
    int mFailedReflection;

    double mAlpha1, mAlpha2, mAlpha3, mAlpha4, mAlpha5, mAlpha6, mAlpha7;
    int mMethod;

    //Method 3 members
    int mNumDist, mNumDir, mNumStep;
    int mDistCount, mDirCount, mIterCount;
    QMap<QString, int> mActionCounter;

    QList<SharedVectorVariableT> mCandidateVars_x;
    QList<SharedVectorVariableT> mCandidateVars_y;
};
#endif // OPTIMIZATIONWORKERCOMPLEXRFP_H
