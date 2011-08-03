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
//! @file   DoubleIntegratorWithDampingAndCoulumbFriction.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-03
//!
//! @brief Core utility for double integrator with provision for some damping and coulimb friction
//!
//$Id$

#ifndef DOUBLEINTEGRATORWITHDAMPINGANDCOULUMBFRICTION_H_INCLUDED
#define DOUBLEINTEGRATORWITHDAMPINGANDCOULUMBFRICTION_H_INCLUDED

#include <deque>
#include "../win32dll.h"
#include "Delay.h"

namespace hopsan {

    class DLLIMPORTEXPORT DoubleIntegratorWithDampingAndCoulumbFriction
    {
    public:
        DoubleIntegratorWithDampingAndCoulumbFriction();
        void initialize(double timestep, double w0, double mass, double Fs, double Fk, double u0, double y0, double sy0);
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
        double mW0, mUs, mUk;
    };
}

#endif // DOUBLEINTEGRATORWITHDAMPINGANDCOULUMBFRICTION_H_INCLUDED
