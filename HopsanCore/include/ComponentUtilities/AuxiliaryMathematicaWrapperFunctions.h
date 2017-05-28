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
