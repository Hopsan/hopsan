#ifndef HYDRAULICPRESSURESOURCE_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCE_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicPressureSource : public ComponentC
{
private:
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mPressure;
    enum {P1,in};

public:
    static Component *Creator()
    {
        std::cout << "running pressuresource creator" << std::endl;
        return new HydraulicPressureSource("DefaultPressureSourceName");
    }

    HydraulicPressureSource(const string name,
                                    const double pressure       = 1.0e5,
                                    const double timestep       = 0.001)
        : ComponentC(name, timestep)
    {
        mStartPressure  = 0.0;
        mStartFlow      = 0.0;
        mPressure       = pressure;
        mZc             = 0.0;

        addPort("P1", "NodeHydraulic", P1);
        addPort("in", "NodeSignal", in);

        registerParameter("P", "Pressure", "Pa", mPressure);
    }


    void initialize()
    {
        //read fron nodes
   		Node* p1_ptr = mPortPtrs[P1]->getNodePtr();

        //write to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, mStartPressure);
        p1_ptr->setData(NodeHydraulic::MASSFLOW, mStartFlow);

    }


    void simulateOneTimestep()
    {
        //Get the nodes
        Node* p1_ptr = mPortPtrs[P1]->getNodePtr();
        Node* p2_ptr = mPortPtrs[in]->getNodePtr();

        //Pressure source equation
        double p;
        if (mPortPtrs[in]->isConnected())
        {
            p = p2_ptr->getData(NodeSignal::VALUE);         //We have a signal!
        }
        else
        {
            p = mPressure;                                  //No signal, use internal parameter
        }

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, p);
        p1_ptr->setData(NodeHydraulic::CHARIMP, mZc);
    }
};

#endif // HYDRAULICPRESSURESOURCE_HPP_INCLUDED
