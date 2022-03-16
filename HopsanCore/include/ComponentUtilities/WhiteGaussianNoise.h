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
//! @file   WhiteGaussianNoise.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-06-09
//!
//! @brief Contains a helper function that generates white Gaussian noise
//!
//$Id$

#ifndef WHITEGAUSSIANNOISE_H_INCLUDED
#define WHITEGAUSSIANNOISE_H_INCLUDED

#include "win32dll.h"

namespace hopsan {

    //! @ingroup ComponentUtilityClasses
    class HOPSANCORE_DLLAPI WhiteGaussianNoise
    {
    public:
        static double getValue();
    };
}

#endif // WHITEGAUSSIANNOISE_H_INCLUDED
