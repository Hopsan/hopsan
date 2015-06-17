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
//! @file   SignalFirstOrderFilter.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal First Order Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALFIRSTORDERFILTER_HPP_INCLUDED
#define SIGNALFIRSTORDERFILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalFirstOrderFilter : public ComponentSignal
    {

    private:
        FirstOrderTransferFunction mTF;
        double wnum, wden, k;
        double min, max;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderFilter();
        }

        void configure()
        {
            addInputVariable("in","","",0.0,&mpND_in);
            addOutputVariable("out", "Filtered value", "", 0.0, &mpND_out);

            addConstant("k", "Gain", "-", 1, k);
            addConstant("omega_num", "Numerator break frequency", "rad/s", 1E+10, wnum);
            addConstant("omega_den", "Denominator break frequency", "rad/s", 1000.0, wden);
            addConstant("y_min", "Lower output limit", "-", -1.5E+300, min);
            addConstant("y_max", "Upper output limit", "-", 1.5E+300, max);
        }


        void initialize()
        {
            double num[2];
            double den[2];

            num[1] = k/wnum;
            num[0] = k;
            den[1] = 1.0/wden;
            den[0] = 1.0;

            mTF.initialize(mTimestep, num, den, (*mpND_in), (*mpND_out), min, max);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF.update((*mpND_in));
        }
    };
}

#endif // SIGNALFIRSTORDERFILTER_HPP_INCLUDED


