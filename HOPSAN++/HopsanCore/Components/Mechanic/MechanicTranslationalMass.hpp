//$Id$

#ifndef MECHANICTRANSLATIONALMASS_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASS_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicTranslationalMass : public ComponentQ
{

private:
    double mMass;
    double mB;
    double mk;
    SecondOrderFilter mFilter;
    Integrator mInt;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        return new MechanicTranslationalMass("TranslationalMass");
    }

    MechanicTranslationalMass(const std::string name) : ComponentQ(name)
    {
        //Set member attributes
        mTypeName = "MechanicTranslationalMass";
        mMass = 1.0;
        mB    = 10;
        mk   = 0.0;

        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeMechanic");
        mpP2 = addPowerPort("P2", "NodeMechanic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("Mass", "Mass", "[kg]",            mMass);
        registerParameter("B", "Viscous Friction", "[Ns/m]", mB);
        registerParameter("k", "Spring Coefficient", "[N/m]",  mk);
    }


    void initialize()
    {
        //mFilter.initialize(0.0,0.0, mTime);
        double x1  = mpP1->readNode(NodeMechanic::POSITION);
        double v1  = mpP1->readNode(NodeMechanic::VELOCITY);
        double F1  = mpP1->readNode(NodeMechanic::FORCE);
        //cout << "x0 = " << x1 << endl;
        double num [] = {0.0, 1.0, 0.0};
        double den [] = {mMass, mB, mk};
        mFilter.initialize(mTime, mTimestep, num, den, -F1, -v1);
        mInt.initialize(mTime, mTimestep, -v1, -x1);
        //mFilter.update(0);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double Zx1  = mpP1->readNode(NodeMechanic::CHARIMP);
        double c1  = mpP1->readNode(NodeMechanic::WAVEVARIABLE);
        double Zx2  = mpP2->readNode(NodeMechanic::CHARIMP);
        double c2  = mpP2->readNode(NodeMechanic::WAVEVARIABLE);

        //Mass equations
        double num[3] = {0.0, 1.0, 0.0};
        double den[3] = {mMass, mB+Zx1+Zx2, mk};

        mFilter.setNumDen(num, den);
        double v2 = mFilter.value(c1-c2);
        double v1 = -v2;
        double x2 = mInt.value(v2);
        double x1 = -x2;
        double F1 = c1 + Zx1*v1;
        double F2 = c2 + Zx2*v2;

        //Write new values to nodes
        mpP1->writeNode(NodeMechanic::FORCE, F1);
        mpP2->writeNode(NodeMechanic::FORCE, F2);
        mpP1->writeNode(NodeMechanic::VELOCITY, v1);
        mpP2->writeNode(NodeMechanic::VELOCITY, v2);
        mpP1->writeNode(NodeMechanic::POSITION, x1);
        mpP2->writeNode(NodeMechanic::POSITION, x2);
        //Update Filter:
        //mFilter.update(c1-c2);
    }
};

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

