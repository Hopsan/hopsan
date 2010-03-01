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
#include <list>

using namespace std;

class DLLIMPORTEXPORT CompParameter
{
    friend class Component;

public:
    ///TODO: getting strings can (probably) be speed up by returning const references instead of copying strings
    string getName();
    string getDesc();
    string getUnit();

    double getValue();

private:
    CompParameter(const string name, const string description, const string unit, double &rValue);

    void setValue(const double value);

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
    const string &getTypeName();
    const string &getTypeCQS();

    //Parameters
    void listParametersConsole();
    double getParameter(const string name);
    void setParameter(const string name, const double value);
    vector<CompParameter> getParameterVector();
    map<string, double> getParameterMap();

    //Ports
    vector<Port*> getPortPtrVector();
    Port &getPort(const string portname);

    //System parent
    ComponentSystem &getSystemParent();

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
    bool getPort(const string portname, Port* &rpPort);
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
    void setSystemParent(ComponentSystem &rComponentSystem);

    //Private member variables
    string mName;
    vector<CompParameter> mParameters;
    ComponentSystem* mpSystemParent;
};


class DLLIMPORTEXPORT ComponentSystem :public Component
{
private:
    class SubComponentStorage
    {
        friend class ComponentSystem;
    private:
        //!This is a helper class for SubComponentStorage containing some component info
        class SubComponentInfo
        {
        public:
            string cqs_type;
            int idx;
        };

        string modifyName(string name);

        map<string, SubComponentInfo> mSubComponentMap;


    public:
        void add(Component* pComponent);
        Component* get(string name);
        void rename(string old_name, string new_name);
        void erase(string name);
        bool have(string name);

        vector<Component*> mComponentSignalptrs;
        vector<Component*> mComponentQptrs;
        vector<Component*> mComponentCptrs;
    };

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
    void renameSubComponent(string old_name, string new_name);
    Port* addSystemPort(const string portname);

    //Getting added components and component names
    Component* getSubComponent(string name);
    ComponentSystem* getSubComponentSystem(string name);
    vector<string> getSubComponentNames();
    bool haveSubComponent(string name);

    //connecting components
    void connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2);
    void connect(Component *pComponent1, const string portname1, Component *pComponent2, const string portname2);
    void connect(Port &rPort1, Port &rPort2);
    void disconnect(Port *pPort1, Port *pPort2);

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

    //Add adn Remove sub nodes
    void addSubNode(Node* node_ptr);
    void removeSubNode(Node* node_ptr);

    //==========Prvate member variables==========
    SubComponentStorage mSubComponentStorage;
    vector<Node*> mSubNodePtrs;
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
