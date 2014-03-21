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
//! @file   OptimizationWorkerComplexRF.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Complex-RF algorithm
//!

#ifndef OPTIMIZATIONWORKERCOMPLEXRF_H
#define OPTIMIZATIONWORKERCOMPLEXRF_H

#include "OptimizationWorkerComplex.h"

class OptimizationHandler;

class OptimizationWorkerComplexRF : public OptimizationWorkerComplex
{
public:
    OptimizationWorkerComplexRF(OptimizationHandler *pHandler);

    void init();
    void run();
    void finalize();
};

#endif // OPTIMIZATIONWORKERCOMPLEXRF_H
