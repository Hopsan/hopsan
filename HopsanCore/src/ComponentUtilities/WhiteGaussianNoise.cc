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
//! @file   WhiteGaussianNoise.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-06-09
//!
//! @brief Contains a white Gaussian noise generator
//!
//$Id$

#include "ComponentUtilities/WhiteGaussianNoise.h"
#include <stdlib.h>
// For MSVC we need this define to get access to M_PI
#define _USE_MATH_DEFINES
#include <cmath>


using namespace hopsan;

double WhiteGaussianNoise::getValue()
{
    // Calc Gaussian random value
     double random1 = 0;
     while(random1 == 0)
     {
        random1 = (double)rand() / (double)RAND_MAX;
     }
     double random2 = (double)rand() / (double)RAND_MAX;
     return sqrt((-2.0)*log(random1))*cos(2.0*M_PI*random2);
}
