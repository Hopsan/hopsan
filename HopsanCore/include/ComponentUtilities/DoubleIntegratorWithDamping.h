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
//! @file   DoubleIntegratorWithDamping.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Core utility for double integrator with provision for some damping
//!
//$Id$

#ifndef DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED
#define DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED

#include "win32dll.h"

namespace hopsan {

    //! @ingroup ComponentUtilityClasses
    class HOPSANCORE_DLLAPI DoubleIntegratorWithDamping
    {
    public:
        void initialize(double timestep, double w0, double u0=0.0, double y0=0.0, double sy0=0.0);
        void initializeValues(double u0, double y0, double sy0);
        void setDamping(double w0);
        void integrate(double u);
        void integrateWithUndo(double u);
        void redoIntegrate(double u);
        double valueFirst();
        double valueSecond();

    private:
        double mDelayU, mDelayY, mDelaySY;
        double mDelayUbackup, mDelayYbackup, mDelaySYbackup;
        double mTimeStep;
        double mW0;
    };
}

#endif // DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED
