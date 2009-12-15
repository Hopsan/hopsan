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
    void setNode(Node* node_ptr);

private:
    string mNodeType;
    Node* mpNode;

};


class Component
{
public:
    Component(string name, double timestep=0.001);
    virtual void simulateOneTimestep();
    void simulate(const double startT, const double Ts);

    void setName(string &rName);
    string &getName();

    string &getType();

    void setTimestep(const double timestep);
    double getTimestep();

    bool isComponentC();
    bool isComponentQ();
    bool isComponentSystem();
    bool isComponentSignal();

    void setSystemparent(Component &rComponent); ///TODO: this should not be public
    Port &getPort(const size_t port_idx); ///TODO: this should not be public

protected:
    void addPort(const size_t port_idx, Port port);
    Component &getSystemparent();

    string mType;
    vector<Port> mPorts;

    double mTimestep;

    bool mIsComponentC;
    bool mIsComponentQ;
    bool mIsComponentSystem;
    bool mIsComponentSignal;

private:
    string mName;

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


class ComponentSystem :public Component
{
    public:
    ComponentSystem(string name, double timestep=0.001);
    void addComponents(vector<Component*> components);
    void addComponent(Component &rComponent);
    void addSubNode(Node* node_ptr);
    void logAllNodes(const double time);
    void connect(Component &rComponent1, size_t portname1, Component &rComponent2, size_t portname2);
    void simulate(const double startT, const double Ts);

    protected:
    vector<Component*> mpSubComponents; //Problems with inheritance and casting?
    vector<Node*> mpSubNodes;
    vector<Component*> mpComponentsQ;
    vector<Component*> mpComponentsC;
    //vector<ComponentSignal*>

    private:



};

#endif // COMPONENTS_H_INCLUDED
