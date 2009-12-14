#ifndef ORIFICE_HPP_INCLUDED
#define ORIFICE_HPP_INCLUDED

#include "Components.h"
#include "Node.h"

class ComponentOrifice : ComponentQ
{

public:
    enum {p1, p2};

    ComponentOrifice(const string name, const double timestep=0.001, const double kc=1.0e-11)
                    : ComponentQ(name, timestep)
    {
        //ComponentQ.__init__(self,  name=name,  timestep=timestep)

        mKc = kc;
        //setNodeSpecifications({'p1':'NodeHydraulic', 'p2':'NodeHydraulic'})
        addPort(p1, Port("NodeHydraulic"));
        addPort(p2, Port("NodeHydraulic"));
    }

    void simulateOneTimestep(self)
    {
		#read from nodes
		Node* p1_ptr = mPorts[p1];
		Node* p2_ptr = mPorts[p2];

        double p1  = p1_ptr->getData(NodeHydraulic::PRESSURE);
        double q1  = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);
        double p2  = p2_ptr->getData(NodeHydraulic::PRESSURE);
        double q2  = p2_ptr->getData(NodeHydraulic::MASSFLOW);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);

        #Delay Line
        q2 = mKc*(c1-c2)/(1+ mKc*(Zc1+Zc2))
        q1 = -q2
        p1 = c1 + q1*Zc1
        p2 = c2 + q2*Zc2

        #Write to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, p1);
        p1_ptr->setData(NodeHydraulic::PRESSURE, q1);
        p2_ptr->setData(NodeHydraulic::PRESSURE, p2);
        p2_ptr->setData(NodeHydraulic::PRESSURE, q2);
    }


private:
    double mKc;

};


#endif // ORIFICE_HPP_INCLUDED
