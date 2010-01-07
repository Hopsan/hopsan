#ifndef TURBORIFICE_HPP_INCLUDED
#define TURBORIFICE_HPP_INCLUDED

#include "Component.h"
#include "Nodes/Nodes.h"
#include "math.h"

class ComponentTurbOrifice : public ComponentQ
{
private:
    double mCq;
    double mA;
    enum {P1, P2};
    double sign(double x)
    {
        return x/fabs(x);
    }
    double sigsqrl(double x,
            const double x0=0.001,
            const double r=4)
    {
        return sign(x)*pow(pow(fabs(x),r)/(pow(fabs(x),(r/2))+pow(x0,r)),(1/r));
    }
public:
    ComponentTurbOrifice(const string name,
                     const double Cq       = 0.67,
                     const double A=0.00001,
                     const double timestep = 0.001)
    : ComponentQ(name, timestep)
    {
        mCq = Cq;
        mA=A;

        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);
        //addMultiPort("P", "NodeHydraulic", 2);

        registerParameter("Cq", "Flödeskoeff.", "-", mCq);
        registerParameter("A", "Area", "m^3", mA);
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
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);

        //Orifice equations
        double Kc = mCq*mA*sqrt(2.0/890.0);
        double q2 = 0.5*pow(Kc,2.0)*(Zc1+Zc2)+sigsqrl(pow(Kc,2.0)*(c1-c2)+0.25*pow(Kc,4.0)*pow(Zc1+Zc2,2.0));
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

#endif // ORIFICE_HPP_INCLUDED
