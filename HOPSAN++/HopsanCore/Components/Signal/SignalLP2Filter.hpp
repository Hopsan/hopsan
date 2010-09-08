//!
//! @file   SignalLP1Filter.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2010-06-10
//!
//! @brief Contains a Signal Second Order Low Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALLP2FILTER_HPP_INCLUDED
#define SIGNALLP2FILTER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalLP2Filter : public ComponentSignal
    {

    private:
        SecondOrderFilter mFilter;
        double mW, mD, mMin, mMax;
        double mStartY;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalLP2Filter("Filter");
        }

        SignalLP2Filter(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalLP2Filter";
            mStartY = 0.0;

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mW=1.0e10;
            mD=1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("w", "Break frequency", "rad/s", mW);
            registerParameter("d", "Damp coefficient", "-", mD);
        }


        void initialize()
        {
            double num[3];
            double den[3];

            num[0] = 0.0;
            num[1] = 0.0;
            num[2] = 1.0;
            den[0] = 1.0/pow(mW,2);
            den[1] = 2.0*mD/mW;
            den[2] = 1.0;

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

#endif // SIGNALLP2FILTER_HPP_INCLUDED


