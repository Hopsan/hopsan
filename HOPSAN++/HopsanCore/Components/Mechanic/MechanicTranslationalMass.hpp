//$Id$

#ifndef MECHANICTRANSLATIONALMASS_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASS_HPP_INCLUDED

#include "HopsanCore.h"
#include "CoreUtilities/TransferFunction.h"

class MechanicTranslationalMass : public ComponentQ
{

private:
    double mMass;
    double mB;
    double mk;
    SecondOrderFilter mFilter;
    Integrator mInt;
    enum {P1, P2};

public:
    static Component *Creator()
    {
        std::cout << "running translational mass creator" << std::endl;
        return new MechanicTranslationalMass("DefaultTranslationalMassName");
    }

    MechanicTranslationalMass(const string name,
                    const double mass      = 1.0,
                    const double viscousfriction = 10,
                    const double springcoefficient = 0.0,
                    const double timestep    = 0.001)
    : ComponentQ(name, timestep)
    {
        //Set member attributes
        mMass = mass;
        mB    = viscousfriction;
        mk   = springcoefficient;
        mTimestep = timestep;

		//Add ports to the component
        addPowerPort("P1", "NodeMechanic", P1);
        addPowerPort("P2", "NodeMechanic", P2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("Mass", "Mass", "[kg]",            mMass);
        registerParameter("B", "Viscous Friction", "[Ns/m]", mB);
        registerParameter("k", "Spring Coefficient", "[N/m]",  mk);
    }

	void initialize()
    {
//        mFilter.initialize(0.0,0.0, mTime);
        double num [] = {0.0, 1.0, 0.0};
        double den [] = {mMass, mB, mk};
        mFilter.initialize(mTime, mTimestep, num, den);
        mInt.initialize(mTime, mTimestep, 0.0, 0.0);
//        mFilter.update(0);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double Zx1  = mPortPtrs[P1]->readNode(NodeMechanic::CHARIMP);
        double c1  = mPortPtrs[P1]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zx2  = mPortPtrs[P2]->readNode(NodeMechanic::CHARIMP);
        double c2  = mPortPtrs[P2]->readNode(NodeMechanic::WAVEVARIABLE);

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
        mPortPtrs[P1]->writeNode(NodeMechanic::FORCE, F1);
        mPortPtrs[P2]->writeNode(NodeMechanic::FORCE, F2);
        mPortPtrs[P1]->writeNode(NodeMechanic::VELOCITY, v1);
        mPortPtrs[P2]->writeNode(NodeMechanic::VELOCITY, v2);
        mPortPtrs[P1]->writeNode(NodeMechanic::POSITION, x1);
        mPortPtrs[P2]->writeNode(NodeMechanic::POSITION, x2);
        //Update Filter:
        //mFilter.update(c1-c2);
    }
};

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

