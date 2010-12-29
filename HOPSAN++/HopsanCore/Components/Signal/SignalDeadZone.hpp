#ifndef SIGNALDEADZONE_HPP_INCLUDED
#define SIGNALDEADZONE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDeadZone : public ComponentSignal
    {


    private:
        double mStartDead;
        double mEndDead;
        double *input, *output;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDeadZone("DeadZone");
            }

        SignalDeadZone(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalDeadZone";
            mStartDead = -1.0;
            mEndDead = 1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("StartDead", "Start of Dead Zone", "-", mStartDead);
            registerParameter("EndDead", "End of Dead Zone", "-", mEndDead);
        }

        void initialize()
        {
            input = mpIn->getNodeDataPtr(NodeSignal::VALUE);
            if(mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double();
            }
        }

        void simulateOneTimestep()
        {
            //Deadzone equations
            if ( (*input) < mStartDead)
            {
                (*output) = (*input) - mStartDead;
            }
            else if ( (*input) > mStartDead && (*input) < mEndDead)
            {
                (*output) = 0;
            }
            else
            {
                (*output) = (*input) - mEndDead;
            }
        }
    };
}

#endif // SIGNALDEADZONE_HPP_INCLUDED
