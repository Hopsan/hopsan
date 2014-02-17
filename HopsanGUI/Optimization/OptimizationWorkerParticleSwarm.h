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
//! @file   OptimizationWorkerParticleSwarm.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id: OptimizationHandler.cpp 6525 2014-01-30 15:58:59Z petno25 $
//!
//! @brief Contains an optimization worker object for the particle swarm algorithm
//!

#ifndef OPTIMIZATIONWORKERPARTICLESWARM_H
#define OPTIMIZATIONWORKERPARTICLESWARM_H

//Qt includes
#include <QVector>
#include <QStringList>

//Hopasn includes
#include "OptimizationWorker.h"

class OptimizationHandler;

class OptimizationWorkerParticleSwarm : public OptimizationWorker
{
public:
    OptimizationWorkerParticleSwarm(OptimizationHandler *pHandler);

    void init();
    void run();
    void finalize();

    void moveParticles();
    void printLogOutput();

    void setOptVar(const QString &var, const QString &value);
    double getOptVar(const QString &var, bool &ok);

protected:
    double mPsOmega, mPsC1, mPsC2;
    bool mPrintLogOutput;
    QStringList mLogOutput;
    QVector< QVector<double> > mVelocities, mBestKnowns;
    QVector<double> mBestObjectives, mBestPoint;
    double mPsBestObj;
};

#endif // OPTIMIZATIONWORKERPARTICLESWARM_H
