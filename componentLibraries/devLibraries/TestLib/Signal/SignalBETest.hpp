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
