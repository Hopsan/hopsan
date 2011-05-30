/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, BjÃ¶rn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at LinkÃ¶ping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

#ifndef SECONDORDERTRANSFERFUNCTION_H_INCLUDED
#define SECONDORDERTRANSFERFUNCTION_H_INCLUDED

#include "../win32dll.h"
#include "Delay.h"

namespace hopsan {

    class DLLIMPORTEXPORT SecondOrderTransferFunction
    {
    public:
        SecondOrderTransferFunction();
        SecondOrderTransferFunction(double num [3], double den [3], double timestep);
        void update(double signal);
        void setCoefficients(double num [3], double den [3], double timestep);
        double value();
        void initialize(double initValueU, double initValueY);

    private:
        double u0, u1, u2;
        double y0, y1, y2;
        double a [3];
        double b [3];
        double mDelayu[2];
        double mDelayy[2];
        double mnum [3];
        double mden [3];
        double mTimestep;
    };
}

#endif // SECONDORDERTRANSFERFUNCTION_H_INCLUDED
