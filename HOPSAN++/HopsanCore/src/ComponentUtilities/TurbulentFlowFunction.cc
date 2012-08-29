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
 *  TurbulentFlow.cc
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-12.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

#include <math.h>
#include "ComponentUtilities/TurbulentFlowFunction.h"

using namespace hopsan;

double TurbulentFlowFunction::getFlow(double c1, double c2, double Zc1, double Zc2)
{
    if (c1 > c2)
    {
        return mKs*(sqrt(c1-c2+(Zc1+Zc2)*(Zc1+Zc2)*mKs*mKs/4.0) - mKs*(Zc1+Zc2)/2.0);
    }
    else
    {
        return mKs*(mKs*(Zc1+Zc2)/2.0 - sqrt(c2-c1+(Zc1+Zc2)*(Zc1+Zc2)*mKs*mKs/4.0));
    }
}


void TurbulentFlowFunction::setFlowCoefficient(double ks)
{
    mKs = ks;
}
