//!
//! @file   Component.h
//! @author FluMeS
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
    //==========Public functions==========
    //Virtual functions
    virtual void initialize(const double startT, const double stopT);
    virtual void simulate(const double startT, const double Ts);
    virtual void setDesiredTimestep(const double timestep);

    //Name and type
    void setName(string name);
    const string &getName();
    const string &getType();
    const string &getTypeName();
    const string &getTypeCQS();

    //Parameters
    void listParametersConsole();
    double getParameter(const string name);
    void setParameter(const string name, const double value);
    map<string, double> getParameterList();

    //Ports
    vector<Port*> getPortPtrVector();
    Port &getPort(const string portname);

    //System parent
    ComponentSystem &getSystemparent();

    // Component type identification
    bool isComponentC();
    bool isComponentQ();
    bool isComponentSystem();
    bool isComponentSignal();

    //void setTimestep(const double timestep); ///TODO: Should it be possible to set timestep of a component? Should only be possible for a Systemcomponent
    //double getTimestep();

protected:
    //==========Protected member functions==========
    //Constructor - Destructor
    Component(string name="DefaultComponentName", double timestep=0.001);
    virtual ~Component(){};

    //Virtual functions
    virtual void initialize(); ///TODO: Default values are hard set
    virtual void simulateOneTimestep();
    virtual void setTimestep(const double timestep);

    //Parameter functions
    void registerParameter(const string name, const string description, const string unit, double &rValue);

    //Port functions
    Port* addPort(const string portname, const string porttype, const NodeTypeT nodetype, const int id=-1);
    Port* addPowerPort(const string portname, const string nodetype, const int id=-1);
    Port* addReadPort(const string portname, const string nodetype, const int id=-1);
    Port* addWritePort(const string portname, const string nodetype, const int id=-1);
    bool getPort(const string portname, Port* &prPort);
    Port &getPortById(const size_t port_idx);

    //==========Protected member variables==========
    string mTypeCQS;
    string mTypeName;
    double mTimestep, mDesiredTimestep;
    double mTime;
    bool mIsComponentC;
    bool mIsComponentQ;
    bool mIsComponentSystem;
    bool mIsComponentSignal;
    vector<Port*> mPortPtrs;

private:
    //Private member functions
    void setSystemparent(ComponentSystem &rComponentSystem);
    //Port* addInnerPortSetNode(const string portname, const string porttype, Node* pNode);
    void addSubNode(Node* node_ptr);

    //Private member variables
    string mName;
    vector<Node*> mSubNodePtrs;
    vector<CompParameter> mParameters;
    ComponentSystem* mpSystemparent;
};


class DLLIMPORTEXPORT ComponentSystem :public Component
{
public:
    //==========Public functions==========
    //Constructor - Destructor
    ComponentSystem(string name="DefaultComponentSystemName", double timestep=0.001);

    //Set the subsystem CQS type
    void setTypeCQS(const string cqs_type);

    //adding components and system ports
    void addComponents(vector<Component*> components);
    void addComponent(Component &rComponent);
    void addComponent(Component *pComponent);
    Port* addSystemPort(const string portname);

    //Getting added components and component names
    Component* getComponent(string name);
    ComponentSystem* getComponentSystem(string name);
    map<string, string> getComponentNames();

    //connecting components
    void connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2);
    void connect(Component *pComponent1, const string portname1, Component *pComponent2, const string portname2);
    void connect(Port &rPort1, Port &rPort2);

    //initializeand simulate
    void initialize(const double startT, const double stopT);
    void simulate(const double startT, const double stopT);

    //Set desired timestep
    void setDesiredTimestep(const double timestep);

private:
    //==========Private functions==========
    //Time specific functions
    void setTimestep(const double timestep);
    void adjustTimestep(double timestep, vector<Component*> componentPtrs);

    //log specific functions
    void preAllocateLogSpace(const double startT, const double stopT);
    void logAllNodes(const double time);

    //Check if connection ok
    bool connectionOK(Node *pNode, Port *pPort1, Port *pPort2);

    //==========Prvate member variables==========
    //vector<Component*> mSubComponentPtrs; //Problems with inheritance and casting?
    vector<Component*> mComponentSignalptrs;
    vector<Component*> mComponentQptrs;
    vector<Component*> mComponentCptrs;
    map<string, string> mComponentNames;

    NodeFactory mpNodeFactory;
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

typedef ClassFactory<string, Component> ComponentFactory;
extern ComponentFactory gCoreComponentFactory;
DLLIMPORTEXPORT ComponentFactory* getCoreComponentFactoryPtr();

#endif // COMPONENT_H_INCLUDED
