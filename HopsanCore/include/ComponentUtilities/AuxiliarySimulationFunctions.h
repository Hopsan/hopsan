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
//! @file   AuxiliarySimulationFunctions.h
//! @author Flumes
//! @date   2010-11-29
//!
//! @brief Contains small auxiliary functions that can be useful when creating components
//!

//$Id$

#ifndef AUXILIARYSIMULATIONFUNCTIONS_H
#define AUXILIARYSIMULATIONFUNCTIONS_H

#include "win32dll.h"
#include <float.h>

namespace hopsan {

//! @brief A const double definition of pi that you can use in your code
//! @ingroup AuxiliarySimulationFunctions
const double pi = 3.14159265358979323846;
// We do not use M_PI here as it is not C++ standard (will not work so easily in MSVC compilation)
// However the value has been copied from M_PI

void HOPSANCORE_DLLAPI limitValue(double &rValue, double min, double max);
bool HOPSANCORE_DLLAPI fuzzyEqual(const double x, const double y, const double epsilon=0.00001);

// ----------Exported Functions converted from auxhop in old Hopsan----------
double HOPSANCORE_DLLAPI signedSquareL(const double x, const double x0);
double HOPSANCORE_DLLAPI dxSignedSquareL(const double x, const double x0);
double HOPSANCORE_DLLAPI squareAbsL(const double x, const double x0);
double HOPSANCORE_DLLAPI dxSquareAbsL(const double x, const double x0);
double HOPSANCORE_DLLAPI Atan2L(const double y, const double x);
double HOPSANCORE_DLLAPI ArcSinL(const double x);
double HOPSANCORE_DLLAPI dxArcSinL(const double x);
double HOPSANCORE_DLLAPI diffAngle(const double fi1, const double fi2);
double HOPSANCORE_DLLAPI CLift(const double alpha, const double CLalpha, const double ap, const double an, const double expclp, const double expcln);
double HOPSANCORE_DLLAPI CDragInd(const double alpha, const double AR, const double e, const double CLalpha, const double ap, const double an, const double expclp, const double expcln);
double HOPSANCORE_DLLAPI CMoment(double alpha, const double Cm0, const double Cmfs, const double ap, const double an, const double expclp, const double expcln);
double HOPSANCORE_DLLAPI segare(const double x, const double d);
double HOPSANCORE_DLLAPI dxSegare(const double x, const double d);
double HOPSANCORE_DLLAPI limit(const double x, const double xmin, const double xmax);
double HOPSANCORE_DLLAPI dxLimit(const double x, const double xmin, const double xmax);
double HOPSANCORE_DLLAPI dxLimit2(const double x, const double sx, const double xmin, const double xmax);
double HOPSANCORE_DLLAPI dxLimit3(const double dx, const double x, const double xmin, const double xmax);
double HOPSANCORE_DLLAPI lowLimit(const double x, const double xmin);
double HOPSANCORE_DLLAPI dxLowLimit(const double x, const double xmin);
double HOPSANCORE_DLLAPI dxLowLimit2(const double x, const double sx, const double xmin);
double HOPSANCORE_DLLAPI div(const double x, const double y);


inline double ifElse(const bool x, const double y1, const double y2)
{
    if (x) { return y1; }
    else { return y2; }
}

// ----------Inline Functions converted from auxhop in old Hopsan----------

//! @brief Returns y1 or y2 depending on the value of x.
//! @ingroup AuxiliarySimulationFunctions
//! @param x input value
//! @param y1 if x is positive
//! @param y2 otherwise
//! @returns Limited derivative of x
inline double ifPositive(const double x, const double y1, const double y2)
{
    if (x >= 0) { return y1; }
    else { return y2; }
}

//! @brief Converts a float point number to a boolean
//! @ingroup AuxiliarySimulationFunctions
//! @param value Double value to convert, 1.0 means true, 0.0 means false
inline bool doubleToBool(const double value)
{
    return(value > 0.5);
}

//! @brief Converts a boolean value to a float point number
//! @ingroup AuxiliarySimulationFunctions
//! @param value Boolean to convert, will return 1.0 if true and 0.0 if false
inline double boolToDouble(const bool value)
{
    if(value)
    {
        return 1.0;
    }
    return 0.0;
}

//! @brief Returns 1.0 if x is positive, else returns 0.0
//! @ingroup AuxiliarySimulationFunctions
//! @param x Value to determine if it is positive
inline double onPositive(const double x)
{
    if (x <= 0.0) { return 0.0; }
    return 1.0;
}


//! @ingroup AuxiliarySimulationFunctions
inline double dxOnPositive(const double /*x*/)
{
    return 0.0;
}

//! @brief Returns 1.0 if x is negative, else returns 0.0
//! @ingroup AuxiliarySimulationFunctions
//! @param x Value to determine if it is positive
inline double onNegative(const double x)
{
    if (x <= 0.0) { return 1.0; }
    return 0.0;
}


//! @ingroup AuxiliarySimulationFunctions
inline double dxOnNegative(const double /*x*/)
{
    return 0.0;
}

//! @ingroup AuxiliarySimulationFunctions
inline double dxAbs(const double x)
{
    if (x < 0.0) { return -1; }
    return 1;
}

//! @ingroup AuxiliarySimulationFunctions
inline double d1Atan2L(const double y, const double x)
{
    //return x/(0.001 + pow(x,2) + pow(y,2));
    return x/(0.001 + x*x + y*y);
}

//! @brief Derivative of ATAN2L with respect to x
//! @ingroup AuxiliarySimulationFunctions
inline double d2Atan2L(const double y, const double x)
{
    //return -y/(0.001 + pow(x,2) + pow(y,2));
    return -y/(0.001 + x*x + y*y);
}

//! @brief Returns the sign of a double (-1.0 or +1.0)
//! @ingroup AuxiliarySimulationFunctions
//! @param x Value to determine sign on
inline double sign(const double x)
{
    if (x>=0.0)
    {
        return 1.0;
    }
    else
    {
        return -1.0;
    }
}

//! @brief Derivative of IfPositive with respect to y1.
//! @ingroup AuxiliarySimulationFunctions
//! @param [in] x input value
//! @returns Limited derivative of x
inline double dtIfPositive(const double x, const double /*y1*/, const double /*y2*/)
{
    if (x >= 0) { return 1.; }
    else { return 0.; }
}

//! @brief Derivative of IfPositive with respect to y1.
//! @ingroup AuxiliarySimulationFunctions
//! @param [in] x input value
//! @returns Limited derivative of x
inline double dfIfPositive(const double x, const double /*y1*/, const double /*y2*/)
{
    if (x <= 0) { return 1.; }
    else { return 0.; }
}

//! @brief Overloads double hopsan::limit() to also include sx (derivative of x) as input
//! @ingroup AuxiliarySimulationFunctions
//! @see void hopsan::limit(&x, min, max)
//! @param x Value to be limited
//! @param sx Derivative of x
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited x value
inline double limit2(const double x, const double /*sx*/, const double xmin, const double xmax)
{
    return hopsan::limit(x, xmin, xmax);
}

//! @brief Apply a lower limit to a value
//! @ingroup AuxiliarySimulationFunctions
//! @param [in] value The value to limit
//! @param [in] limit The lower limit
//! @returns limit if value < limit else value
inline double lowerLimit(const double value, const double limit)
{
    if (value < limit) { return limit; } return value;
}

//! @brief Apply a upper limit to a value
//! @ingroup AuxiliarySimulationFunctions
//! @param [in] value The value to limit
//! @param [in] limit The upper limit
//! @returns limit if value > limit else value
inline double upperLimit(const double value, const double limit)
{
    if (value > limit) { return limit; } return value;
}

//! @brief Converts an angle in degrees to radians
//! @ingroup AuxiliarySimulationFunctions
//! @param[in] deg The angle in degrees
//! @returns The angle in radians
inline double deg2rad(const double deg)
{
    return (pi/180.0)*deg;
}

//! @brief Converts an angle in radians to degrees
//! @ingroup AuxiliarySimulationFunctions
//! @param[in] rad The angle in radians
//! @returns The angle in degrees
inline double rad2deg(const double rad)
{
    return (180.0/pi)*rad;
}

//! @brief Check if input variables have the same sign
//! @ingroup AuxiliarySimulationFunctions
//! @returns true or false
inline bool equalSignsBool(const double x, const double y)
{
    //    //! @warning This will NOT work (double != double)
    //    return (hopsan::sign(x) != hopsan::sign(y)) {
    return ( ((x < 0.0) && ( y < 0.0)) || ((x >= 0.0) && (y >= 0.0)) );
}

//! @brief Check if input variables have the same sign
//! @ingroup AuxiliarySimulationFunctions
//! @returns 1.0 (true) or 0.0 (false)
inline double equalSigns(const double x, const double y)
{
    if ( equalSignsBool(x,y) )
    {
        return 1.0;
    }
    else
    {
        return 0.0;
    }
}


// ----------Inline Functions Modelica Wrappers----------


//! @brief Returns the integer modulus of x/y
//! @ingroup AuxiliarySimlationFunctions
//! @ingroup ModelicaWrapperFunctions
//! @param x Numerator
//! @param y Denominator
//! @returns x%y
inline double mod(const double x, const double y)
{
    // The inputs will be rounded to closest int value
    return double(int(x+0.5)%int(y+0.5));
}


//! @brief Returns the integer remainder of x/y, such that div(x,y)*y + rem(x,y) = x
//! @ingroup AuxiliarySimlationFunctions
//! @ingroup ModelicaWrapperFunctions
//! @param x Numerator
//! @param y Denominator
//! @returns Integer remainder of x/y
inline double rem(const double x, const double y)
{
    return x - div(x,y)*y;
}

//! @brief Check if x > y
//! @ingroup AuxiliarySimlationFunctions
//! @ingroup ModelicaWrapperFunctions
//! @param[in] x Value x
//! @param[in] y Value y
//! @returns true if x>y
inline double greaterThan(const double x, const double y)
{
    if(x>y) return 1;
    else return 0;
}

//! @brief Check if x >= y
//! @ingroup AuxiliarySimlationFunctions
//! @ingroup ModelicaWrapperFunctions
//! @param[in] x Value x
//! @param[in] y Value y
//! @returns true if x>=y
inline double greaterThanOrEqual(const double x, const double y)
{
    if(x>=y) return 1;
    else return 0;
}


//! @brief Prevents a value from becoming exactly equal to zero
//! @param Value to check
//! @returns Limited value
inline double nonZero(const double x)
{
    const double limit = DBL_MIN*10;
    //! @todo we could turn this into a template function where you can specify the limit value at compile time
    if( (x >= 0.0) && (x < limit) )
    {
        return limit;
    }
    else if( (x < 0.0) && (x > -limit) )
    {
        return -limit;
    }
    else
    {
        return x;
    }
}

}
#endif // AUXILIARYSIMULATIONFUNCTIONS_H
