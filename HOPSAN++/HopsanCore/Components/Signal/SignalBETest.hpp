/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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
            return new SignalBETest("Test");
        }

        SignalBETest(const std::string name) : ComponentSignal(name)
        {
            mGain = 1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            test = true;

            registerParameter("k", "Gain value", "[-]", mGain);
            registerParameter("filename", "Data Curve", "", mDataCurveFileName);
            registerParameter("tb", "TestBOOL", "", test);

            bool success;
            myDataCurve = new CSVParser(success);
        }


        void initialize()
        {
            if(test)
                addWarningMessage("APAN AR HAR!");
            bool success=true;
            if(myDataCurve)
            {
                delete myDataCurve;
                myDataCurve = new CSVParser(success, mDataCurveFileName);
                if(!success)
                {
                    std::stringstream ss;
                    ss << "Unable to initialize CVS file: " << mDataCurveFileName;
                    addErrorMessage(ss.str());
                    stopSimulation();
                }
            }
            if(success)
            {

                std::stringstream ss;
                //            ss << myDataCurve->mData[0][3] << "  " << myDataCurve->mData[1][3];
                ss << mGain << "  " << myDataCurve->interpolate(mGain);
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
