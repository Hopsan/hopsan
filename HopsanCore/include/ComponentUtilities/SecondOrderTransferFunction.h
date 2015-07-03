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

    class DLLIMPORTEXPORT SecondOrderTransferFunction
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

    class DLLIMPORTEXPORT SecondOrderTransferFunctionVariable
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
