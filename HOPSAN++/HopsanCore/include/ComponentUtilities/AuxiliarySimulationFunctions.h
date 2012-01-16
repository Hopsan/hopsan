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

extern "C" {
    void DLLIMPORTEXPORT limitValue(double &value, double min, double max);
    double DLLIMPORTEXPORT sign(double x);

    // ----------Functions converted from auxhop in old Hopsan----------
    double DLLIMPORTEXPORT ifPositive(double x, double y1, double y2);
    double DLLIMPORTEXPORT dtIfPositive(double x, double y1, double y2);
    double DLLIMPORTEXPORT dfIfPositive(double x, double y1, double y2);
    double DLLIMPORTEXPORT signedSquareL(double x, double x0);
    double DLLIMPORTEXPORT dxSignedSquareL(double x, double x0);
    double DLLIMPORTEXPORT squareAbsL(double x, double x0);
    double DLLIMPORTEXPORT dxSquareAbsL(double x, double x0);
    double DLLIMPORTEXPORT Atan2L(double y, double x);
    double DLLIMPORTEXPORT ArcSinL(double x);
    double DLLIMPORTEXPORT dxArcSinL(double x);
    double DLLIMPORTEXPORT diffAngle(double fi1, double fi2);
    double DLLIMPORTEXPORT CLift( double alpha,double CLalpha,double ap,double an,double expclp,double expcln);
    double DLLIMPORTEXPORT CDragInd(double alpha,double AR,double e,double CLalpha,double ap,double an,double expclp,double expcln);
    double DLLIMPORTEXPORT equalSigns(double x, double y);
    double DLLIMPORTEXPORT limit(double x, double xmin, double xmax);
    double DLLIMPORTEXPORT dxLimit(double x, double xmin, double xmax);
    double DLLIMPORTEXPORT limit2(double x, double sx, double xmin, double xmax);
    double DLLIMPORTEXPORT dxLimit2(double x, double sx, double xmin, double xmax);
}

// ----------Inline Functions converted from auxhop in old Hopsan----------

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

}
#endif // AUXILIARYSIMULATIONFUNCTIONS_H
