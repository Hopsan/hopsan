//$Id$

#ifndef MECHANICFORCETRANSFORMER_HPP_INCLUDED
#define MECHANICFORCETRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"

class MechanicForceTransformer : public ComponentC
{

private:
    double mStartPosition;
    double mStartVelocity;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running force transformer creator" << std::endl;
        return new MechanicForceTransformer("DefaultForceTransformerName");
    }

    MechanicForceTransformer(const string name,
                    const double timestep    = 0.001,
                    const double startposition = 0.0,
                    const double startvelocity = 0.0)

    : ComponentC(name, timestep)
    {
        //Set member attributes
        mTypeName = "MechanicForceTransformer";
        mStartPosition = startposition;
        mStartVelocity = startvelocity;

		//Add ports to the component
        addReadPort("in", "NodeSignal", in);
        addPowerPort("out", "NodeMechanic", out);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("x0", "Initial Position", "[m]", mStartPosition);
        registerParameter("v0", "Initial Velocity", "[m/s]", mStartVelocity);
    }


	void initialize()
    {
        mPortPtrs[out]->writeNode(NodeMechanic::POSITION, mStartPosition);
        mPortPtrs[out]->writeNode(NodeMechanic::VELOCITY, mStartVelocity);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double signal  = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Spring equations
        double c = signal;
        double Zc = 0.0;

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeMechanic::WAVEVARIABLE, c);
        mPortPtrs[out]->writeNode(NodeMechanic::CHARIMP, Zc);
    }
};

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
