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
//! @file   SecondOrderTransferFunction.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains the Core Second Order Transfer Function class
//!
//$Id$

#ifndef SECONDORDERTRANSFERFUNCTION_H_INCLUDED
#define SECONDORDERTRANSFERFUNCTION_H_INCLUDED

#include "win32dll.h"
#include "Delay.hpp"

namespace hopsan {

    class HOPSANCORE_DLLAPI SecondOrderTransferFunction
    {
    public:
        void initialize(double timestep, double num[3], double den[3], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300, double sy0=0.0);
        void initializeValues(double u0, double y0);
        void setBackupLength(size_t nStep);
        void setNum(double num[3]);
        void setDen(double den[3]);
        void setNumDen(double num[3], double den[3]);
        void setMinMax(double min, double max);
        void backup();
        void restoreBackup(size_t nSteps=1);
        double update(double u);
        double updateWithBackup(double u);
        double value() const;
        double delayedU() const;
        double delayed2U() const;
        double delayedY() const;
        double delayed2Y() const;
        bool isSaturated() const;

    private:
        double mValue;
        double mDelayedU, mDelayed2U, mDelayedY, mDelayed2Y;
        double mCoeffU[3];
        double mCoeffY[3];
        double mMin, mMax;
        double mTimeStep;
        bool mIsSaturated;
        Delay mBackupU, mBackupY;
    };

    class HOPSANCORE_DLLAPI SecondOrderTransferFunctionVariable
    {
    public:
        void initialize(double *pTimestep, double num[3], double den[3], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setNum(double num[3]);
        void setDen(double den[3]);
        void setNumDen(double num[3], double den[3]);
        void setMinMax(double min, double max);
        double update(double u);
        double value();
        void recalculateCoefficients();

    private:
        double mValue;
        double mDelayU[2], mDelayY[2];
        double mDen[3];
        double mNum[3];
        double mCoeffU[3];
        double mCoeffY[3];
        double mMin, mMax;
        double *mpTimeStep;
        double mPrevTimeStep;
    };
}

#endif
