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
//! @file   OptimizationHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-08-02
//! @version $Id$
//!
//! @brief Contains a handler for optimizations
//!
//$Id$

#ifndef OPTIMIZATIONHANDLER_H
#define OPTIMIZATIONHANDLER_H

//Qt includes
#include <QVector>
#include <QStringList>

//Forward declarations
class ModelWidget;
class TerminalWidget;
class TerminalConsole;
class HcomHandler;
class Configuration;

class OptimizationHandler : public QObject
{
    Q_OBJECT

    friend class HcomHandler;
    friend class OptimizationDialog;
public:
    //Enums
    enum OptDataType{Int, Double};
    enum OptAlgorithmType{Uninitialized, Complex, ParticleSwarm};

    //Constructor
    OptimizationHandler(HcomHandler *pHandler);

    //Public access functions
    double getOptimizationObjectiveValue(int idx);
    double getOptVar(QString &var, bool &ok) const;
    void setOptVar(const QString &var, const QString &value, bool &ok);
    double getParameter(const int pointIdx, const int parIdx) const;

    Configuration *mpConfig;
    TerminalConsole *mpConsole;
    HcomHandler *mpHcomHandler;

signals:
    void optimizationFinished();

private:
    //Help functions (Complex-RF)
    void crfInit();
    void crfRun();
    void crfForget();
    void crfFindcenter();
    void crfReflectWorst();
    double crfMaxpardiff();

    //Help functions (Particle Swarm)
    void psInit();
    void psRun();
    void psMoveParticles();
    void psPrintLogOutput();

    //Help functions (all algorithms)
    bool checkForConvergence();
    void plotPoints();
    void plotObjectiveFunctionValues();
    void plotParameters();
    void finalize();
    void calculatebestandworstid();

    //Complex-RF member variables
    double mCrfAlpha, mCrfRfak, mCrfGamma, mCrfKf;
    int mCrfWorstCounter;
    QVector<double> mCrfCenter;

    //Particle swarm member variables
    double mPsOmega, mPsC1, mPsC2;
    bool mPsPrintLogOutput;
    QStringList mPsLogOutput;
    QVector< QVector<double> > mPsVelocities, mPsBestKnowns;
    QVector<double> mObjectives, mPsBestObjectives, mPsBestPoint;
    double mPsBestObj;

    //Member variables (both algorithms)
    int mNumPoints;
    int mEvalId;
    int mNumParameters;
    QVector<double> mParMin, mParMax;
    QVector< QVector<double> > mParameters, mOldParameters;
    OptAlgorithmType mAlgorithm;
    OptDataType mParameterType;
    double mMaxEvals;
    int mWorstId, mBestId, mLastWorstId;
    int mConvergenceReason;
    double mParTol, mFuncTol;
    QVector<ModelWidget *> mModelPtrs;
    bool mPlotPoints;
    bool mPlotObjectiveFunctionValues;
    bool mPlotParameters;
    bool mPlotVariables;
};

#endif // OPTIMIZATIONHANDLER_H
