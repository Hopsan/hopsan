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
//! @file   AuxiliarySimulationFunctions.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Contiains a second order integrator utility with provision for some damping
//!
//$Id$

#include "ComponentUtilities/AuxiliarySimulationFunctions.h"
#include <cmath>

//! @defgroup ComponentUtilities ComponentUtilities
//! @defgroup AuxiliarySimulationFunctions AuxiliarySimulationFunctions
//! @ingroup ComponentUtilities
//! @defgroup ComponentUtilityClasses ComponentUtilityClasses
//! @ingroup ComponentUtilities


//! @brief Limits a value so it is between min and max
//! @ingroup AuxiliarySimulationFunctions
//! @param &rValue Reference pointer to the value
//! @param min Lower limit of the value
//! @param max Upper limit of the value
void hopsan::limitValue(double &rValue, double min, double max)
{
    if(min>max)
    {
        double temp;
        temp = max;
        max = min;
        min = temp;
    }
    if(rValue > max)
    {
        rValue = max;
    }
    else if(rValue < min)
    {
        rValue = min;
    }
}


//! @ingroup AuxiliarySimulationFunctions
double hopsan::signedSquareL(const double x, const double x0)
{
    return (-sqrt(x0) + sqrt(x0 + fabs(x))) * hopsan::sign(x);
}


//! @ingroup AuxiliarySimulationFunctions
double hopsan::dxSignedSquareL(const double x, const double x0)
{
    return (1.0 / (sqrt(x0 + fabs(x)) * 2.0));
}


//! @ingroup AuxiliarySimulationFunctions
double hopsan::squareAbsL(const double x, const double x0)
{
    return (-sqrt(x0) + sqrt(x0 + fabs(x)));
}

//! @ingroup AuxiliarySimulationFunctions
double hopsan::dxSquareAbsL(const double x, const double x0)
{
    return 1.0 / (sqrt(x0 + fabs(x)) * 2.0) * hopsan::sign(x);
}

//! @brief Safe variant of atan2
//! @ingroup AuxiliarySimulationFunctions
double hopsan::Atan2L(const double y, const double x)
{
    if (x >0. || x<0.)
    { return atan2(y,x);}
        else
    {return 0.;}
}

//! @brief Returns 1.0 if input variables have same sign, else returns 0.0
//! @ingroup AuxiliarySimulationFunctions
double hopsan::equalSigns(const double x, const double y)
{
//    //! @warning This will NOT work (double != double)
//    if (hopsan::sign(x) != hopsan::sign(y)) {
//        return 0.0;
//    }
//    return 1.0;

    if ( ((x < 0.0) && ( y < 0.0)) || ((x >= 0.0) && (y >= 0.0)) )
    {
        return 1.0;
    }
    else
    {
        return 0.0;
    }
}

//! @brief Safe variant of asin
//! @ingroup AuxiliarySimulationFunctions
double hopsan::ArcSinL(const double x)
{
    return asin(hopsan::limit(x,-0.999,0.999));
}

//! @brief derivative of AsinL
//! @ingroup AuxiliarySimulationFunctions
double hopsan::dxArcSinL(const double x)
{
    return 1.0/sqrt(1 - pow(limit(x,-0.999,0.999),2));
}

//! @brief difference between two angles, fi1-fi2
//! @ingroup AuxiliarySimulationFunctions
double hopsan::diffAngle(const double fi1, const double fi2)
{   double output;
    double output0 = fi1-fi2;
    double output1 = fi1-fi2 + 2.0*pi;//3.14159;
    double output2 = fi1-fi2 - 2.0*pi;//3.14159;
    if (fabs(output0)> fabs(output1))
      {output = output1;}
    else if (fabs(output0)< fabs(output2))
      {output = output2;}
    else
      {output = output0;}
    //this is a fix to make it work for small angle differences. Something is wrong with the conditions
    output=fi1-fi2;
    return output;
}

//! @brief Induced drag coefficient for aircraft model
//! @ingroup AuxiliarySimulationFunctions
double hopsan::CLift(const double alpha, const double CLalpha, const double ap, const double an, const double expclp, const double expcln)
{
    return sin(2.0*alpha)/sqrt(2.0) + ((-(1.0/sqrt(2.0)) + CLalpha/2.0)*sin(2.0*alpha))/
            (1 + fabs(onNegative(sin(alpha))*pow(sin(alpha)/an,expcln) +
                      onPositive(sin(alpha))*pow(sin(alpha)/ap,expclp)));
}

//! @brief Induced drag coefficient for aircraft model
//! @ingroup AuxiliarySimulationFunctions
double hopsan::CDragInd(const double alpha, const double AR, const double e, const double CLalpha, const double ap, const double an, const double expclp, const double expcln)
{
    return 0.35355*(1.0 - 1.0/
                    (1.0 + fabs(onNegative(sin(alpha))*pow(sin(alpha)/an,expcln) +
                                onPositive(sin(alpha))*pow(sin(alpha)/ap,expclp))))*(1 - cos(2.0*alpha)) +
            ((1 - cos(2.0*alpha))*(sin(2.0*alpha)/sqrt(2.0) +
                                   ((-(1.0/sqrt(2.0)) + CLalpha/2.0)*sin(2.0*alpha))/
                                   (1 + fabs(onNegative(sin(alpha))*pow(sin(alpha)/an,expcln) +
                                             onPositive(sin(alpha))*pow(sin(alpha)/ap,expclp)))))/(2.0*AR*e);
}

//! @brief Overloads void hopsan::limitValue() with a return value.
//! @ingroup AuxiliarySimulationFunctions
//! @see void hopsan::limitValue(&value, min, max)
//! @param x Value to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
double hopsan::limit(const double x, const double xmin, const double xmax)
{
    double output = x;
    hopsan::limitValue(output, xmin, xmax);
    return output;
}


//! @brief Sets the derivative of x to zero if x is outside of limits.
//! @ingroup AuxiliarySimulationFunctions
//! @details Returns 1.0 if x is within limits, else 0.0. Used to make the derivative of x zero if limit is reached.
//! @param x Value whos derivative is to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited derivative of x
double hopsan::dxLimit(const double x, const double xmin, const double xmax)
{
    if (x >= xmax) { return 0.0; }
    if (x <= xmin) { return 0.0; }
    return 1.0;
}



//! @brief Limits the derivative of x when x is outside of its limits.
//! @ingroup AuxiliarySimulationFunctions
//! Returns 1.0 if x is within borders, or if x is outside borders but derivative has opposite sign (so that x can only move back to the limited range).
//! @param x Value whos derivative is to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited derivative of x
double hopsan::dxLimit2(const double x, const double sx, const double xmin, const double xmax)
{
    if (x >= xmax && sx >= 0.0) { return 0.0; }
    if (x <= xmin && sx <= 0.0) { return 0.0; }
    return 1.0;
}
