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
//! @file   AuxiliaryMathematicaWrapperFunctions.h
//! @author Flumes
//! @date   2012-01-03
//!
//! @brief Contains small auxiliary Mathematica syntax wrapper functions that can be useful when creating components from Mathematica Compgen
//!
//$Id$

#ifndef AUXILIARYMATHEMATICAWRAPPERFUNCTIONS_H
#define AUXILIARYMATHEMATICAWRAPPERFUNCTIONS_H

#include <cmath>

namespace hopsan {

//! @brief Wrapper function, for using Mathematica syntax
inline double Power(const double x, const double y)
{
    return pow(x, y);
}

//! @brief Wrapper function, for using Mathematica syntax
inline double Sin(const double x)
{
    return sin(x);
}


//! @brief Wrapper function, for using Mathematica syntax
inline double Cos(const double x)
{
    return cos(x);
}

//! @brief Wrapper function, for using Mathematica syntax
inline double Tan(const double x)
{
    return tan(x);
}

//! @brief function for using Mathematica syntax
inline double Cot(const double x)
{
    return 1.0/tan(x);
}

//! @brief function for using Mathematica syntax
inline double Csc(const double x)
{
    return 1.0/sin(x);
}

//! @brief function for using Mathematica syntax
inline double Sec(const double x)
{
    return 1.0/cos(x);
}

//! @brief function for using Mathematica syntax
inline double SecL(const double x)
{
    return 1.0/cos(x);
}

//! @brief function for using Mathematica syntax
inline double DxSecL(const double x)
{
    return tan(x)/cos(x);
}

//! @brief Wrapper function, for using Mathematica syntax
inline double Sqrt(const double x)
{
    return sqrt(x);
}

//! @brief Wrapper function, for using Mathematica syntax
inline double Abs(const double x)
{
    return fabs(x);
}

}

#endif // AUXILIARYMATHEMATICAWRAPPERFUNCTIONS_H
