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
//! @file   SignalSecondOrderFilter.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Second Order Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALSECONORDERFILTER_HPP_INCLUDED
#define SIGNALSECONORDERFILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSecondOrderFilter : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mTF2;
        double mWnum, mDnum, mWden, mDden, mK;
        double mMin, mMax;
        double *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSecondOrderFilter();
        }

        void configure()
        {
            addInputVariable("in","","", 0.0, &mpIn);
            addOutputVariable("out", "","",0.0, &mpOut);

            addConstant("k", "Gain", "-", 1.0, mK);
            addConstant("omega_1", "Numerator break frequency", "Frequency", 1.0e10, mWnum);
            addConstant("delta_1", "Numerator damp coefficient", "", 1.0, mDnum);
            addConstant("omega_2", "Denominator break frequency", "Frequency", 1000, mWden);
            addConstant("delta_2", "Denominator damp coefficient", "", 1.0, mDden);
            addConstant("y_min", "Lower output limit", "", -1.5E+300, mMin);
            addConstant("y_max", "Upper output limit", "", 1.5E+300, mMax);
        }


        void initialize()
        {
            double num[3];
            double den[3];

            num[2] = mK/(mWnum*mWnum);
            num[1] = mK*2.0*mDnum/mWnum;
            num[0] = mK;
            den[2] = 1.0/pow(mWden, 2);
            den[1] = 2.0*mDden/mWden;
            den[0] = 1.0;

            mTF2.initialize(mTimestep, num, den, (*mpIn), (*mpOut), mMin, mMax);

            // Do not write initial value to out port, its startvalue is used to initialize the filter
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mTF2.update(*mpIn);
        }
    };
}

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED
