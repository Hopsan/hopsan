//!
//! @file   IntegratorLimited.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Limited Integrator Utility
//!
//$Id$

#include <iostream>
#include <cassert>
#include <math.h>
#include "../HopsanCore.h"
#include "IntegratorLimited.h"

using namespace hopsan;

/*! @class IntegratorLimited
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

IntegratorLimited::IntegratorLimited()
{
}


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
