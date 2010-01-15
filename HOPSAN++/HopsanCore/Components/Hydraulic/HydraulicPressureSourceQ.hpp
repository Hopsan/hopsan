#ifndef HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicPressureSourceQ : public ComponentQ
{
private:
    double mStartPressure;
    double mStartFlow;
    double mPressure;
    enum {P1,in};

public:
    static Component *Creator()
    {
        std::cout << "running pressureSourceQ creator" << std::endl;
        return new HydraulicPressureSourceQ("DefaultPressureSourceQName");
    }

    HydraulicPressureSourceQ(const string name,
                             const double pressure = 1.0e5,
                             const double timestep = 0.001)
	: ComponentQ(name, timestep)
    {
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mPressure      = pressure;

        addPort("P1", "NodeHydraulic", P1);
        addPort("in", "NodeSignal", in);

        registerParameter("P", "Tryck", "Pa", mPressure);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get the nodes
   		Node* p1_ptr = mPortPtrs[P1]->getNodePtr();
        Node* p2_ptr = mPortPtrs[in]->getNodePtr();

        //Get variable values from nodes
		double c  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc = p1_ptr->getData(NodeHydraulic::CHARIMP);

        //Flow source equations
        double q,p;

        if (mPortPtrs[in]->isConnected())
        {
            q = (p2_ptr->getData(NodeSignal::VALUE) - c)/Zc;
            p = p2_ptr->getData(NodeSignal::VALUE);         //We have a signal!
        }
        else
        {
            q = (mPressure - c)/Zc;
            p = mPressure;                                  //No signal, use internal parameter
        }

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q);
        p1_ptr->setData(NodeHydraulic::PRESSURE, p);
    }
};

#endif // HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
