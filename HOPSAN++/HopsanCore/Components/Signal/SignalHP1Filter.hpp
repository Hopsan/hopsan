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
        double *input, *output;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalHP1Filter("Filter");
        }

        SignalHP1Filter(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalHP1Filter";

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mW=1000.0;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("w", "Break frequency", "rad/s", mW);
        }


        void initialize()
        {
                //Make in1 or in2 be zero if they are not connected
            if(mpIn->isConnected())
            {
                input = mpIn->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                input = new double(0);
            }

                //output must be connected
            if(mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double();
            }

            double num[2];
            double den[2];

            num[0] = 1.0/mW;
            num[1] = 0.0;
            den[0] = 1.0/mW;
            den[1] = 1.0;

            mFilter.initialize(mTimestep, num, den, (*input), (*input), mMin, mMax);

            (*output) = (*input);
        }


        void simulateOneTimestep()
        {
            (*output) = mFilter.update((*input));
        }
    };
}

#endif // SIGNALHP1FILTER_HPP_INCLUDED


