#ifndef HYDRAULICLAMINARORIFICE_HPP_INCLUDED
#define HYDRAULICLAMINARORIFICE_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicLaminarOrifice : public ComponentQ
{
private:
    double mKc;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        std::cout << "running orifice creator" << std::endl;
        return new HydraulicLaminarOrifice("DefaultLaminarOrificeName");
    }

    HydraulicLaminarOrifice(const string name      = "DefaultOrificeName",
                             const double kc       = 1.0e-11,
                             const double timestep = 0.001)
        : ComponentQ(name, timestep)
    {
        mTypeName = "HydraulicLaminarOrifice";
        mKc = kc;

        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        registerParameter("Kc", "Pressure-Flow Coefficient", "[m^5/Ns]", mKc);
    }


    void initialize()
    {
        //Nothing to initialize
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

        //Orifice equations
        double q2 = mKc*(c1-c2)/(1.0+mKc*(Zc1+Zc2));
        double q1 = -q2;
        double p1 = c1 + q1*Zc1;
        double p2 = c2 + q2*Zc2;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
        mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);
    }
};

#endif // HYDRAULICLAMINARORIFICE_HPP_INCLUDED
