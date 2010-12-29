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
        double *input, *output;
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

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("w", "Break frequency", "rad/s", mW);
            registerParameter("d", "Damp coefficient", "-", mD);
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
            (*output) = mStartY;
        }


        void simulateOneTimestep()
        {
            (*output) = mFilter.update((*input));
        }
    };
}

#endif // SIGNALHP2FILTER_HPP_INCLUDED


