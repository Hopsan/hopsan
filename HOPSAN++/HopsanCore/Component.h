//!
//! @file   Component.h
//! @author <FluMeS>
//! @date   2009-12-20
//!
//! @brief Contains Component base classes as well as Component Parameter class
//!
//$Id$

#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include "Node.h"
#include "CoreUtilities/ClassFactory.h"
#include "win32dll.h"
#include <string>

using namespace std;
Port* CreatePort(const string &rPortType);

class DLLIMPORTEXPORT CompParameter
{
    friend class Component;

private:
    CompParameter(const string name, const string description, const string unit, double &rValue); //should maybe be a description field as well

    double getValue();
    void setValue(const double value);

    ///TODO: getting strings can (probably) be speed up by returning const references instead of copying strings
    string getName();
    string getDesc();
    string getUnit();

    string mName;
    string mDescription;
    string mUnit;
    double* mpValue;
};

class ComponentSystem; //Forward declaration

class DLLIMPORTEXPORT Component
{
    friend class ComponentSystem;

public:
    void simulate(const double startT, const double Ts);

    void setName(string name);
    const string &getName();

    const string &getType();

    void listParametersConsole();
    double getParameter(const string name);
    void setParameter(const string name, const double value);

    ComponentSystem &getSystemparent();

    void setTimestep(const double timestep); ///TODO: Should it be possible to set timestep of a component? Should only be possible for a Systemcomponent
    double getTimestep();

    bool isComponentC();
    bool isComponentQ();
    bool isComponentSystem();
    bool isComponentSignal();

    Port &getPort(const string portname);

protected:
    Component(string name, double timestep=0.001);
    virtual ~Component(){};
    virtual void initialize(); ///TODO: Default values are hard set
    virtual void simulateOneTimestep();
    void registerParameter(const string name, const string description, const string unit, double &rValue);

    bool getPort(const string portname, Port* &prPort);
    Port &getPortById(const size_t port_idx);
    //void addPort(const size_t port_idx, Port port);
//    void addPort(const string portname, const string nodetype, const int id=-1); ///TODO: Should be deleted after subsystem is changed e.g. addInnerPort and so on
    void addPort(const string portname, const string porttype, const NodeTypeT nodetype, const int id=-1);
    void addPowerPort(const string portname, const string nodetype, const int id=-1);
    void addReadPort(const string portname, const string nodetype, const int id=-1);
    void addWritePort(const string portname, const string nodetype, const int id=-1);
    //void addMultiPort(const string portname, const string nodetype, const size_t nports, const size_t startctr=0);

    string mType;
    double mTimestep;
    double mTime;

    vector<Port*> mPortPtrs, mInnerPortPtrs;

    bool mIsComponentC;
    bool mIsComponentQ;
    bool mIsComponentSystem;
    bool mIsComponentSignal;

private:
    void setSystemparent(ComponentSystem &rComponentSystem);

    void addInnerPortSetNode(const string portname, const string porttype, Node* pNode);

    void addSubNode(Node* node_ptr);

    vector<Node*> mSubNodePtrs;

    vector<CompParameter> mParameters;

    string mName;
    ComponentSystem* mpSystemparent;
};


class DLLIMPORTEXPORT ComponentSignal :public Component
{
protected:
    ComponentSignal(string name, double timestep=0.001);
};


class DLLIMPORTEXPORT ComponentC :public Component
{
protected:
    ComponentC(string name, double timestep=0.001);
};


class DLLIMPORTEXPORT ComponentQ :public Component
{
protected:
    ComponentQ(string name, double timestep=0.001);
};


class DLLIMPORTEXPORT ComponentSystem :public Component
{
public:
    ComponentSystem(string name, double timestep=0.001);
    void addComponents(vector<Component*> components);
    void addComponent(Component &rComponent);
    void connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2);
    void connect(Port &rPort1, Port &rPort2);
    void simulate(const double startT, const double stopT);
    void preAllocateLogSpace(const double startT, const double stopT);
    void logAllNodes(const double time);

private:
    //void addInnerPortSetNode(const string portname, const string porttype, Node* pNode);
    bool connectionOK(Node *pNode, Port *pPort1, Port *pPort2);

    vector<Component*> mSubComponentPtrs; //Problems with inheritance and casting?
    vector<Component*> mComponentSignalptrs;
    vector<Component*> mComponentQptrs;
    vector<Component*> mComponentCptrs;

    NodeFactory mpNodeFactory;
};

typedef ClassFactory<string, Component> ComponentFactory;
extern ComponentFactory gCoreComponentFactory;
DLLIMPORTEXPORT ComponentFactory* getCoreComponentFactoryPtr();

#endif // COMPONENT_H_INCLUDED
