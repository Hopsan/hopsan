#ifndef SIGNALDEADZONE_HPP_INCLUDED
#define SIGNALDEADZONE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalDeadZone : public ComponentSignal
{


private:
    double mStartDead;
    double mEndDead;

    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalDeadZone("DeadZone");
    }

    SignalDeadZone(const string name) : ComponentSignal(name)
    {
        mTypeName = "SignalDeadZone";
        mStartDead = -1.0;
        mEndDead = 1.0;

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("StartDead", "Start of Dead Zone", "-", mStartDead);
        registerParameter("EndDead", "End of Dead Zone", "-", mEndDead);
    }

    void initialize()
    {
        //Nothing to initilize
    }

    void simulateOneTimestep()
    {
        //get variable values from nodes
        double input = mpIn->readNode(NodeSignal::VALUE);

        //deadzone equations
        double output;

        if (input < mStartDead)
        {
            output = input - mStartDead;
        }
        else if (input > mStartDead && input < mEndDead)
        {
            output = 0;
        }
        else
        {
            output = input - mEndDead;
        }

        //write new values to node
        mpOut->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALDEADZONE_HPP_INCLUDED
