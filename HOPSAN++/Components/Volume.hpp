#ifndef VOLUME_HPP_INCLUDED
#define VOLUME_HPP_INCLUDED

#include "Components.h"
#include "Nodes.h"

class ComponentVolume : public ComponentC
{
private:
    double mAlpha;
    double mZc; ///TODO: Should be only in node.
    enum {P1, P2};

public:
    ComponentVolume(const string name, const double volume=1.0e-3,
                    const double bulkmudulus=1.0e9, const double alpha=0,
                    const double timestep=0.001)
                    : ComponentC(name, timestep)
    {
        mZc = bulkmudulus/volume*timestep;
        mAlpha = alpha;
        //setNodeSpecifications({'p1':'NodeHydraulic', 'p2':'NodeHydraulic'})
        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);
    }

    void simulateOneTimestep()
    {
		//read from nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        double q1  = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double q2  = p2_ptr->getData(NodeHydraulic::MASSFLOW);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);

        //Delay Line
        double c10 = c2 + 2*mZc * q2;
        double c20 = c1 + 2*mZc * q1;
        c1 = mAlpha*c1 + (1-mAlpha)*c10;
        c2 = mAlpha*c2 + (1-mAlpha)*c20;

        //Write to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, c1);
        p2_ptr->setData(NodeHydraulic::WAVEVARIABLE, c2);
        p1_ptr->setData(NodeHydraulic::CHARIMP, mZc);
        p2_ptr->setData(NodeHydraulic::CHARIMP, mZc);
    }
};


#endif // VOLUME_HPP_INCLUDED
