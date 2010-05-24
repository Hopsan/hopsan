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
#include "Port.h"
#include "CoreUtilities/ClassFactory.h"
#include "win32dll.h"
#include <string>
#include <list>

using namespace std;

class DLLIMPORTEXPORT CompParameter
{
    friend class Component;

public:
    //! @todo getting strings can (maybe, dont really know) be speed up by returning const references instead of copying strings
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
    enum typeCQS {C, Q, S, NOCQSTYPE};
    //==========Public functions==========
    //Virtual functions
    virtual void initialize(const double startT, const double stopT);
    virtual void simulate(const double startT, const double Ts);
    virtual void finalize(const double startT, const double Ts);
    virtual void setDesiredTimestep(const double timestep);

    //Name and type
    void setName(string name, bool doOnlyLocalRename=false);
    const string &getName();
    const string &getTypeName();
    //const string &getTypeCQS();
    typeCQS getTypeCQS();
    string getTypeCQSString();
    string convertTypeCQS2String(typeCQS type);

    //Parameters
    void listParametersConsole();
    const vector<string> getParameterNames();
    const string getParameterUnit(const string name);
    const string getParameterDescription(const string name);
    double getParameterValue(const string name);
    void setParameterValue(const string name, const double value);

    vector<CompParameter> getParameterVector();
    map<string, double> getParameterMap();

    //Ports
    vector<Port*> getPortPtrVector();
    Port *getPort(const string portname);

    //System parent
    ComponentSystem *getSystemParent();

    // Component type identification
    bool isComponentC();
    bool isComponentQ();
    bool isComponentSystem();
    bool isComponentSignal();

    //! @todo Should it be possible to set timestep of a component? Should only be possible for a Systemcomponent
    //void setTimestep(const double timestep);
    //double getTimestep();
    double *getTimePtr();

protected:
    //==========Protected member functions==========
    //Constructor - Destructor
    Component(string name="Component", double timestep=0.001);
    virtual ~Component(){};

    //Virtual functions
    virtual void initialize(); //! @todo Default values are hard set
    virtual void simulateOneTimestep();
    virtual void finalize();
    virtual void setTimestep(const double timestep);

    //Parameter functions
    void registerParameter(const string name, const string description, const string unit, double &rValue);

    //Port functions
    Port* addPort(const string portname, Port::PORTTYPE porttype, const NodeTypeT nodetype);
    Port* addPowerPort(const string portname, const string nodetype);
    Port* addReadPort(const string portname, const string nodetype);
    Port* addWritePort(const string portname, const string nodetype);
    bool getPort(const string portname, Port* &rpPort);
    void renamePort(const string oldname, const string newname);
    void deletePort(const string name);

    //==========Protected member variables==========
    //string mTypeCQS;
    typeCQS mTypeCQS;
    string mTypeName;
    double mTimestep, mDesiredTimestep;
    double mTime;
    bool mIsComponentC;
    bool mIsComponentQ;
    bool mIsComponentSystem;
    bool mIsComponentSignal;

private:
    //Private member functions
    void setSystemParent(ComponentSystem &rComponentSystem);

    //Private member variables
    string mName;
    vector<CompParameter> mParameters;
    ComponentSystem* mpSystemParent;
    typedef map<string, Port*> PortPtrMapT;
    typedef pair<string, Port*> PortPtrPairT;
    PortPtrMapT mPortPtrMap;

};


class DLLIMPORTEXPORT ComponentSystem :public Component
{
private:
    class SubComponentStorage
    {
        friend class ComponentSystem;
    private:
        typedef map<string, Component*> SubComponentMapT;
        SubComponentMapT mSubComponentMap;

    public:
        void add(Component* pComponent);
        Component* get(const string &rName);
        void rename(const string &rOldName, const string &rNewName);
        void remove(const string &rName);
        bool have(const string &rName);
        bool changeTypeCQS(const string &rName, const typeCQS newType);

        vector<Component*> mComponentSignalptrs;
        vector<Component*> mComponentQptrs;
        vector<Component*> mComponentCptrs;
        vector<Component*> mComponentUndefinedptrs;
    };

public:
    //==========Public functions==========
    //Constructor - Destructor
    ComponentSystem(string name="ComponentSystem", double timestep=0.001);

    //Set the subsystem CQS type
    void setTypeCQS(const string cqs_type, bool doOnlyLocalSet=false);
    void setTypeCQS(typeCQS cqs_type, bool doOnlyLocalSet=false);

    //adding removing and renaming components
    void addComponents(vector<Component*> components);
    void addComponent(Component &rComponent);
    void addComponent(Component *pComponent);
    void renameSubComponent(string old_name, string new_name);
    void removeSubComponent(string name, bool doDelete=false);
    void removeSubComponent(Component *pComponent, bool doDelete=false);

    //Add system ports
    Port* addSystemPort(const string portname);
    void renameSystemPort(const string oldname, const string newname);
    void deleteSystemPort(const string name);

    //Getting added components and component names
    Component* getSubComponent(string name);
    ComponentSystem* getSubComponentSystem(string name);
    vector<string> getSubComponentNames();
    bool haveSubComponent(string name);

    //connecting components
    bool connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2);
    bool connect(Component *pComponent1, const string portname1, Component *pComponent2, const string portname2);
    bool connect(Port &rPort1, Port &rPort2);
    bool connect(Port *pPort1, Port *pPort2);
    bool connect(string compname1, string portname1, string compname2, string portname2);
    bool disconnect(string compname1, string portname1, string compname2, string portname2);
    void disconnect(Port *pPort1, Port *pPort2);

    //initializeand simulate
    void initialize(const double startT, const double stopT);
    void simulate(const double startT, const double stopT);
    void finalize(const double startT, const double stopT);

    //Set desired timestep
    void setDesiredTimestep(const double timestep);

    //Get desired timestep
    double getDesiredTimeStep();

    //Stop a running init or simulation
    void stop();

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

    //Add and Remove sub nodes
    void addSubNode(Node* node_ptr);
    void removeSubNode(Node* node_ptr);

    //==========Prvate member variables==========
    SubComponentStorage mSubComponentStorage;
    vector<Node*> mSubNodePtrs;
    NodeFactory mpNodeFactory;

    bool mStop;
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
