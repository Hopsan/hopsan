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
//! @file   Integrator.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains the Core Utility Integrator class
//!
//$Id$

#ifndef INTEGRATOR_H_INCLUDED
#define INTEGRATOR_H_INCLUDED

#include "win32dll.h"

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class Integrator
{
public:
    inline void initialize(const double timestep, const double u0=0.0, const double y0=0.0)
    {
        mDelayU = u0;
        mDelayY = y0;
        mTimeStep = timestep;
    }

    inline void initializeValues(const double u0, const double y0)
    {
        mDelayU = u0;
        mDelayY = y0;
    }

    //! @brief Updates the integrator one timestep and returns the new value
    inline double update(const double u)
    {
        //Bilinear transform is used
        mDelayY = mDelayY + mTimeStep/2.0*(u + mDelayU);
        mDelayU = u;
        return mDelayY;
    }

    //! @brief Returns the integrator value
    //! @return The integrated actual value.
    //! @see update(double u)
    inline double value() const
    {
        return mDelayY;
    }

private:
    double mDelayU, mDelayY;
    double mTimeStep;
};

}

#endif // INTEGRATOR_H_INCLUDED
