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
//! @file   WhiteGaussianNoise.cpp
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
