//$Id$

#ifndef MECHANICTRANSLATIONALSPRING_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRING_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicTranslationalSpring : public ComponentC
{

private:
    double mk;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        return new MechanicTranslationalSpring("TranslationalSpring");
    }

    MechanicTranslationalSpring(const string name) : ComponentC(name)
    {
        //Set member attributes
        mTypeName = "MechanicTranslationalSpring";
        mk   = 100.0;

        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeMechanic");
        mpP2 = addPowerPort("P2", "NodeMechanic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("k", "Spring Coefficient", "[N/m]",  mk);
    }


    void initialize()
    {
        mpP1->writeNode(NodeMechanic::VELOCITY, 0.0);
        mpP1->writeNode(NodeMechanic::FORCE, 0.0);
        mpP1->writeNode(NodeMechanic::CHARIMP, mk * mTimestep);
        mpP1->writeNode(NodeMechanic::WAVEVARIABLE, 0.0);
        mpP2->writeNode(NodeMechanic::VELOCITY, 0.0);
        mpP2->writeNode(NodeMechanic::FORCE, 0.0);
        mpP2->writeNode(NodeMechanic::CHARIMP, mk * mTimestep);
        mpP2->writeNode(NodeMechanic::WAVEVARIABLE, 0.0);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double v1  = mpP1->readNode(NodeMechanic::VELOCITY);
        double v2  = mpP2->readNode(NodeMechanic::VELOCITY);
        double lastc1  = mpP1->readNode(NodeMechanic::WAVEVARIABLE);
        double lastc2  = mpP2->readNode(NodeMechanic::WAVEVARIABLE);

        //Spring equations
        double Zc = mk * mTimestep;
        double c1 = lastc2 + 2.0*Zc*v2;
        double c2 = lastc1 + 2.0*Zc*v1;

        //Write new values to nodes
        mpP1->writeNode(NodeMechanic::WAVEVARIABLE, c1);
        mpP2->writeNode(NodeMechanic::WAVEVARIABLE, c2);
        mpP1->writeNode(NodeMechanic::CHARIMP, Zc);
        mpP2->writeNode(NodeMechanic::CHARIMP, Zc);
    }
};

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


