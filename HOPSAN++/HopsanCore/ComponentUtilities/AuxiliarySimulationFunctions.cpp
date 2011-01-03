//$Id$

#include "AuxiliarySimulationFunctions.h"

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
void hopsan::limit(double &value, double min, double max)
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
double sign(double x)
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
