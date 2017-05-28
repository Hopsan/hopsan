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
        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalFirstOrderFilter();
        }

        void configure()
        {
            addInputVariable("in","","",0.0,&mpIn);
            addOutputVariable("out", "Filtered value", "", 0.0, &mpOut);

            addConstant("k", "Gain", "", 1, k);
            addConstant("omega_num", "Numerator break frequency", "Frequency", 1E+10, wnum);
            addConstant("omega_den", "Denominator break frequency", "Frequency", 1000.0, wden);
            addConstant("y_min", "Lower output limit", "", -1.5E+300, min);
            addConstant("y_max", "Upper output limit", "", 1.5E+300, max);
        }


        void initialize()
        {
            double num[2];
            double den[2];

            num[1] = k/wnum;
            num[0] = k;
            den[1] = 1.0/wden;
            den[0] = 1.0;

            mTF.initialize(mTimestep, num, den, (*mpIn), (*mpOut), min, max);
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mTF.update((*mpIn));
        }
    };
}

#endif // SIGNALFIRSTORDERFILTER_HPP_INCLUDED


