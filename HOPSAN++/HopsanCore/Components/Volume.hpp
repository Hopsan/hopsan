#ifndef VOLUME_HPP_INCLUDED
#define VOLUME_HPP_INCLUDED

#include "Component.h"
#include "Nodes/Nodes.h"

class ComponentVolume : public ComponentC
{

private:
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mAlpha;
    double mVolume;
    double mBulkmodulus;
    enum {P1, P2};

public:
    ComponentVolume(const string name,
                    const double volume      = 1.0e-3,
                    const double bulkmudulus = 1.0e9,
                    const double alpha       = 0.0,
                    const double timestep    = 0.001)
    : ComponentC(name, timestep)
    {
        //Set member attributes
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mBulkmodulus   = bulkmudulus;
        mVolume        = volume;
        mZc            = mBulkmodulus/mVolume*mTimestep;
		mAlpha         = alpha;

		//Add ports to the component
        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("V", "Volym", "m^3",            mVolume);
        registerParameter("Be", "Kompressionsmodul", "Pa", mBulkmodulus);
        registerParameter("a", "Lågpasskoeficient", "-",  mAlpha);
        registerParameter("Zc", "Kappasitans", "Ns/m^5",   mZc);
    }


	void initialize()
    {
        mZc = mBulkmodulus/mVolume*mTimestep; //Need to be updated at simulation start since it is volume and bulk that are set.

        //Read from nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        //Write to nodes
        p1_ptr->setData(NodeHydraulic::MASSFLOW,     mStartFlow);
        p1_ptr->setData(NodeHydraulic::PRESSURE,     mStartPressure);
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        p1_ptr->setData(NodeHydraulic::CHARIMP,      mZc);
        p2_ptr->setData(NodeHydraulic::MASSFLOW,     mStartFlow);
        p2_ptr->setData(NodeHydraulic::PRESSURE,     mStartPressure);
        p2_ptr->setData(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        p2_ptr->setData(NodeHydraulic::CHARIMP,      mZc);
	}


    void simulateOneTimestep()
    {
        //Get the nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        //Get variable values from nodes
        double q1  = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double q2  = p2_ptr->getData(NodeHydraulic::MASSFLOW);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);

        //Orifice equations
        double c10 = c2 + 2.0*mZc * q2;
        double c20 = c1 + 2.0*mZc * q1;
        c1 = mAlpha*c1 + (1.0-mAlpha)*c10;
        c2 = mAlpha*c2 + (1.0-mAlpha)*c20;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, c1);
        p2_ptr->setData(NodeHydraulic::WAVEVARIABLE, c2);
        p1_ptr->setData(NodeHydraulic::CHARIMP,      mZc);
        p2_ptr->setData(NodeHydraulic::CHARIMP,      mZc);
    }
};

#endif // VOLUME_HPP_INCLUDED
