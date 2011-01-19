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





double onPositive(double x)
{
    if (x < 0.0) { return 0.0; }
    return 1.0;
}



double dxOnPositive(double x)
{
    return 0.0;
}



//double onNegative(double x)
//{

//    ret_val = 0.0;
//    if (x < 0.0) { return 0.0; }
//    return 0.0;
//}



//double dxOnNegative(double x)
//{
//    return 0.0;
//}



//double signedSquareL(double x, double x0)
//{
//    return (-sqrt(x0) + sqrt(x0 + fabs(x))) * hopsan::sign(x);
//}

///* -------------------------------------------------------------------- */
//double dxSignedSquareL(double x, double x0)
//{
//    /* System generated locals */
//    double ret_val;

//    /* Builtin functions */
//    double sqrt(double);

///* -------------------------------------------------------------------- */
//    ret_val = 1 / (sqrt(*x0 + dabs(*x)) * 2);
//    return ret_val;
//} /* dxsigsqrl_ */

///* -------------------------------------------------------------------- */
//double sqrabsl_(double *x, double *x0)
//{
//    /* System generated locals */
//    double ret_val;

//    /* Builtin functions */
//    double sqrt(double);

///* -------------------------------------------------------------------- */
//    ret_val = -sqrt(*x0) + sqrt(*x0 + dabs(*x));
//    return ret_val;
//} /* sqrabsl_ */

///* -------------------------------------------------------------------- */
//double dxsqrabsl_(double *x, double *x0)
//{
//    /* System generated locals */
//    double ret_val;

//    /* Builtin functions */
//    double sqrt(double);

//    /* Local variables */
//    extern double sign1_(double *);

///* -------------------------------------------------------------------- */
//    ret_val = 1 / (sqrt(*x0 + dabs(*x)) * 2) * sign1_(x);
//    return ret_val;
//} /* dxsqrabsl_ */

///* -------------------------------------------------------------------- */
//double sign1_(double *x)
//{
//    /* System generated locals */
//    double ret_val;

///* -------------------------------------------------------------------- */
//    ret_val = 1.0;
//    if (*x < 0.0) {
//        ret_val = -1.0;
//    }
//    return ret_val;
//} /* sign1_ */

///* -------------------------------------------------------------------- */
//double equalsigns_(double *x, double *y)
//{
//    /* System generated locals */
//    double ret_val;

//    /* Local variables */
//    extern double sign1_(double *);

///* -------------------------------------------------------------------- */
//    ret_val = 1.0;
//    if (sign1_(x) != sign1_(y)) {
//        ret_val = 0.0;
//    }
//    return ret_val;
//} /* equalsigns_ */

///* -------------------------------------------------------------------- */
//double limitdouble_(double *x, double *xmin, double *xmax)
//{
//    /* System generated locals */
//    double ret_val;

///* -------------------------------------------------------------------- */
//    ret_val = *x;
//    if (*x >= *xmax) {
//        ret_val = *xmax;
//    }
//    if (*x <= *xmin) {
//        ret_val = *xmin;
//    }
//    return ret_val;
//} /* limitdouble_ */

///* -------------------------------------------------------------------- */
//double dxlimitdouble_(double *x, double *xmin, double *xmax)
//{
//    /* System generated locals */
//    double ret_val;

///* -------------------------------------------------------------------- */
//    ret_val = 1.0;
//    if (*x >= *xmax) {
//        ret_val = 0.0;
//    }
//    if (*x <= *xmin) {
//        ret_val = 0.0;
//    }
//    return ret_val;
//} /* dxlimitdouble_ */

///* -------------------------------------------------------------------- */
//double limit2double_(double *x, double *sx, double *xmin, double *xmax)
//{
//    /* System generated locals */
//    double ret_val;

///* -------------------------------------------------------------------- */
//    ret_val = *x;
//    if (*x >= *xmax) {
//        ret_val = *xmax;
//    }
//    if (*x <= *xmin) {
//        ret_val = *xmin;
//    }
//    return ret_val;
//} /* limit2double_ */

///* -------------------------------------------------------------------- */
//double dxlimit2double_(double *x, double *sx, double *xmin, double *xmax)
//{
//    /* System generated locals */
//    double ret_val;

///* -------------------------------------------------------------------- */
//    ret_val = 1.0;
//    if (*x >= *xmax && *sx >= 0.0) {
//        ret_val = 0.0;
//    }
//    if (*x <= *xmin && *sx <= 0.0) {
//        ret_val = 0.0;
//    }
///* 	write(*,*)"x ",x,sx,DxLimit2double */
//    return ret_val;
//} /* dxlimit2double_ */

///* ============================================================ */
