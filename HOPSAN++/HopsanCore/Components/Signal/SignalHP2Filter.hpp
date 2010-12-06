//!
//! @file   SignalLP1Filter.hpp
//! @author Robert Braun <karl.pettersson@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a Signal Second Order High Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALHP2FILTER_HPP_INCLUDED
#define SIGNALHP2FILTER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHP2Filter : public ComponentSignal
    {

    private:
        SecondOrderFilter mFilter;
        double mW, mD, mMin, mMax;
        double mStartY;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalHP2Filter("Filter");
        }

        SignalHP2Filter(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalHP2Filter";
            mStartY = 0.0;

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mW=1000.0;
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

            num[0] = 1.0/(mW*mW);
            num[1] = 0.0;
            num[2] = 0.0;
            den[0] = 1.0/(mW*mW);
            den[1] = 2.0*mD/mW;
            den[2] = 1.0;

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

#endif // SIGNALHP2FILTER_HPP_INCLUDED


