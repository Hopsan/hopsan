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

#include <deque>
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
