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
//! @file   SignalSecondOrderTransferFunction.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-04-27
//!
//! @brief Contains a Signal Second Order transfer function
//!
//$Id$

#ifndef SIGNALSECONORDERTRANSFERFUNCTION_HPP_INCLUDED
#define SIGNALSECONORDERTRANSFERFUNCTION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSecondOrderTransferFunction : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mTF2;
        double a0, a1, a2, b0, b1, b2;

        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalSecondOrderTransferFunction();
        }

        void configure()
        {
            addInputVariable("in","","", 0.0, &mpND_in);
            addOutputVariable("out", "","",0.0, &mpND_out);

            addConstant("a_2", "S^2 numerator coefficient", "-", 1, a2);
            addConstant("a_1", "S^1 numerator coefficient", "-", 1, a1);
            addConstant("a_0", "S^0 numerator coefficient", "-", 1, a0);

            addConstant("b_2", "S^2 denominator coefficient", "-", 1, b2);
            addConstant("b_1", "S^1 denominator coefficient", "-", 1, b1);
            addConstant("b_0", "S^0 denominator coefficient", "-", 1, b0);
        }


        void initialize()
        {
            double num[3];
            double den[3];

            num[0] = a0;
            num[1] = a1;
            num[2] = a2;
            den[0] = b0;
            den[1] = b1;
            den[2] = b2;

            mTF2.initialize(mTimestep, num, den, *mpND_in, *mpND_out);

            // Do not write initial value to out port, its startvalue is used to initialize the filter
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF2.update(*mpND_in);
        }
    };
}

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED
