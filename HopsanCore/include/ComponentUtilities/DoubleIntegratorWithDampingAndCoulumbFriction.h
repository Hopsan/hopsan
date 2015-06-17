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
//! @file   DoubleIntegratorWithDampingAndCoulumbFriction.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-03
//!
//! @brief Core utility for double integrator with provision for some damping and coulomb friction
//!
//$Id$

#ifndef DOUBLEINTEGRATORWITHDAMPINGANDCOULUMBFRICTION_H_INCLUDED
#define DOUBLEINTEGRATORWITHDAMPINGANDCOULUMBFRICTION_H_INCLUDED

#include "win32dll.h"

namespace hopsan {

    //! @ingroup ComponentUtilityClasses
    class DLLIMPORTEXPORT DoubleIntegratorWithDampingAndCoulombFriction
    {
    public:
        void initialize(double timestep, double w0, double Fs, double Fk, double u0, double y0, double sy0);
        void initializeValues(double u0, double y0, double sy0);
        void setDamping(double w0);
        void setFriction(double Fs, double Fk);
        void integrate(double u);
        void integrateWithUndo(double u);
        void redoIntegrate(double u);
        double valueFirst();
        double valueSecond();

    private:
        double mDelayU, mDelayY, mDelaySY;
        double mDelayUbackup, mDelayYbackup, mDelaySYbackup;
        double mTimeStep;
        double mW0, mUs, mUk;
        int movement;
    };
}

#endif // DOUBLEINTEGRATORWITHDAMPINGANDCOULUMBFRICTION_H_INCLUDED
