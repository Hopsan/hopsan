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
//! @file   OptimizationWorkerParameterSweep.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-19
//! @version $Id$
//!
//! @brief Contains an optimization worker object for parameter sweeps
//!


#ifndef OPTIMIZATIONWORKERPARAMETERSWEEP_H
#define OPTIMIZATIONWORKERPARAMETERSWEEP_H

#include "OptimizationWorker.h"

class OptimizationWorkerParameterSweep : public OptimizationWorker
{
public:
    OptimizationWorkerParameterSweep(OptimizationHandler *pHandler);

    virtual void init();
    virtual void run();
    virtual void finalize();

    virtual void setOptVar(const QString &var, const QString &value);
    virtual double getOptVar(const QString &var, bool &ok);

    void evaluateAllPoints();
    void printLogFile();

private:
    QVector< QVector<double> > mAllPoints;
    QVector<double> mAllObjectives;
    int mNumThreads;
    int mLength;
};

#endif // OPTIMIZATIONWORKERPARAMETERSWEEP_H
