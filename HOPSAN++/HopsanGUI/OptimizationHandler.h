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

//Forward declarations
class ModelWidget;
class TerminalWidget;
class TerminalConsole;
class HcomHandler;

class OptimizationHandler : public QObject
{
    friend class HcomHandler;
public:
    //Enums
    enum OptDataType{Int, Double};
    enum OptAlgorithmType{Uninitialized, Complex, ParticleSwarm};

    //Constructor
    OptimizationHandler(HcomHandler *pHandler);

    //Public access functions
    double getOptimizationObjectiveValue(int idx);

    void optPlotPoints();
    void optPlotBestWorstObj();

private:
    TerminalConsole *mpConsole;
    HcomHandler *mpHcomHandler;

    //Help functions
    void optComplexInit();
    void optComplexRun();
    void optComplexForget();
    void optComplexCalculatebestandworstid();
    void optComplexFindcenter();
    bool optComlexCheckconvergence();
    double optComplexMaxpardiff();
    void optParticleInit();
    void optParticleRun();

    //Optimization
    int mOptNumPoints;
    int mOptNumParameters;
    QVector<double> mOptParMin, mOptParMax;
    QVector< QVector<double> > mOptParameters, mOptOldParameters;
    QVector< QVector<double> > mOptVelocities, mOptBestKnowns;
    QVector<double> mOptObjectives, mOptBestObjectives, mOptBestPoint;
    OptAlgorithmType mOptAlgorithm;
    OptDataType mOptParameterType;
    int mOptWorstCounter;
    double mOptBestObj;
    double mOptMaxEvals;
    double mOptAlpha, mOptRfak, mOptGamma, mOptKf;
    double mOptOmega, mOptC1, mOptC2;
    double mOptWorst;
    int mOptWorstId, mOptBestId, mOptLastWorstId;
    QVector<double> mOptCenter;
    int mOptConvergenceReason;
    double mOptParTol, mOptFuncTol;
    bool mOptMulticore;
    bool mOptPlotPoints;
    bool mOptPlotBestWorst;
    bool mOptPlotVariables;
    ModelWidget *mpOptModel;
};

#endif // OPTIMIZATIONHANDLER_H
