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
    virtual void evaluateCandidateParticles(bool firstTime=true);
    virtual void evaluatePoints(bool firstTime=true);
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

    double mAlpha1, mAlpha2, mAlpha3, mAlpha4, mAlpha5, mAlpha6, mAlpha7;
    int mMethod;

    QList<SharedVectorVariableT> mCandidateVars_x;
    QList<SharedVectorVariableT> mCandidateVars_y;
};
#endif // OPTIMIZATIONWORKERCOMPLEXRFP_H
