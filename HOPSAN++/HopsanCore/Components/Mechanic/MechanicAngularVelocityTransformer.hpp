//!
//! @file   MechanicAngularVelocityTransformer.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains an angular velocity transformer component
//!
//$Id$

#ifndef MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicAngularVelocityTransformer : public ComponentQ
{

private:
    double mSignal;
    Integrator mInt;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new MechanicAngularVelocityTransformer("AngularVelocityTransformer");
    }

    MechanicAngularVelocityTransformer(const string name) : ComponentQ(name)
    {
        mSignal = 0.0;

        //Set member attributes
        mTypeName = "MechanicAngularVelocityTransformer";

        //Add ports to the component
        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addPowerPort("out", "NodeMechanicRotational");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("omega", "Generated angular velocity", "[rad/s]", mSignal);
    }


    void initialize()
    {
        double signal;
        if(mpIn->isConnected())
            signal  = mpIn->readNode(NodeSignal::VALUE);
        else
            signal = mSignal;
        mInt.initialize(mTime, mTimestep, signal, 0.0);
    }


    void simulateOneTimestep()
    {
        double signal;
        //Get variable values from nodes
        if(mpIn->isConnected())
            signal  = mpIn->readNode(NodeSignal::VALUE);
        else
            signal = mSignal;
        double c =mpOut->readNode(NodeMechanicRotational::WAVEVARIABLE);
        double Zc =mpOut->readNode(NodeMechanicRotational::CHARIMP);

        //Spring equations
        double omega = signal;
        double phi = mInt.value(omega);
        double T = c + Zc*omega;

        //Write new values to nodes
        mpOut->writeNode(NodeMechanicRotational::ANGLE, phi);
        mpOut->writeNode(NodeMechanicRotational::ANGULARVELOCITY, omega);
        mpOut->writeNode(NodeMechanicRotational::TORQUE, T);
    }
};

#endif // MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED




