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
