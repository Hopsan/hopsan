/*
 *  TurbulentFlow.cc
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-12.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

#include <math.h>
#include "TurbulentFlowFunction.h"

TurbulentFlowFunction::TurbulentFlowFunction()
{
}


TurbulentFlowFunction::TurbulentFlowFunction(double ks)
{
    mKs = ks;
}

double TurbulentFlowFunction::getFlow(double c1, double c2, double Zc1, double Zc2)
{
    if (c1 > c2)
    {
        return mKs*(sqrt(c1-c2+pow((Zc1+Zc2),2)*pow(mKs,2)/4) - mKs*(Zc1+Zc2)/2);
    }
    else
    {
        return mKs*(mKs*(Zc1+Zc2)/2 - sqrt(c2-c1+pow((Zc1+Zc2),2)*pow(mKs,2)/4));
    }
}


void TurbulentFlowFunction::setFlowCoefficient(double ks)
{
    mKs = ks;
}
