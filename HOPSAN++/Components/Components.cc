#include "Components.h"

//Constructor
Component::Component(string name, double timestep=0.001)
{
    mName = name;
    //this->mName = name;
    mTimestep = timestep;
}

void Component::simulate(const double startT, const double Ts)
{
//TODO: adjust self.timestep or simulation depending on Ts from system above (self.timestep should be multipla of Ts)
    double stopT = startT+Ts;
    double time = startT;
    while (time < stopT)
    {
        simulateOneTimestep();
        time += mTimestep;
    }
}
