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

    class DLLIMPORTEXPORT IntegratorLimited
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
