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
//! @file   SignalIntegratorLimited.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-17
//!
//! @brief Contains a Signal Integrator Component with Limited Output
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegratorLimited : public ComponentSignal
    {

    private:
        double mMin, mMax;
        double mPrevU, mPrevY;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited();
        }

        void configure()
        {

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("y_min", "Lower output limit", "[-]", mMin);
            registerParameter("y_max", "Upper output limit", "[-]", mMax);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            limitValue(startY, mMin, mMax);
            mPrevU = startY;
            mPrevU = startY;
        }


        void simulateOneTimestep()
        {
            //Filter equations
            //Bilinear transform is used
            (*mpND_out) = mPrevY + mTimestep/2.0*((*mpND_in) + mPrevU);

            limitValue((*mpND_out), mMin, mMax);

            //Update filter:
            mPrevU = (*mpND_in);
            mPrevY = (*mpND_out);
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED_HPP_INCLUDED


