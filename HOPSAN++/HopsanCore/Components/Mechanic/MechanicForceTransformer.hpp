#ifndef MECHANICFORCETRANSFORMER_HPP_INCLUDED
#define MECHANICFORCETRANSFORMER_HPP_INCLUDED

#include "HopsanCore.h"

class MechanicForceTransformer : public ComponentC
{

private:
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running force transformer creator" << std::endl;
        return new MechanicForceTransformer("DefaultForceTransformerName");
    }

    MechanicForceTransformer(const string name,
                    const double timestep    = 0.001)
    : ComponentC(name, timestep)
    {
        //Set member attributes

		//Add ports to the component
        addPort("in", "NodeSignal", in);
        addPort("out", "NodeMechanic", out);

        //Register changable parameters to the HOPSAN++ core
    }


	void initialize()
    {
    }

    void simulateOneTimestep()
    {
        //Get the nodes
		Node* p1_ptr = mPorts[in].getNodePtr();
		Node* p2_ptr = mPorts[out].getNodePtr();

        //Get variable values from nodes
        double signal  = p1_ptr->getData(NodeSignal::VALUE);

        //Spring equations
        double c = signal;
        double Zc = 0.0;

        //Write new values to nodes
        p2_ptr->setData(NodeMechanic::WAVEVARIABLE, c);
        p2_ptr->setData(NodeMechanic::CHARIMP, Zc);
    }
};

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
