#ifndef PRESSURESOURCE_HPP_INCLUDED
#define PRESSURESOURCE_HPP_INCLUDED

class ComponentPressureSource : public ComponentC
{

public:
    enum {P1};

    ComponentPressureSource(const string name, const double pressure=1.0e5, const double timestep=0.001)
                    :ComponentC(name, timestep)
    {
        mPressure = pressure;

        addPort(P1, Port("NodeHydraulic"));
    }

    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[P1].getNodePtr();

        //delayed line
        double p1 = mPressure;
        double zc = 0;

        //write to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, p1);
        p1_ptr->setData(NodeHydraulic::CHARIMP, zc);
    }

private:
    double mPressure;

};

#endif // PRESSURESOURCE_HPP_INCLUDED
