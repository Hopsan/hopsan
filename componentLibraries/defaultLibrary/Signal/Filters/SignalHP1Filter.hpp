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
//! @file   SignalHP1Filter.hpp
//! @author Robert Braun <karl.pettersson@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a Signal First Order High Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALHP1FILTER_HPP_INCLUDED
#define SIGNALHP1FILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHP1Filter : public ComponentSignal
    {

    private:
        FirstOrderTransferFunction mTF;
        double mW, mMin, mMax;
        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalHP1Filter();
        }

        void configure()
        {
            addInputVariable("in","","", 0.0, &mpIn);
            addOutputVariable("out", "","",0.0, &mpOut);

            addConstant("omega", "Break frequency", "Frequency", 1000.0, mW);
            addConstant("y_min", "Lower output limit", "", -1.5E+300, mMin);
            addConstant("y_max", "Upper output limit", "", 1.5E+300, mMax);
        }


        void initialize()
        {
            double num[2];
            double den[2];

            num[1] = 1.0/mW;
            num[0] = 0.0;
            den[1] = 1.0/mW;
            den[0] = 1.0;

            mTF.initialize(mTimestep, num, den, (*mpIn), (*mpOut), mMin, mMax);

            // Do not init output as the startvalue given in that port will initialize the filter output
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mTF.update((*mpIn));
        }
    };
}

#endif // SIGNALHP1FILTER_HPP_INCLUDED


