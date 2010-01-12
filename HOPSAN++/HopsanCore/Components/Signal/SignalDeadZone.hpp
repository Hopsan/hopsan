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

        addPort("in", "NodeSignal", in);
        addPort("out", "NodeSignal", out);

        registerParameter("StartDead", "Start of Dead Zone", "-", mStartDead);
        registerParameter("EndDead", "End of Dead Zone", "-", mEndDead);
    }

	void initialize()
	{
        //Nothing to initilize
	}

    void simulateOneTimestep()
    {
        //read from nodes
        Node* p1_ptr = mPorts[in].getNodePtr();
        Node* p2_ptr = mPorts[out].getNodePtr();

        //get variable values from nodes
        double input = p1_ptr->getData(NodeSignal::VALUE);

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
        p2_ptr->setData(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALDEADZONE_HPP_INCLUDED
