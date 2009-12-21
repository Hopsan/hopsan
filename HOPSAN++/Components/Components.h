#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <string>
#include "Nodes.h"
using namespace std;

class Component; //forward declaration
class Port
{
    friend class Component;
public:
    Port();
    Port(string portname, string node_type);
    string &getNodeType();
    Node &getNode();
    Node* getNodePtr();
    void setNode(Node* node_ptr);

private:
    string mPortName, mNodeType;
    Node* mpNode;
    Component* mpComponent;
};

class ComponentSystem;  //forward declaration
class Component
{
public:
    Component(string name, double timestep=0.001);
    virtual void initialize(); ///TODO: Default values are hard set
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

    void setSystemparent(ComponentSystem &rComponentSystem); ///TODO: this should not be public
    Port &getPortById(const size_t port_idx); ///TODO: this should not be public
    Port &getPort(const string portname);

protected:
    //void addPort(const size_t port_idx, Port port);
    void addPort(const string portname, const string nodetype, const int id=-1);
    //void addMultiPort(const string portname, const string nodetype, const size_t nports, const size_t startctr=0);
    ComponentSystem &getSystemparent();

    string mType;
    vector<Port> mPorts;

    double mTimestep;

    bool mIsComponentC;
    bool mIsComponentQ;
    bool mIsComponentSystem;
    bool mIsComponentSignal;

private:
    string mName;

    ComponentSystem* mpSystemparent;

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
    void preAllocateLogSpace(const double startT, const double stopT);
    void logAllNodes(const double time);
    void connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2);
    void simulate(const double startT, const double stopT);

    protected:
    vector<Component*> mSubComponentPtrs; //Problems with inheritance and casting?
    vector<Node*> mSubNodePtrs;
    vector<Component*> mComponentQptrs;
    vector<Component*> mComponentCptrs;
    //vector<ComponentSignal*>

    private:



};

#endif // COMPONENTS_H_INCLUDED
