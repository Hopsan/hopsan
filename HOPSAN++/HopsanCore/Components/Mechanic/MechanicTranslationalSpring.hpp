#ifndef MECHANICTRANSLATIONALSPRING_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRING_HPP_INCLUDED

#include "HopsanCore.h"

class MechanicTranslationalSpring : public ComponentC
{

private:
    double mk;
    double mTimestep;
    enum {P1, P2};

public:
    static Component *Creator()
    {
        std::cout << "running translational spring creator" << std::endl;
        return new MechanicTranslationalSpring("DefaultTranslationalSpringName");
    }

    MechanicTranslationalSpring(const string name,
                    const double springcoefficient = 100.0,
                    const double timestep    = 0.001)
    : ComponentC(name, timestep)
    {
        //Set member attributes
        mk   = springcoefficient;
        mTimestep = timestep;

		//Add ports to the component
        addPort("P1", "NodeMechanic", P1);
        addPort("P2", "NodeMechanic", P2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("k", "Spring Coefficient", "[N/m]",  mk);
    }


	void initialize()
    {

    }

    void simulateOneTimestep()
    {
        //Get the nodes
		Node* p1_ptr = mPortPtrs[P1]->getNodePtr();
		Node* p2_ptr = mPortPtrs[P2]->getNodePtr();

        //Get variable values from nodes
        double v1  = p1_ptr->getData(NodeMechanic::VELOCITY);
        double v2  = p2_ptr->getData(NodeMechanic::VELOCITY);
        double lastc1  = p1_ptr->getData(NodeMechanic::WAVEVARIABLE);
        double lastc2  = p2_ptr->getData(NodeMechanic::WAVEVARIABLE);

        //Spring equations
        double Zc = mk * mTimestep;
        double c1 = lastc2 + 2.0*Zc*v2;
        double c2 = lastc1 + 2.0*Zc*v1;

        //Write new values to nodes
        p1_ptr->setData(NodeMechanic::WAVEVARIABLE, c1);
        p2_ptr->setData(NodeMechanic::WAVEVARIABLE, c2);
        p1_ptr->setData(NodeMechanic::CHARIMP, Zc);
        p2_ptr->setData(NodeMechanic::CHARIMP, Zc);
    }
};

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


