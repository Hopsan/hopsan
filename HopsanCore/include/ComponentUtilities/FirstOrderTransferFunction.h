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

namespace hopsan {

    class DLLIMPORTEXPORT FirstOrderTransferFunction
    {
    public:
        void initialize(double timestep, double num[2], double den[2], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setMinMax(double min, double max);
        void setNum(double num[2]);
        void setDen(double den[2]);
        void setNumDen(double num[2], double den[2]);
        double update(double u);
        double value() const;
        double delayedU() const;
        double delayedY() const;
        bool isSaturated() const;

    private:
        double mValue;
        double mDelayedU, mDelayedY;
        double mCoeffU[2];
        double mCoeffY[2];
        double mMin, mMax;
        double mTimeStep;
        bool mIsSaturated;
    };


    class DLLIMPORTEXPORT FirstOrderTransferFunctionVariable
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
