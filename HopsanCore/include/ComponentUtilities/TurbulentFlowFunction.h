/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   TurbulentFlowFunction.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a hysteresis function for valves and signals
//!
//$Id$

//Equation for turbulent flow through an orifice

#ifndef TURBULENTFLOWFUNCTION_H_INCLUDED
#define TURBULENTFLOWFUNCTION_H_INCLUDED

#include <cmath>

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class TurbulentFlowFunction
{
public:
    inline double getFlow(double c1, double c2, double Zc1, double Zc2) const
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

    inline void setFlowCoefficient(double ks)
    {
        mKs = ks;
    }

private:
    double mKs;
};

}

#endif // TURBULENTFLOW_H_INCLUDED
