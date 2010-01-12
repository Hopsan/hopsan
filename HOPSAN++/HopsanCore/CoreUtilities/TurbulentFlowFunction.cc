/*
 *  TurbulentFlow.cc
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-XX.
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
    //double k1 = pow(mKs,2) * (Zc1+Zc2)/2.0;
   // double k2 = 4 / (pow(mKs,2) * pow((Zc1+Zc2),2));
    if (c1 > c2)
    {
        return mKs*(sqrt(c1-c2+pow((Zc1+Zc2),2)*pow(mKs,2)/4) - mKs*(Zc1+Zc2)/2);

        //k1 * (sqrt(k2*(c1-c2)+1)-1);
    }
    else
    {
        return mKs*(mKs*(Zc1+Zc2)/2 - sqrt(c2-c1+pow((Zc1+Zc2),2)*pow(mKs,2)/4));

        //return k1*(1-sqrt(k2*(c2-c1)+1));
    }
}


void TurbulentFlowFunction::setFlowCoefficient(double ks)
{
    mKs = ks;
}





