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
//! @file   SignalIntegrator2.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Integrator Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED2_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED2_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegratorLimited2 : public ComponentSignal
    {

    private:
        IntegratorLimited mIntegrator;
        double mStartY;
        double mMin, mMax;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited2();
        }

        void configure()
        {
            mStartY = 0.0;

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            registerParameter("y_min", "Lower output limit", "[-]", mMin, Constant);
            registerParameter("y_max", "Upper output limit", "[-]", mMax, Constant);

            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            mIntegrator.initialize(mTimestep, (*mpND_in), mStartY, mMin, mMax);

            (*mpND_out) = (*mpND_in);
            limitValue((*mpND_out), mMin, mMax);
        }


        void simulateOneTimestep()
        {

            //Filter equation
            (*mpND_out) = mIntegrator.update((*mpND_in));
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED2_HPP_INCLUDED


