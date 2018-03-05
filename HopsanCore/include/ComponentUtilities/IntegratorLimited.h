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
//! @file   IntegratorLimited.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains the Core Utility Limited Integrator class
//!
//$Id$

#ifndef INTEGRATORLIMITED_H_INCLUDED
#define INTEGRATORLIMITED_H_INCLUDED

#include <deque>
#include "win32dll.h"
#include "Delay.hpp"

namespace hopsan {

    class HOPSANCORE_DLLAPI IntegratorLimited
    {
    public:
        void initialize(double timestep, double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setMinMax(double min, double max);
        double update(double u);
	double value();

    private:
        double mDelayU, mDelayY;
        double mMin, mMax;
        double mTimeStep;
//	bool mIsInitialized;
    };
}

#endif // INTEGRATOR_H_INCLUDED
