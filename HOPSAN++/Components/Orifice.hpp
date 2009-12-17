#ifndef ORIFICE_HPP_INCLUDED
#define ORIFICE_HPP_INCLUDED

#include "Components.h"
#include "Nodes.h"

class ComponentOrifice : public ComponentQ
{

public:
    enum {P1, P2};

    ComponentOrifice(const string name, const double kc=1.0e-11, const double timestep=0.001)
                    : ComponentQ(name, timestep)
    {
        mKc = kc;
        addPort(P1, Port("NodeHydraulic"));
        addPort(P2, Port("NodeHydraulic"));
    }

    void simulateOneTimestep()
    {
		//read from nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);

        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);

        //Delay Line
        double q2 = mKc*(c1-c2)/(1+ mKc*(Zc1+Zc2));
        double q1 = -q2;
        double p1 = c1 + q1*Zc1;
        double p2 = c2 + q2*Zc2;

        //Write to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, p1);
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q1);
        p2_ptr->setData(NodeHydraulic::PRESSURE, p2);
        p2_ptr->setData(NodeHydraulic::MASSFLOW, q2);
    }


private:
    double mKc;

};


#endif // ORIFICE_HPP_INCLUDED
