//!
//! @file   MechanicTorsionalSpring.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a torsional spring
//!
//$Id$

#ifndef MECHANICTORSIONALSPRING_HPP_INCLUDED
#define MECHANICTORSIONALSPRING_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicTorsionalSpring : public ComponentC
{

private:
    double mk;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        return new MechanicTorsionalSpring("TorsionalSpring");
    }

    MechanicTorsionalSpring(const std::string name) : ComponentC(name)
    {
        //Set member attributes
        mTypeName = "MechanicTorsionalSpring";
        mk   = 100.0;

        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        mpP2 = addPowerPort("P2", "NodeMechanicRotational");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("k", "Spring Coefficient", "[N/rad]",  mk);
    }


    void initialize()
    {
        mpP1->writeNode(NodeMechanicRotational::ANGLE, 0.0);
        mpP1->writeNode(NodeMechanicRotational::ANGULARVELOCITY, 0.0);
        mpP1->writeNode(NodeMechanicRotational::TORQUE, 0.0);
        mpP1->writeNode(NodeMechanicRotational::CHARIMP, mk * mTimestep);
        mpP1->writeNode(NodeMechanicRotational::WAVEVARIABLE, 0.0);
        mpP2->writeNode(NodeMechanicRotational::ANGLE, 0.0);
        mpP2->writeNode(NodeMechanicRotational::ANGULARVELOCITY, 0.0);
        mpP2->writeNode(NodeMechanicRotational::TORQUE, 0.0);
        mpP2->writeNode(NodeMechanicRotational::CHARIMP, mk * mTimestep);
        mpP2->writeNode(NodeMechanicRotational::WAVEVARIABLE, 0.0);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double omega1  = mpP1->readNode(NodeMechanicRotational::ANGULARVELOCITY);
        double omega2  = mpP2->readNode(NodeMechanicRotational::ANGULARVELOCITY);
        double lastc1  = mpP1->readNode(NodeMechanicRotational::WAVEVARIABLE);
        double lastc2  = mpP2->readNode(NodeMechanicRotational::WAVEVARIABLE);

        //Spring equations
        double Zc = mk * mTimestep;
        double c1 = lastc2 + 2.0*Zc*omega2;
        double c2 = lastc1 + 2.0*Zc*omega1;

        //Write new values to nodes
        mpP1->writeNode(NodeMechanicRotational::WAVEVARIABLE, c1);
        mpP2->writeNode(NodeMechanicRotational::WAVEVARIABLE, c2);
        mpP1->writeNode(NodeMechanicRotational::CHARIMP, Zc);
        mpP2->writeNode(NodeMechanicRotational::CHARIMP, Zc);
    }
};

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


