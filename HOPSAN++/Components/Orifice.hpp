#ifndef ORIFICE_HPP_INCLUDED
#define ORIFICE_HPP_INCLUDED

#include "Components.h"

class ComponentOrifice : Component
{

public:
    ComponentOrifice(const string name, const double timestep=0.001, const double kc=1.0e-11)
                    : Component(name, timestep)
    {
        //ComponentQ.__init__(self,  name=name,  timestep=timestep)

        mKc = kc;
        setNodeSpecifications({'p1':'NodeHydraulic', 'p2':'NodeHydraulic'})
    }

    void simulateOneTimestep(self)
    {
        #Input
        p1  = self.getNode('p1').getPressure()
        q1  = self.getNode('p1').getMassflow()
        c1  = self.getNode('p1').getWavevariable()
        Zc1 = self.getNode('p1').getCharimp()
        p2  = self.getNode('p2').getPressure()
        q2  = self.getNode('p2').getMassflow()
        c2  = self.getNode('p2').getWavevariable()
        Zc2 = self.getNode('p2').getCharimp()

        #Delay Line
        q2 = self.kc*(c1-c2)/(1+ self.kc*(Zc1+Zc2))
        q1 = -q2
        p1 = c1 + q1*Zc1
        p2 = c2 + q2*Zc2
    }


private:
    double mKc;





};


#endif // ORIFICE_HPP_INCLUDED
