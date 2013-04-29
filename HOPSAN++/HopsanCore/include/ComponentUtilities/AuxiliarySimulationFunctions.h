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

namespace hopsan {

//! @brief A const double definition of pi that you can use in your code
//! @ingroup AuxiliarySimulationFunctions
const double pi = 3.14159265358979323846;

void DLLIMPORTEXPORT limitValue(double &rValue, double min, double max);
bool DLLIMPORTEXPORT fuzzyEqual(const double x, const double y, const double epsilon=0.00001);

// ----------Functions converted from auxhop in old Hopsan----------
double DLLIMPORTEXPORT signedSquareL(const double x, const double x0);
double DLLIMPORTEXPORT dxSignedSquareL(const double x, const double x0);
double DLLIMPORTEXPORT squareAbsL(const double x, const double x0);
double DLLIMPORTEXPORT dxSquareAbsL(const double x, const double x0);
double DLLIMPORTEXPORT Atan2L(const double y, const double x);
double DLLIMPORTEXPORT ArcSinL(const double x);
double DLLIMPORTEXPORT dxArcSinL(const double x);
double DLLIMPORTEXPORT diffAngle(const double fi1, const double fi2);
double DLLIMPORTEXPORT CLift(const double alpha, const double CLalpha, const double ap, const double an, const double expclp, const double expcln);
double DLLIMPORTEXPORT CDragInd(const double alpha, const double AR, const double e, const double CLalpha, const double ap, const double an, const double expclp, const double expcln);
double DLLIMPORTEXPORT CMoment(double alpha, const double Cm0, const double Cmfs, const double ap, const double an, const double expclp, const double expcln);
double DLLIMPORTEXPORT equalSigns(const double x, const double y);
double DLLIMPORTEXPORT limit(const double x, const double xmin, const double xmax);
double DLLIMPORTEXPORT dxLimit(const double x, const double xmin, const double xmax);
double DLLIMPORTEXPORT dxLimit2(const double x, const double sx, const double xmin, const double xmax);
double DLLIMPORTEXPORT div(const double x, const double y);


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
    if (x < 0.0) { return 0.0; }
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
    if (x < 0.0) { return 1.0; }
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
//! @param x input value
//! @param y1 dummy
//! @param y2 dummy
//! @returns Limited derivative of x
inline double dtIfPositive(const double x, const double /*y1*/, const double /*y2*/)
{
    if (x >= 0) { return 1.; }
    else { return 0.; }
}

//! @brief Derivative of IfPositive with respect to y1.
//! @ingroup AuxiliarySimulationFunctions
//! @param x input value
//! @param y1 dummy
//! @param y2 dummy
//! @returns Limited derivative of x
inline double dfIfPositive(const double x, const double /*y1*/, const double /*y2*/)
{
    if (x >= 0) { return 1.; }
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

// ----------Inline Functions Modelica Wrappers----------


//! @brief Returns the integer modulus of x/y
//! @ingroup AuxiliarySimlationFunctions
//! @ingroup ModelicaWrapperFunctions
//! @param x Numinator
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
//! @param x Numinator
//! @param y Denominator
//! @returns Integer remainder of x/y
inline double rem(const double x, const double y)
{
    return x - div(x,y)*y;
}

}
#endif // AUXILIARYSIMULATIONFUNCTIONS_H
