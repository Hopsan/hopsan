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
//! @file   SignalBETest.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Test Component
//!
//$Id$

#ifndef SIGNALTEST_HPP_INCLUDED
#define SIGNALTEST_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <algorithm>
#include <sstream>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalBETest : public ComponentSignal
    {

    private:
        double mGain;
        Port *mpIn, *mpOut;

        double *mpND_in, *mpND_out;

        std::string mDataCurveFileName;

        bool test;
        CSVParser *myDataCurve;

    public:
        static Component *Creator()
        {
            return new SignalBETest();
        }

        void configure()
        {
            mGain = 1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            test = true;

            registerParameter("k", "Gain value", "[-]", mGain);
            registerParameter("filename", "Data Curve", "", mDataCurveFileName);
            registerParameter("tb", "TestBOOL", "", test);

            myDataCurve = 0;
        }


        void initialize()
        {
            if(test)
                addWarningMessage("APAN AR HAR!");
            bool success=true;
            if(myDataCurve != 0)
            {
                delete myDataCurve;
            }

            myDataCurve = new CSVParser(success, mDataCurveFileName);
            if(!success)
            {
                std::stringstream ss;
                ss << "Unable to initialize CVS file: " << mDataCurveFileName;
                addErrorMessage(ss.str());
                stopSimulation();
            }

            if(success)
            {

                std::stringstream ss;
                //            ss << myDataCurve->mData[0][3] << "  " << myDataCurve->mData[1][3];
                //bool ok;
                ss << mGain << "  " << myDataCurve->interpolate(mGain, 1);
                addInfoMessage(ss.str());

                mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
                mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
            }
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mGain * (*mpND_in);
        }
    };
}

#endif // SIGNALTEST_HPP_INCLUDED
