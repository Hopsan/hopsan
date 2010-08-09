//!
//! @file   MechanicRotationalInertia.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a mechanic rotational inertia component
//!
//$Id$

#ifndef MECHANICROTATIONALINERTIA_HPP_INCLUDED
#define MECHANICROTATIONALINERTIA_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicRotationalInertia : public ComponentQ
{

private:
    double mJ;
    double mB;
    double mk;
    SecondOrderFilter mFilter;
    Integrator mInt;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        return new MechanicRotationalInertia("RotationalInertia");
    }

    MechanicRotationalInertia(const std::string name) : ComponentQ(name)
    {
        //Set member attributes
        mTypeName = "MechanicRotationalInertia";
        mJ = 1.0;
        mB    = 10;
        mk   = 0.0;

        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        mpP2 = addPowerPort("P2", "NodeMechanicRotational");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("J", "Moment of Inertia", "[kgm^2]", mJ);
        registerParameter("B", "Viscous Friction", "[Ns/rad]", mB);
        registerParameter("k", "Spring Coefficient", "[N/rad]",  mk);
    }


    void initialize()
    {
        double phi1  = mpP1->readNode(NodeMechanicRotational::ANGLE);
        double omega1  = mpP1->readNode(NodeMechanicRotational::ANGULARVELOCITY);
        double T1  = mpP1->readNode(NodeMechanicRotational::TORQUE);

        double num [] = {0.0, 1.0, 0.0};
        double den [] = {mJ, mB, mk};
        mFilter.initialize(mTime, mTimestep, num, den, -T1, -omega1);
        mInt.initialize(mTime, mTimestep, -omega1, -phi1);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double Zx1  = mpP1->readNode(NodeMechanicRotational::CHARIMP);
        double c1  = mpP1->readNode(NodeMechanicRotational::WAVEVARIABLE);
        double Zx2  = mpP2->readNode(NodeMechanicRotational::CHARIMP);
        double c2  = mpP2->readNode(NodeMechanicRotational::WAVEVARIABLE);

        //Mass equations
        double num[3] = {0.0, 1.0, 0.0};
        double den[3] = {mJ, mB+Zx1+Zx2, mk};

        mFilter.setNumDen(num, den);
        double omega2 = mFilter.value(c1-c2);
        double omega1 = -omega2;
        double phi2 = mInt.value(omega2);
        double phi1 = -phi2;
        double T1 = c1 + Zx1*omega1;
        double T2 = c2 + Zx2*omega2;

        //Write new values to nodes
        mpP1->writeNode(NodeMechanicRotational::TORQUE, T1);
        mpP2->writeNode(NodeMechanicRotational::TORQUE, T2);
        mpP1->writeNode(NodeMechanicRotational::ANGULARVELOCITY, omega1);
        mpP2->writeNode(NodeMechanicRotational::ANGULARVELOCITY, omega2);
        mpP1->writeNode(NodeMechanicRotational::ANGLE, phi1);
        mpP2->writeNode(NodeMechanicRotational::ANGLE, phi2);
        //Update Filter:
        //mFilter.update(c1-c2);
    }
};

#endif // MECHANICROTATIONALINERTIA_HPP_INCLUDED

