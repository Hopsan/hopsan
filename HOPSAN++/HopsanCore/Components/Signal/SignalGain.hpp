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
//! @file   SignalGain.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Gain Component
//!
//$Id$

#ifndef SIGNALGAIN_HPP_INCLUDED
#define SIGNALGAIN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <algorithm>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalGain : public ComponentSignal
    {

    private:
        double mGain;
        Port *mpIn, *mpOut;

        double *mpND_in, *mpND_out;

        std::string mDataCurveFileName;

        //CSVParser *myDataCurve;

    public:
        static Component *Creator()
        {
            return new SignalGain("Gain");
        }

        SignalGain(const std::string name) : ComponentSignal(name)
        {
            mGain = 1.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("k", "Gain value", "[-]", mGain);
   //         registerParameter("", "Data Curve", "", mDataCurveFileName);

            //myDataCurve = new CSVParser();
        }


        void initialize()
        {
//            stringstream ss;
////            ss << myDataCurve->mData[0][3] << "  " << myDataCurve->mData[1][3];
//            ss << mGain << "  " << myDataCurve->interpolate(mGain);
//            addInfoMessage(ss.str());

            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mGain * (*mpND_in);
        }
    };
}

#endif // SIGNALGAIN_HPP_INCLUDED
