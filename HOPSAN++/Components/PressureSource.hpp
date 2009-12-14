#ifndef PRESSURESOURCE_HPP_INCLUDED
#define PRESSURESOURCE_HPP_INCLUDED

class ComponentPressureSource : ComponentC
{

public:
    enum {P1, P2};

    ComponentPressureSource(const string name, const double timestep=0.001, const double pressure=1.0e5)
                    : ComponentQ(name, timestep)
    {
        mPressure = pressure;

        addPort(P1, Port("NodeHydraulic"));
    }

    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[P1];

        //delayed line
        double p1 = mPressure;
        double Zc = 0;

        //write to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, p1);
        p1_ptr->setData(NodeHydraulic::CHARIMP, Zc);
    }

private:
    double mPressure;

};

#endif // PRESSURESOURCE_HPP_INCLUDED
