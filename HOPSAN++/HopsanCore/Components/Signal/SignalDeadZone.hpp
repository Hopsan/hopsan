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
        double *mpND_in, *mpND_out;
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
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }

        void simulateOneTimestep()
        {
            //Deadzone equations
            if ( (*mpND_in) < mStartDead)
            {
                (*mpND_out) = (*mpND_in) - mStartDead;
            }
            else if ( (*mpND_in) > mStartDead && (*mpND_in) < mEndDead)
            {
                (*mpND_out) = 0;
            }
            else
            {
                (*mpND_out) = (*mpND_in) - mEndDead;
            }
        }
    };
}

#endif // SIGNALDEADZONE_HPP_INCLUDED
