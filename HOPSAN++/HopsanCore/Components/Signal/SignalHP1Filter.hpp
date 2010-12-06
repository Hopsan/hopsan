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

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHP1Filter : public ComponentSignal
    {

    private:
        FirstOrderFilter mFilter;
        double mW, mMin, mMax;
        double mStartY;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalHP1Filter("Filter");
        }

        SignalHP1Filter(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalHP1Filter";
            mStartY = 0.0;

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mW=1000.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("w", "Break frequency", "rad/s", mW);
        }


        void initialize()
        {
            double num[2];
            double den[2];

            num[0] = 1.0/mW;
            num[1] = 0.0;
            den[0] = 1.0/mW;
            den[1] = 1.0;

            mFilter.initialize(mTimestep, num, den, mStartY, mStartY, mMin, mMax);

            //Writes out the value for time "zero"
            mpOut->writeNode(NodeSignal::VALUE, mStartY);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, mFilter.update(u));
        }
    };
}

#endif // SIGNALHP1FILTER_HPP_INCLUDED


