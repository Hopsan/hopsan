#ifndef EXTERNALPRESSURESOURCEQ_HPP_INCLUDED
#define EXTERNALPRESSURESOURCEQ_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class ComponentExternalPressureSourceQ : public ComponentQ
{
private:
    double mStartPressure;
    double mStartFlow;
    double mPressure;
    enum {P1};

public:
    static Component *Creator()
    {
        std::cout << "running pressureSourceQ creator" << std::endl;
        return new ComponentExternalPressureSourceQ("DefaultPressureSourceQName");
    }

    ComponentExternalPressureSourceQ(const string name,
                                      const double pressure = 1.0e5,
                                      const double timestep = 0.001)
	: ComponentQ(name, timestep)
    {
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mPressure      = pressure;

        addPort("P1", "NodeHydraulic", P1);

        registerParameter("P", "Tryck", "Pa", mPressure);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get the nodes
   		Node* p1_ptr = mPorts[P1].getNodePtr();

        //Get variable values from nodes
		double c  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc = p1_ptr->getData(NodeHydraulic::CHARIMP);

        //Flow source equations
        double q = (mPressure - c)/Zc;
		double p = mPressure;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q);
        p1_ptr->setData(NodeHydraulic::PRESSURE, p);
    }
};

#endif // EXTERNALPRESSURESOURCEQ_HPP_INCLUDED
