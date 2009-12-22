#ifndef PRESSURESOURCE_HPP_INCLUDED
#define PRESSURESOURCE_HPP_INCLUDED

class ComponentPressureSource : public ComponentC
{

private:
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mPressure;
    enum {P1};

public:
    ComponentPressureSource(const string name,
                            const double pressure = 1.0e5,
                            const double timestep = 0.001)
    : ComponentC(name, timestep)
    {
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mPressure      = pressure;
        mZc            = 0.0;

        addPort("P1", "NodeHydraulic", P1);

        registerParameter("Tryck", "Pa", mPressure);
    }


    void initialize()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[P1].getNodePtr();

        //write to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, mStartPressure);
        p1_ptr->setData(NodeHydraulic::MASSFLOW, mStartFlow);
	}


    void simulateOneTimestep()
    {
        //Get the nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();

        //Pressure source equation
        double p = mPressure;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, p);
        p1_ptr->setData(NodeHydraulic::CHARIMP,      mZc);
    }
};

#endif // PRESSURESOURCE_HPP_INCLUDED
