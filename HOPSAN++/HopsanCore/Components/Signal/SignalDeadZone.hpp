#ifndef SIGNALDEADZONE_HPP_INCLUDED
#define SIGNALDEADZONE_HPP_INCLUDED

#include "HopsanCore.h"

class SignalDeadZone : public ComponentSignal
{


private:
    double mStartDead;
    double mEndDead;

    enum{in, out};

public:
    static Component *Creator()
    {
        std::cout << "running DeadZone creator" << std::endl;
        return new SignalDeadZone("DefaultDeadZoneName");
    }

    SignalDeadZone(const string name,
                   const double startdead = -1.0,
                   const double enddead = 1.0,
                   const double timestep = 0.001)
    : ComponentSignal(name, timestep)
    {
        mStartDead = startdead;
        mEndDead = enddead;

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

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
        double input = mPortPtrs[in]->readNode(NodeSignal::VALUE);

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
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALDEADZONE_HPP_INCLUDED
