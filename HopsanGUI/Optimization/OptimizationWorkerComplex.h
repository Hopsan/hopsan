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
//! @file   OptimizationWorkerComplex.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id: OptimizationHandler.cpp 6525 2014-01-30 15:58:59Z petno25 $
//!
//! @brief Contains a base class for optimization worker objects using constrained simplex (complex) algorithms
//!

#ifndef OPTIMIZATIONWORKERCOMPLEX_H
#define OPTIMIZATIONWORKERCOMPLEX_H

#include <QString>

#include "OptimizationWorker.h"

class OptimizationHandler;

class OptimizationWorkerComplex : public OptimizationWorker
{
public:
    OptimizationWorkerComplex(OptimizationHandler *pHandler);
    virtual void init();
    virtual void run();
    virtual void finalize();

    void findCenter();
    void forget();

    virtual void setOptVar(const QString &var, const QString &value);
    virtual double getOptVar(const QString &var, bool &ok);

protected:
    double mAlpha, mRfak, mGamma, mKf;
    bool mDontChangeStartValues;
    int mWorstCounter;
    QVector<double> mCenter;
};

#endif // OPTIMIZATIONWORKERCOMPLEX_H
