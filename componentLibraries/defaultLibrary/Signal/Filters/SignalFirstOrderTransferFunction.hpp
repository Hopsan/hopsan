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

        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderTransferFunction();
        }

        void configure()
        {
            addInputVariable("in", "","",0.0,&mpIn);
            addOutputVariable("out","Filtered value","",0.0,&mpOut);

            addConstant("a_1", "S^1 numerator coefficient", "", 1, mNum[1]);
            addConstant("a_0", "S^0 numerator coefficient", "", 1, mNum[0]);

            addConstant("b_1", "S^1 denominator coefficient", "", 1, mDen[1]);
            addConstant("b_0", "S^0 denominator coefficient", "", 1, mDen[0]);
        }


        void initialize()
        {
            mTF.initialize(mTimestep, mNum, mDen, *mpIn, *mpOut);

            //Writes out the value for time "zero"
            //(*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mTF.update(*mpIn);
        }
    };
}

#endif
