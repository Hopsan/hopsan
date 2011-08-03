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
//! @file   FirstOrderTransferFunction.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains the Core First Order Transfer Function class
//!
//$Id$

#ifndef FIRSTORDERTRANSFERFUNCTION_H_INCLUDED
#define FIRSTORDERTRANSFERFUNCTION_H_INCLUDED

#include <deque>
#include "../win32dll.h"
#include "Delay.hpp"

namespace hopsan {

    /*
            num[0]*s + num[1]
    G = -------------------------
            den[0]*s + den[1]
    */

    class DLLIMPORTEXPORT FirstOrderTransferFunction
    {
    public:
        //FirstOrderTransferFunction();
        void initialize(double timestep, double num[2], double den[2], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setMinMax(double min, double max);
        void setNum(double num[2]);
        void setDen(double den[2]);
        void setNumDen(double num[2], double den[2]);
        double update(double &u);
        double &value();

    private:
        double mValue;
        double mDelayU, mDelayY;
        double mCoeffU[2];
        double mCoeffY[2];
        double mMin, mMax;
        double mTimeStep;
    };
}
#endif
