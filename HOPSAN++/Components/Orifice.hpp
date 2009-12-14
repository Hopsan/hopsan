#ifndef ORIFICE_HPP_INCLUDED
#define ORIFICE_HPP_INCLUDED

#include "Components.h"
#include "Node.h"

class ComponentOrifice : ComponentQ
{

public:
    enum {P1, P2};

    ComponentOrifice(const string name, const double timestep=0.001, const double kc=1.0e-11)
                    : ComponentQ(name, timestep)
    {
        //ComponentQ.__init__(self,  name=name,  timestep=timestep)

        mKc = kc;
        //setNodeSpecifications({'p1':'NodeHydraulic', 'p2':'NodeHydraulic'})
        addPort(P1, Port("NodeHydraulic"));
        addPort(P2, Port("NodeHydraulic"));
    }

    void simulateOneTimestep()
    {
		//read from nodes
		Node* p1_ptr = mPorts[P1];
		Node* p2_ptr = mPorts[P2];

        double p1  = p1_ptr->getData(NodeHydraulic::PRESSURE);
        double q1  = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);
        double p2  = p2_ptr->getData(NodeHydraulic::PRESSURE);
        double q2  = p2_ptr->getData(NodeHydraulic::MASSFLOW);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);

        //Delay Line
        q2 = mKc*(c1-c2)/(1+ mKc*(Zc1+Zc2))
        q1 = -q2
        p1 = c1 + q1*Zc1
        p2 = c2 + q2*Zc2

        //Write to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, P1);
        p1_ptr->setData(NodeHydraulic::PRESSURE, Q1);
        p2_ptr->setData(NodeHydraulic::PRESSURE, P2);
        p2_ptr->setData(NodeHydraulic::PRESSURE, Q2);
    }


private:
    double mKc;

};


#endif // ORIFICE_HPP_INCLUDED
