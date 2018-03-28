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
//! @file   FirstOrderTransferFunction.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains the Core First Order Transfer Function class
//!
//$Id$

#ifndef FIRSTORDERTRANSFERFUNCTION_H_INCLUDED
#define FIRSTORDERTRANSFERFUNCTION_H_INCLUDED

#include "win32dll.h"
#include "Delay.hpp"

namespace hopsan {

    class HOPSANCORE_DLLAPI FirstOrderTransferFunction
    {
    public:
        void initialize(double timestep, double num[2], double den[2], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setBackupLength(size_t nSteps);
        void setMinMax(double min, double max);
        void setNum(double num[2]);
        void setDen(double den[2]);
        void setNumDen(double num[2], double den[2]);
        void restoreBackup(size_t nSteps=1);
        void backup();
        double update(double u);
        double updateWithBackup(double u);
        double value() const;
        double delayedU() const;
        double delayedY() const;
        bool isSaturated() const;

    protected:
        double mValue;
        double mDelayedU, mDelayedY;
        double mCoeffU[2];
        double mCoeffY[2];
        double mMin, mMax;
        double mTimeStep;
        bool mIsSaturated;
        Delay mBackupU, mBackupY;
    };

    class HOPSANCORE_DLLAPI FirstOrderLowPassFilter : public FirstOrderTransferFunction
    {
    public:
        void initialize(double timestep, double wc, double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        double breakFrequency() const;
    };


    class HOPSANCORE_DLLAPI FirstOrderTransferFunctionVariable
    {
    public:
        void initialize(double *pTimestep, double num[2], double den[2], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setMinMax(double min, double max);
        void setNum(double num[2]);
        void setDen(double den[2]);
        void setNumDen(double num[2], double den[2]);
        void recalculateCoefficients();
        double update(double u);
        double value();

    private:
        double mValue;
        double mDelayU, mDelayY;
        double mNum[2];
        double mDen[2];
        double mCoeffU[2];
        double mCoeffY[2];
        double mMin, mMax;
        double *mpTimeStep;
        double mPrevTimeStep;
    };
}
#endif
