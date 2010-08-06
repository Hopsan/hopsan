//!
//! @file   MechanicTorqueTransformer.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a mechanic prescribed torque component
//!
//$Id$

#ifndef MECHANICTORQUETRANSFORMER_HPP_INCLUDED
#define MECHANICTORQUETRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicTorqueTransformer : public ComponentC
{

private:
    double mStartAngle;
    double mStartAngularVelocity;
    double mStartTorque;
    double mSignal;
    Port *mpIn, *mpP1;

public:
    static Component *Creator()
    {
        return new MechanicTorqueTransformer("TorqueTransformer");
    }

    MechanicTorqueTransformer(const string name) : ComponentC(name)
    {
        //Set member attributes
        mTypeName = "MechanicForceTransformer";
        mStartAngle = 0.0;
        mStartAngularVelocity = 0.0;
        mStartTorque = 0.0;
        mSignal = 0.0;

        //Add ports to the component
        mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
        mpP1 = addPowerPort("P1", "NodeMechanicRotational");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("phi0", "Initial Angle", "[rad]", mStartAngle);
        registerParameter("omega0", "Initial Angular Velocity", "[rad/s]", mStartAngularVelocity);
        registerParameter("T", "Generated Torque", "[Nm]", mSignal);
    }


    void initialize()
    {
        mpP1->writeNode(NodeMechanicRotational::ANGLE, mStartAngle);
        mpP1->writeNode(NodeMechanicRotational::ANGULARVELOCITY, mStartAngularVelocity);
        mpP1->writeNode(NodeMechanicRotational::TORQUE, mStartTorque);
        mpP1->writeNode(NodeMechanicRotational::CHARIMP, 0.0);
        mpP1->writeNode(NodeMechanicRotational::WAVEVARIABLE, 0.0);
    }


    void simulateOneTimestep()
    {
        double signal;
        //Get variable values from nodes
        if(mpIn->isConnected())
            signal  = mpIn->readNode(NodeSignal::VALUE);
        else
            signal = mSignal;

        //Transformer equations
        double c = signal;
        double Zc = 0.0;

        //Write new values to nodes
        mpP1->writeNode(NodeMechanicRotational::WAVEVARIABLE, c);
        mpP1->writeNode(NodeMechanicRotational::CHARIMP, Zc);
    }
};

#endif // MECHANICTORQUETRANSFORMER_HPP_INCLUDED
