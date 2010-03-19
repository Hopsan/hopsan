//$Id$

#ifndef MECHANICFORCETRANSFORMER_HPP_INCLUDED
#define MECHANICFORCETRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicForceTransformer : public ComponentC
{

private:
    double mStartPosition;
    double mStartVelocity;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        //std::cout << "running force transformer creator" << std::endl;
        return new MechanicForceTransformer("ForceTransformer");
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
        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addPowerPort("out", "NodeMechanic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("x0", "Initial Position", "[m]", mStartPosition);
        registerParameter("v0", "Initial Velocity", "[m/s]", mStartVelocity);
    }


	void initialize()
    {
        mpOut->writeNode(NodeMechanic::POSITION, mStartPosition);
        mpOut->writeNode(NodeMechanic::VELOCITY, mStartVelocity);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double signal  = mpIn->readNode(NodeSignal::VALUE);

        //Spring equations
        double c = signal;
        double Zc = 0.0;

        //Write new values to nodes
        mpOut->writeNode(NodeMechanic::WAVEVARIABLE, c);
        mpOut->writeNode(NodeMechanic::CHARIMP, Zc);
    }
};

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
