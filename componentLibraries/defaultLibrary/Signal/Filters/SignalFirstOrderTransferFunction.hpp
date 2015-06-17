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
//! @file   SignalFirstOrderTransferFunction.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-08-03
//!
//! @brief Contains a Signal First Order transfer function
//!
//$Id$

#ifndef SIGNALFIRSTORDERTRANSFERFUNCTION_HPP_INCLUDED
#define SIGNALFIRSTORDERTRANSFERFUNCTION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalFirstOrderTransferFunction : public ComponentSignal
    {

    private:
        FirstOrderTransferFunction mTF;
        double mNum[2], mDen[2];

        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderTransferFunction();
        }

        void configure()
        {
            addInputVariable("in", "","",0.0,&mpND_in);
            addOutputVariable("out","Filtered value","",0.0,&mpND_out);

            addConstant("a_1", "S^1 numerator coefficient", "-", 1, mNum[1]);
            addConstant("a_0", "S^0 numerator coefficient", "-", 1, mNum[0]);

            addConstant("b_1", "S^1 denominator coefficient", "-", 1, mDen[1]);
            addConstant("b_0", "S^0 denominator coefficient", "-", 1, mDen[0]);
        }


        void initialize()
        {
            mTF.initialize(mTimestep, mNum, mDen, *mpND_in, *mpND_out);

            //Writes out the value for time "zero"
            //(*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF.update(*mpND_in);
        }
    };
}

#endif
