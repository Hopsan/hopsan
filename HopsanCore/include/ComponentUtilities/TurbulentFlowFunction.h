/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
