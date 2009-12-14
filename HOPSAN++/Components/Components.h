#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <string>
using namespace std;

class Component
{
public:
    Component(string name, double timestep=0.001);
    virtual void simulateOneTimestep()=0;
    void simulate(const double startT, const double Ts);

    void setName(string &rName);
    string &getName();

    void setTimestep(const double timestep);
    double getTimestep();


private:
    string mName;
    double mTimestep;


};


#endif // COMPONENTS_H_INCLUDED
