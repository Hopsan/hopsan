//$Id$

#include "AuxiliarySimulationFunctions.h"
#include <math.h>

//! Multiplies the value by two.
//! @param[in] input is the value to multiply.
//! @return the input value times two.
double hopsan::multByTwo(double input)
{
    return 2.0*input;
}


//! @brief Limits a value so it is between min and max
//! @param &value Reference pointer to the value
//! @param min Lower limit of the value
//! @param max Upper limit of the value
void hopsan::limitValue(double &value, double min, double max)
{
    if(min>max)
    {
        double temp;
        temp = max;
        max = min;
        min = temp;
    }
    if(value > max)
    {
        value = max;
    }
    else if(value < min)
    {
        value = min;
    }
}


//! @brief Converts a float point number to a boolean
//! @param value Double value to convert, 1.0 means true, 0.0 means false
bool hopsan::doubleToBool(double value)
{
    return(value > 0.5);
}


//! @brief Converts a boolean value to a float point number
//! @param value Boolean to convert, will return 1.0 if true and 0.0 if false
double hopsan::boolToDouble(bool value)
{
    if(value)
    {
        return 1.0;
    }
    return 0.0;
}

//! @brief Returns the sign of a double (-1.0 or +1.0)
//! @param x Value to determine sign on
double hopsan::sign(double x)
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


//! @brief Returns 1.0 if x is positive, else returns 0.0
//! @param x Value to determine if it is positive
double hopsan::onPositive(double x)
{
    if (x < 0.0) { return 0.0; }
    return 1.0;
}



double hopsan::dxOnPositive(double x)
{
    return 0.0;
}


//! @brief Returns 1.0 if x is negative, else returns 0.0
//! @param x Value to determine if it is positive
double hopsan::onNegative(double x)
{
    if (x < 0.0) { return 1.0; }
    return 0.0;
}


double hopsan::dxOnNegative(double x)
{
    return 0.0;
}


double hopsan::signedSquareL(double x, double x0)
{
    return (-sqrt(x0) + sqrt(x0 + fabs(x))) * hopsan::sign(x);
}


double hopsan::dxSignedSquareL(double x, double x0)
{
    return (1.0 / (sqrt(x0 + fabs(x)) * 2.0));
}


double hopsan::squareAbsL(double x, double x0)
{
    return (-sqrt(x0) + sqrt(x0 + fabs(x)));
}


double hopsan::dxSquareAbsL(double x, double x0)
{
    return 1.0 / (sqrt(x0 + fabs(x)) * 2.0) * hopsan::sign(x);
}


//! @brief Returns 1.0 if input variables have same sign, else returns 0.0
double hopsan::equalSigns(double x, double y)
{
    if (hopsan::sign(x) != hopsan::sign(y)) {
        return 0.0;
    }
    return 1.0;
}


//! @brief Overloads void hopsan::limitValue() with a return value.
//! @see void hopsan::limitValue(&value, min, max)
//! @param x Value to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
double hopsan::limit(double x, double xmin, double xmax)
{
    double output = x;
    hopsan::limitValue(output, xmin, xmax);
    return output;
}


//! @brief Sets the derivative of x to zero if x is outside of limits.
//! Returns 1.0 if x is within limits, else 0.0. Used to make the derivative of x zero if limit is reached.
//! @param x Value whos derivative is to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited derivative of x
double hopsan::dxLimit(double x, double xmin, double xmax)
{
    if (x >= xmax) { return 0.0; }
    if (x <= xmin) { return 0.0; }
    return 1.0;
}


//! @brief Overloads double hopsan::limit() to also include sx (derivative of x) as input
//! @see void hopsan::limit(&x, min, max)
//! @param x Value to be limited
//! @param sx Derivative of x
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited x value
double hopsan::limit2(double x, double sx, double xmin, double xmax)
{
    return hopsan::limit(x, xmin, xmax);
}


//! @brief Limits the derivative of x when x is outside of its limits.
//! Returns 1.0 if x is within borders, or if x is outside borders but derivative has opposite sign (so that x can only move back to the limited range).
//! @param x Value whos derivative is to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited derivative of x
double hopsan::dxLimit2(double x, double sx, double xmin, double xmax)
{
    if (x >= xmax && sx >= 0.0) { return 0.0; }
    if (x <= xmin && sx <= 0.0) { return 0.0; }
    return 1.0;
}

//! @brief Wrapper function, for using Mathematica syntax
double hopsan::Power(double x, double y)
{
    return pow(x, y);
}


//! @brief Wrapper function, for using Mathematica syntax
double hopsan::Sin(double x)
{
    return sin(x);
}


//! @brief Wrapper function, for using Mathematica syntax
double hopsan::Cos(double x)
{
    return cos(x);
}


//! @brief Wrapper function, for using Mathematica syntax
double hopsan::Tan(double x)
{
    return tan(x);
}


//! @brief Wrapper function, for using Mathematica syntax
double hopsan::Sqrt(double x)
{
    return Sqrt(x);
}
