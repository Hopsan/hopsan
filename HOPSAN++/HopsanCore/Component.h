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
#include "CoreUtilities/ClassFactory.hpp"
#include "win32dll.h"
#include <string>
#include <list>
#include <map>

namespace hopsan {

    class DLLIMPORTEXPORT CompParameter
    {
        friend class Component;

    public:
        //! @todo getting strings can (maybe, dont really know) be speed up by returning const references instead of copying strings
        std::string getName();
        std::string getDesc();
        std::string getUnit();

        double getValue();
        double *getValuePtr();

    private:
        CompParameter(const std::string name, const std::string description, const std::string unit, double &rValue);

        void setValue(const double value);
        //void setMappedValue(std::string key);

        std::string mName;
        std::string mDescription;
        std::string mUnit;
        double* mpValue;
        std::string mMapKey;
    };


    //! @class SystemParameters
    //! @brief Adds a new system parameter
    //!
    //! This class is used in ComponentSystems to contain "system global" parameters
    class DLLIMPORTEXPORT SystemParameters
    {
    public:
        bool add(std::string sysParName, double value);
        bool getValue(std::string sysParName, double &value);
        std::map<std::string, double> getSystemParameterMap();
        std::string findOccurrence(double *mappedValue);
        void erase(std::string sysParName);
        bool mapParameter(std::string sysParName, double *mappedValue);
        void unMapParameter(std::string sysParName, double *mappedValue);
        void unMapParameter(double *mappedValue);
        void update();
        bool update(std::string sysParName);

    protected:

    private:
        typedef std::list<double*> DblPointerList;
        typedef std::pair<double, DblPointerList> SystemParameter;

        std::map<std::string, SystemParameter> mSystemParameters;
    };


    class ComponentSystem; //Forward declaration

    class DLLIMPORTEXPORT Component
    {
        friend class ComponentSystem;

    public:
        virtual ~Component();

        enum typeCQS {C, Q, S, NOCQSTYPE};
        //==========Public functions==========
        //Virtual functions
        virtual void loadStartValues();
        virtual void loadStartValuesFromSimulation();
        virtual void initialize(const double startT, const double stopT, const size_t nSamples);
        virtual void simulate(const double startT, const double Ts);
        virtual void finalize(const double startT, const double Ts);
        virtual void setDesiredTimestep(const double timestep);
        virtual bool isSimulationOk();

        //Name and type
        void setName(std::string name, bool doOnlyLocalRename=false);
        const std::string &getName();
        const std::string &getTypeName();
        //const string &getTypeCQS();
        typeCQS getTypeCQS();
        std::string getTypeCQSString();
        std::string convertTypeCQS2String(typeCQS type);

        //Parameters
        void listParametersConsole();
        const std::vector<std::string> getParameterNames();
        const std::string getParameterUnit(const std::string name);
        const std::string getParameterDescription(const std::string name);
        double getParameterValue(const std::string name);
        double *getParameterValuePtr(const std::string name);
        std::string getParameterValueTxt(const std::string name);
        bool setParameterValue(const std::string name, const double value);
        bool setParameterValue(const std::string parName, const std::string sysParName);

        std::vector<CompParameter> getParameterVector();
        std::map<std::string, double> getParameterMap();

        //Start values
        double getStartValue(Port* pPort, const size_t idx);
        void setStartValue(Port* pPort, const size_t &idx, const double &value);

        //Ports
        std::vector<Port*> getPortPtrVector();
        Port *getPort(const std::string portname);

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

        void setMeasuredTime(double time);
        double getMeasuredTime();

        void addDebugMessage(std::string message);

        //Temporarily made public for RT-simulation
        virtual void initialize(); //! @todo Default values are hard set

    protected:
        //==========Protected member functions==========
        //Constructor - Destructor
        Component(std::string name="Component", double timestep=0.001);

        //Virtual functions
        virtual void simulateOneTimestep();
        virtual void finalize();
        virtual void setTimestep(const double timestep);

        //Stop a running simulation
        void stopSimulation();

        //Parameter functions
        void registerParameter(const std::string name, const std::string description, const std::string unit, double &rValue);

        //Port functions
        Port* addPort(const std::string portname, Port::PORTTYPE porttype, const NodeTypeT nodetype, Port::CONREQ connection_requirement);
        Port* addPowerPort(const std::string portname, const std::string nodetype);
        Port* addReadPort(const std::string portname, const std::string nodetype, Port::CONREQ connection_requirement=Port::REQUIRED);
        Port* addWritePort(const std::string portname, const std::string nodetype, Port::CONREQ connection_requirement=Port::REQUIRED);
        bool getPort(const std::string portname, Port* &rpPort);
        std::string renamePort(const std::string oldname, const std::string newname);
        void deletePort(const std::string name);

        //Unique name functions
        virtual std::string determineUniquePortName(std::string portname);

        //==========Protected member variables==========
        //string mTypeCQS;
        typeCQS mTypeCQS;
        std::string mTypeName;
        double mTimestep, mDesiredTimestep;
        double mTime;
        bool mIsComponentC;
        bool mIsComponentQ;
        bool mIsComponentSystem;
        bool mIsComponentSignal;

    private:
        //Private member functions
        void setSystemParent(ComponentSystem *pComponentSystem);

        //Private member variables
        std::string mName;
        std::vector<CompParameter> mParameters;
        ComponentSystem* mpSystemParent;
        typedef std::map<std::string, Port*> PortPtrMapT;
        typedef std::pair<std::string, Port*> PortPtrPairT;
        PortPtrMapT mPortPtrMap;
        double mMeasuredTime;
    };


    class DLLIMPORTEXPORT ComponentSystem :public Component
    {
    public:
        //==========Public functions==========
        //Constructor - Destructor
        ComponentSystem(std::string name="ComponentSystem", double timestep=0.001);

        //Load from external file
        void loadSystemFromFile(std::string filepath);

        //Set the subsystem CQS type
        void setTypeCQS(const std::string cqs_type, bool doOnlyLocalSet=false);
        void setTypeCQS(typeCQS cqs_type, bool doOnlyLocalSet=false);
        bool changeTypeCQS(const std::string name, const typeCQS newType);

        //adding removing and renaming components
        void addComponents(std::vector<Component*> components);
        void addComponent(Component *pComponent);
        void renameSubComponent(std::string old_name, std::string new_name);
        void removeSubComponent(std::string name, bool doDelete=false);
        void removeSubComponent(Component *pComponent, bool doDelete=false);

        //Handle system ports
        Port* addSystemPort(std::string portname);
        std::string renameSystemPort(const std::string oldname, const std::string newname);
        void deleteSystemPort(const std::string name);

        //Getting added components and component names
        Component* getComponent(std::string name);
        Component* getSubComponent(std::string name);
        ComponentSystem* getSubComponentSystem(std::string name);
        std::vector<std::string> getSubComponentNames();
        bool haveSubComponent(std::string name);

        //Connecting and disconnecting components
        bool connect(Port *pPort1, Port *pPort2);
        bool connect(std::string compname1, std::string portname1, std::string compname2, std::string portname2);
        bool disconnect(std::string compname1, std::string portname1, std::string compname2, std::string portname2);
        void disconnect(Port *pPort1, Port *pPort2);

        //initialize and simulate
        bool isSimulationOk();
        void loadStartValues();
        void loadStartValuesFromSimulation();
        void initialize(const double startT, const double stopT, const size_t nSamples=2048);
        void simulateMultiThreadedOld(const double startT, const double stopT);
        void simulateMultiThreaded(const double startT, const double stopT, const size_t nThreads = 0);
        void simulate(const double startT, const double stopT);
        void finalize(const double startT, const double stopT);

        //Set and get desired timestep
        void setDesiredTimestep(const double timestep);
        double getDesiredTimeStep();

        //Stop a running init or simulation
        void stop();

        //System parameters
        SystemParameters &getSystemParameters();

    private:
        //==========Private functions==========
        //Time specific functions
        void setTimestep(const double timestep);
        void adjustTimestep(double timestep, std::vector<Component*> componentPtrs);

        //log specific functions
        void preAllocateLogSpace(const double startT, const double stopT, const size_t nSamples = 2048);
        void logAllNodes(const double time);

        //Try to find sub component ptr
        //Component* getSubComponent(std::string name);

        //Check if connection ok
        bool connectionOK(Node *pNode, Port *pPort1, Port *pPort2);

        //Add and Remove sub nodes
        void addSubNode(Node* node_ptr);
        void removeSubNode(Node* node_ptr);

        //Add and Remove subcomponent ptrs from storage vectors
        void addSubComponentPtrToStorage(Component* pComponent);
        void removeSubComponentPtrFromStorage(Component* c_ptr);

        //UniqueName specific functions
        std::string determineUniquePortName(std::string portname);
        std::string determineUniqueComponentName(std::string name);

        //==========Prvate member variables==========
        typedef std::map<std::string, Component*> SubComponentMapT;
        SubComponentMapT mSubComponentMap;
        std::vector<Component*> mComponentSignalptrs;
        std::vector<Component*> mComponentQptrs;
        std::vector<Component*> mComponentCptrs;
        std::vector<Component*> mComponentUndefinedptrs;

        std::vector<Node*> mSubNodePtrs;
        NodeFactory mpNodeFactory;

        bool mStop;

        SystemParameters mSystemParameters;
    };

    class DLLIMPORTEXPORT ComponentSignal :public Component
    {
    protected:
        ComponentSignal(std::string name, double timestep=0.001);
    };


    class DLLIMPORTEXPORT ComponentC :public Component
    {
    protected:
        ComponentC(std::string name, double timestep=0.001);
    };


    class DLLIMPORTEXPORT ComponentQ :public Component
    {
    protected:
        ComponentQ(std::string name, double timestep=0.001);
    };

    typedef ClassFactory<std::string, Component> ComponentFactory;
    extern ComponentFactory gCoreComponentFactory;
    DLLIMPORTEXPORT ComponentFactory* getCoreComponentFactoryPtr();
}

#endif // COMPONENT_H_INCLUDED
