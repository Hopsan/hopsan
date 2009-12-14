#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <string>
#include "Nodes.h"
using namespace std;

class Port
{
public:
    Port();
    Port(string node_type);
    string &getNodeType();
    Node &getNode();
    Node* getNodePtr();

private:
    string mNodeType;
    Node* mpNode;

};


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

protected:
    void addPort(const size_t port_idx, Port port);
    string mType;
    vector<Port> mPorts;

private:
    string mName;
    double mTimestep;
    Component* mpSystemparent;





};


class ComponentC :public Component
{
public:
    ComponentC(string name, double timestep=0.001);


};


class ComponentQ :public Component
{
public:
    ComponentQ(string name, double timestep=0.001);


};



#endif // COMPONENTS_H_INCLUDED
