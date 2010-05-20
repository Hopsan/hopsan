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
    double mSignal;
    Port *mpIn, *mpP1;

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
        mSignal = 0.0;

        //Add ports to the component
        mpIn = addReadPort("in", "NodeSignal");
        mpP1 = addPowerPort("P1", "NodeMechanic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("x0", "Initial Position", "[m]", mStartPosition);
        registerParameter("v0", "Initial Velocity", "[m/s]", mStartVelocity);
        registerParameter("Force", "Generated force", "[N]", mSignal);
    }


    void initialize()
    {
        mpP1->writeNode(NodeMechanic::POSITION, mStartPosition);
        mpP1->writeNode(NodeMechanic::VELOCITY, mStartVelocity);
        mpP1->writeNode(NodeMechanic::CHARIMP, 0.0);
        mpP1->writeNode(NodeMechanic::WAVEVARIABLE, 0.0);
    }


    void simulateOneTimestep()
    {
        double signal;
        //Get variable values from nodes
        if(mpIn->isConnected())
            signal  = mpIn->readNode(NodeSignal::VALUE);
        else
            signal = mSignal;

        //Spring equations
        double c = signal;
        double Zc = 0.0;

        //Write new values to nodes
        mpP1->writeNode(NodeMechanic::WAVEVARIABLE, c);
        mpP1->writeNode(NodeMechanic::CHARIMP, Zc);
    }
};

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
