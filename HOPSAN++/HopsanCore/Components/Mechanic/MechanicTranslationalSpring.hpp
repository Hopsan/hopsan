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
        mTypeName = "MechanicTranslationalSpring";
        mk   = springcoefficient;
        mTimestep = timestep;

		//Add ports to the component
        addPowerPort("P1", "NodeMechanic", P1);
        addPowerPort("P2", "NodeMechanic", P2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("k", "Spring Coefficient", "[N/m]",  mk);
    }


	void initialize()
    {

    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double v1  = mPortPtrs[P1]->readNode(NodeMechanic::VELOCITY);
        double v2  = mPortPtrs[P2]->readNode(NodeMechanic::VELOCITY);
        double lastc1  = mPortPtrs[P1]->readNode(NodeMechanic::WAVEVARIABLE);
        double lastc2  = mPortPtrs[P2]->readNode(NodeMechanic::WAVEVARIABLE);

        //Spring equations
        double Zc = mk * mTimestep;
        double c1 = lastc2 + 2.0*Zc*v2;
        double c2 = lastc1 + 2.0*Zc*v1;

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeMechanic::WAVEVARIABLE, c1);
        mPortPtrs[P2]->writeNode(NodeMechanic::WAVEVARIABLE, c2);
        mPortPtrs[P1]->writeNode(NodeMechanic::CHARIMP, Zc);
        mPortPtrs[P2]->writeNode(NodeMechanic::CHARIMP, Zc);
    }
};

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


