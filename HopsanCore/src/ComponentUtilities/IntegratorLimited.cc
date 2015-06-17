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
//! @file   IntegratorLimited.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Limited Integrator Utility
//!
//$Id$

//#include <iostream>
//#include <cassert>
//#include <math.h>
#include "ComponentUtilities/IntegratorLimited.h"

using namespace hopsan;

/*! @class hopsan::IntegratorLimited
 *  @ingroup ComponentUtilityClasses
 *  @brief The IntegratorLimited class implements a integrator using bilinear
 *  transform which integrates a variable with limited output signal and wind-up protection
 *
 *  The class implements
 *   \f[ y = \int_{0}^{t}
 *     \left\{
 *       \begin{array}{ll}
 *         0 & \mbox{if } \int u \, dt + y_0 < y_{min} \\
 *         0 & \mbox{if } \int u \, dt + y_0 > y_{max} \\
 *         u & \mbox{otherwise}
 *       \end{array}
 *     \right. \, dt + y_0\f]
 */
/*   \f[ y =
 *     \left\{
 *       \begin{array}{ll}
 *         y_{min} & \mbox{if } \int u \, dt + y_0 < y_{min} \\
 *         y_{max} & \mbox{if } \int u \, dt + y_0 > y_{max} \\
 *         \displaystyle{\int_{0}^{t} u \, dt + y_0} & \mbox{otherwise}
 *       \end{array}
 *     \right.\f]
 */

//  \f[y=\left\{\begin{array}{ll} y_{min} & \mbox{if } \int u \, dt + y_0 < y_{min} \\ y_{max} & \mbox{if } \int u \, dt + y_0 > y_{max} \\ \displaystyle{\int_{0}^{t} u \, dt + y_0} & \mbox{otherwise} \end{array} \right.\f]
//!

void IntegratorLimited::initialize(double timestep, double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mDelayU = u0;
    mDelayY = std::max(std::min(y0, mMax), mMin);
    mTimeStep = timestep;
}


void IntegratorLimited::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
}


void IntegratorLimited::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


double IntegratorLimited::update(double u)
{
    //Filter equation
    //Bilinear transform is used

    double y = mDelayY + mTimeStep/2.0*(u + mDelayU);
    //cout << "mMin: " << mMin << " mMax: " << mMax << " y: " << y << endl;
    if (y > mMax)
    {
        mDelayY = mMax;
        mDelayU = 0.0;
    }
    else if (y < mMin)
    {
        mDelayY = mMin;
        mDelayU = 0.0;
    }
    else
    {
        mDelayY = y;
        mDelayU = u;
    }

    return mDelayY;
}



//! Observe that a call to this method has to be followed by another call to update(double u)
//! @return The integrated actual value.
//! @see value(double u)
double IntegratorLimited::value()
{
    return mDelayY;
}
