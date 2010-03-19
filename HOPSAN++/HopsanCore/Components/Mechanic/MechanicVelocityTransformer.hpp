//$Id$

#ifndef MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicVelocityTransformer : public ComponentQ
{

private:
    Integrator mInt;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        //std::cout << "running velocity transformer creator" << std::endl;
        return new MechanicVelocityTransformer("VelocityTransformer");
    }

    MechanicVelocityTransformer(const string name,
                    const double timestep    = 0.001)
    : ComponentQ(name, timestep)
    {
        //Set member attributes
        mTypeName = "MechanicVelocityTransformer";

		//Add ports to the component
        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addPowerPort("out", "NodeMechanic");

        //Register changable parameters to the HOPSAN++ core
    }


	void initialize()
    {
        double signal  = mpIn->readNode(NodeSignal::VALUE);
        mInt.initialize(mTime, mTimestep, signal, 0.0);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double signal  = mpIn->readNode(NodeSignal::VALUE);
        double c =mpOut->readNode(NodeMechanic::WAVEVARIABLE);
        double Zc =mpOut->readNode(NodeMechanic::CHARIMP);


        //Spring equations
        double v = signal;
        double x = mInt.value(v);
        double F = c + Zc*v;

        //Write new values to nodes
       mpOut->writeNode(NodeMechanic::POSITION, x);
       mpOut->writeNode(NodeMechanic::VELOCITY, v);
       mpOut->writeNode(NodeMechanic::FORCE, F);
    }
};

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED




