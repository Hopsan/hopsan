//!
//! @file   SignalLP1Filter.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2010-06-10
//!
//! @brief Contains a Signal First Order Low Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALLP1FILTER_HPP_INCLUDED
#define SIGNALLP1FILTER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalLP1Filter : public ComponentSignal
    {

    private:
        FirstOrderFilter mFilter;
        double mW, mMin, mMax;
        double mStartY;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalLP1Filter("Filter");
        }

        SignalLP1Filter(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalLP1Filter";
            mStartY = 0.0;

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mW=1.0e10;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("w", "Break frequency", "rad/s", mW);
        }


        void initialize()
        {
            double num[2];
            double den[2];

            num[0] = 0.0;
            num[1] = 1.0;
            den[0] = 1.0/mW;
            den[1] = 1.0;

            mFilter.initialize(mTime, mTimestep, num, den, mStartY, mStartY, mMin, mMax);

            //Writes out the value for time "zero"
            mpOut->writeNode(NodeSignal::VALUE, mStartY);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, mFilter.value(u));
        }
    };
}

#endif // SIGNALLP1FILTER_HPP_INCLUDED


