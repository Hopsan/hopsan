#ifndef MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED

#include "HopsanCore.h"

class MechanicVelocityTransformer : public ComponentQ
{

private:
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running velocity transformer creator" << std::endl;
        return new MechanicVelocityTransformer("DefaultVelocityTransformerName");
    }

    MechanicVelocityTransformer(const string name,
                    const double timestep    = 0.001)
    : ComponentQ(name, timestep)
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
		Node* p1_ptr = mPortPtrs[in]->getNodePtr();
		Node* p2_ptr = mPortPtrs[out]->getNodePtr();

        //Get variable values from nodes
        double signal  = p1_ptr->getData(NodeSignal::VALUE);
        double c = p2_ptr->getData(NodeMechanic::WAVEVARIABLE);
        double Zc = p2_ptr->getData(NodeMechanic::CHARIMP);


        //Spring equations
        double v = signal;
        double F = c + Zc*v;

        //Write new values to nodes
        p2_ptr->setData(NodeMechanic::VELOCITY, v);
        p2_ptr->setData(NodeMechanic::FORCE, F);
    }
};

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED




