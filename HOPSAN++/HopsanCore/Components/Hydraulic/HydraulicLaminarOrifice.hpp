#ifndef HYDRAULICLAMINARORIFICE_HPP_INCLUDED
#define HYDRAULICLAMINARORIFICE_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicLaminarOrifice : public ComponentQ
{
private:
    double mKc;
    enum {P1, P2};

public:
    static Component *Creator()
    {
        std::cout << "running orifice creator" << std::endl;
        return new HydraulicLaminarOrifice("DefaultOrificeName");
    }

    HydraulicLaminarOrifice(const string name,
                             const double kc       = 1.0e-11,
                             const double timestep = 0.001)
        : ComponentQ(name, timestep)
    {
        mKc = kc;

        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);

        registerParameter("Kc", "Pressure-Flow Coefficient", "[m^5/Ns]", mKc);
    }


    void initialize()
    {
        //Nothing to initialize
    }


    void simulateOneTimestep()
    {
        //Get the nodes
        Node* p1_ptr = mPortPtrs[P1]->getNodePtr();
        Node* p2_ptr = mPortPtrs[P2]->getNodePtr();

        //Get variable values from nodes
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);

        //Orifice equations
        double q2 = mKc*(c1-c2)/(1.0+mKc*(Zc1+Zc2));
        double q1 = -q2;
        double p1 = c1 + q1*Zc1;
        double p2 = c2 + q2*Zc2;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, p1);
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q1);
        p2_ptr->setData(NodeHydraulic::PRESSURE, p2);
        p2_ptr->setData(NodeHydraulic::MASSFLOW, q2);
    }
};

#endif // HYDRAULICLAMINARORIFICE_HPP_INCLUDED
