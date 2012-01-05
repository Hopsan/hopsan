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
//! @file   Integrator.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains the Core Utility Integrator class
//!
//$Id$

#ifndef INTEGRATOR_H_INCLUDED
#define INTEGRATOR_H_INCLUDED

#include "win32dll.h"

namespace hopsan {

class DLLIMPORTEXPORT Integrator
{
    public:
        Integrator();
        void initialize(double timestep, double u0=0.0, double y0=0.0);
        void initializeValues(double u0, double y0);
        double update(double &u);
        double value();

    private:
        double mDelayU, mDelayY;
        double mTimeStep;
    };

}

#endif // INTEGRATOR_H_INCLUDED
