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
//! @file   SecondOrderFilter.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains the Core Second Order Filter class
//!
//$Id$

#ifndef SECONDORDERFILTER_H_INCLUDED
#define SECONDORDERFILTER_H_INCLUDED

#include <deque>
#include "../win32dll.h"
#include "Delay.h"

namespace hopsan {

    /*
            num[0]*s^2 + num[1]*s + num[2]
    G = --------------------------------------
            den[0]*s^2 + den[1]*s + den[2]
    */

    class DLLIMPORTEXPORT SecondOrderFilter
    {
    public:
        SecondOrderFilter();
        void initialize(double timestep, double num[3], double den[3], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setNum(double num[3]);
        void setDen(double den[3]);
        void setNumDen(double num[3], double den[3]);
        void setMinMax(double min, double max);
        double update(double u);
	double value();

    private:
        double mValue;
        double mDelayU[2], mDelayY[2];
        double mCoeffU[3];
        double mCoeffY[3];
        double mMin, mMax;
        double mTimeStep;
    };
}

#endif // SECONDORDERFILTER_H_INCLUDED
