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
//! @file   DoubleIntegratorWithDamping.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Core utility for double integrator with provision for some damping
//!
//$Id$

#ifndef DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED
#define DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED

#include <deque>
#include "../win32dll.h"
#include "Delay.h"

namespace hopsan {

    class DLLIMPORTEXPORT DoubleIntegratorWithDamping
    {
    public:
        DoubleIntegratorWithDamping();
        void initialize(double timestep, double w0, double u0=0.0, double y0=0.0, double sy0=0.0);
        void initializeValues(double u0, double y0, double sy0);
        void setDamping(double w0);
        void integrate(double u);
        void integrateWithUndo(double u);
        void redoIntegrate(double u);
        double valueFirst();
        double valueSecond();

    private:
        double mDelayU, mDelayY, mDelaySY;
        double mDelayUbackup, mDelayYbackup, mDelaySYbackup;
        double mTimeStep;
        double mW0;
    };
}

#endif // DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED
