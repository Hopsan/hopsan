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
        return new MechanicForceTransformer("ForceTransformer");
    }

    MechanicForceTransformer(const string name) : ComponentC(name)
    {
        //Set member attributes
        mTypeName = "MechanicForceTransformer";
        mStartPosition = 0.0;
        mStartVelocity = 0.0;

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
