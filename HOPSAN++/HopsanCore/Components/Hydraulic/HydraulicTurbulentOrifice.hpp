#ifndef HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
#define HYDRAULICTURBULENTORIFICE_HPP_INCLUDED

#include "Component.h"
#include "Nodes/Nodes.h"
#include "math.h"
#include "CoreUtilities/TurbulentFlowFunction.h"

class HydraulicTurbulentOrifice : public ComponentQ
{
private:
    double mCq;
    double mA;
    double mKc;
    TurbulentFlowFunction qTurb;
    enum {P1, P2};
//    double sign(double x)
//    {
//        return x/fabs(x);
//    }
//    double sigsqrl(double x,
//            const double x0=0.001,
//            const double r=4)
//    {
//        return sign(x)*pow(pow(fabs(x),r)/(pow(fabs(x),(r/2))+pow(x0,r)),(1/r));
//    }
public:
    static Component *Creator()
    {
        std::cout << "running orifice creator" << std::endl;
        return new HydraulicLaminarOrifice("DefaultTurbulentOrificeName");
    }

    HydraulicTurbulentOrifice(const string name,
                     const double Cq       = 0.67,
                     const double A        = 0.00001,
                     const double timestep = 0.001)
    : ComponentQ(name, timestep)
    {
        mTypeName = "HydraulicTurbulentOrificeName";
        mCq = Cq;
        mA = A;
        mKc = mCq*mA*sqrt(2.0/890.0);

        addPowerPort("P1", "NodeHydraulic", P1);
        addPowerPort("P2", "NodeHydraulic", P2);
        //addMultiPort("P", "NodeHydraulic", 2);

        registerParameter("Cq", "Flow coefficient", "[-]", mCq);
        registerParameter("A", "Area", "m^3", mA);
    }


    void initialize()
    {
		qTurb.setFlowCoefficient(mKc);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1 = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mPortPtrs[P1]->readNode(NodeHydraulic::CHARIMP);
        double c2 = mPortPtrs[P2]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mPortPtrs[P2]->readNode(NodeHydraulic::CHARIMP);

        //Orifice equations
        double q2 = qTurb.getFlow(c1,c2,Zc1,Zc2);
        double q1 = -q2;
        double p1 = c1 + q1*Zc1;
        double p2 = c2 + q2*Zc2;

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE, p1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW, q1);
        mPortPtrs[P2]->writeNode(NodeHydraulic::PRESSURE, p2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::MASSFLOW, q2);
    }
};

#endif // HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
