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
    TransferFunction Filter;
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
        addPort("P1", "NodeMechanic", P1);
        addPort("P2", "NodeMechanic", P2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("Mass", "Mass", "[kg]",            mMass);
        registerParameter("B", "Viscous Friction", "[Ns/m]", mB);
        registerParameter("k", "Spring Coefficient", "[N/m]",  mk);
    }


	void initialize()
    {
    //Nothing to initialize
    }

    void simulateOneTimestep()
    {
        //Get the nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        //Get variable values from nodes
        double Zx1  = p1_ptr->getData(NodeMechanic::CHARIMP);
        double c1  = p1_ptr->getData(NodeMechanic::WAVEVARIABLE);
        double Zx2  = p2_ptr->getData(NodeMechanic::CHARIMP);
        double c2  = p2_ptr->getData(NodeMechanic::WAVEVARIABLE);

        //Mass equations
        double num [] = {0.0, 1.0, 0.0};
        double den [] = {mk, mB+Zx1+Zx2, mMass};
        Filter.setCoefficients(num, den, mTimestep);
        double v2 = Filter.filter(c1-c2);
        double v1 = -v2;
        double F1 = c1 + Zx1*v1;
        double F2 = c2 + Zx2*v2;

        //Write new values to nodes
        p1_ptr->setData(NodeMechanic::FORCE, F1);
        p2_ptr->setData(NodeMechanic::FORCE, F2);
        p1_ptr->setData(NodeMechanic::VELOCITY, v1);
        p2_ptr->setData(NodeMechanic::VELOCITY, v2);
    }
};

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

