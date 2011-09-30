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

/*
 *  TurbulentFlowFunction.h
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-12.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

//Equation for turbulent flow through an orifice

#ifndef TURBULENTFLOWFUNCTION_H_INCLUDED
#define TURBULENTFLOWFUNCTION_H_INCLUDED

#include "../win32dll.h"

namespace hopsan {

    class DLLIMPORTEXPORT TurbulentFlowFunction
    {
    public:
        TurbulentFlowFunction();
        TurbulentFlowFunction(double ks);
        double getFlow(double c1, double c2, double Zc1, double Zc2);
        void setFlowCoefficient(double ks);
    private:
        double mKs;
    };
}

#endif // TURBULENTFLOW_H_INCLUDED
