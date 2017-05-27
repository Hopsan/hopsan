/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSecondOrderTransferFunction();
        }

        void configure()
        {
            addInputVariable("in","","", 0.0, &mpIn);
            addOutputVariable("out", "","",0.0, &mpOut);

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

            mTF2.initialize(mTimestep, num, den, *mpIn, *mpOut);

            // Do not write initial value to out port, its startvalue is used to initialize the filter
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mTF2.update(*mpIn);
        }
    };
}

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED
