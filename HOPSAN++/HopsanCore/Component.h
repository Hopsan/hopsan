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

    private:
        CompParameter(const std::string name, const std::string description, const std::string unit, double &rValue);

        void setValue(const double value);

        std::string mName;
        std::string mDescription;
        std::string mUnit;
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
        void setParameterValue(const std::string name, const double value);

        std::vector<CompParameter> getParameterVector();
        std::map<std::string, double> getParameterMap();

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


    protected:
        //==========Protected member functions==========
        //Constructor - Destructor
        Component(std::string name="Component", double timestep=0.001);
        virtual ~Component(){};

        //Virtual functions
        virtual void initialize(); //! @todo Default values are hard set
        virtual void simulateOneTimestep();
        virtual void finalize();
        virtual void setTimestep(const double timestep);

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
        void setSystemParent(ComponentSystem &rComponentSystem);

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
    private:
        class SubComponentStorage
        {
            friend class ComponentSystem;
        private:
            typedef std::map<std::string, Component*> SubComponentMapT;
            SubComponentMapT mSubComponentMap;

        public:
            void add(Component* pComponent);
            Component* get(const std::string &rName);
            void rename(const std::string &rOldName, const std::string &rNewName);
            void remove(const std::string &rName);
            bool have(const std::string &rName);
            bool changeTypeCQS(const std::string &rName, const typeCQS newType);

            std::vector<Component*> mComponentSignalptrs;
            std::vector<Component*> mComponentQptrs;
            std::vector<Component*> mComponentCptrs;
            std::vector<Component*> mComponentUndefinedptrs;
        };

    public:
        //==========Public functions==========
        //Constructor - Destructor
        ComponentSystem(std::string name="ComponentSystem", double timestep=0.001);

        //Set the subsystem CQS type
        void setTypeCQS(const std::string cqs_type, bool doOnlyLocalSet=false);
        void setTypeCQS(typeCQS cqs_type, bool doOnlyLocalSet=false);

        //adding removing and renaming components
        void addComponents(std::vector<Component*> components);
        void addComponent(Component &rComponent);
        void addComponent(Component *pComponent);
        void renameSubComponent(std::string old_name, std::string new_name);
        void removeSubComponent(std::string name, bool doDelete=false);
        void removeSubComponent(Component *pComponent, bool doDelete=false);

        //Add system ports
        Port* addSystemPort(std::string portname);
        std::string renameSystemPort(const std::string oldname, const std::string newname);
        void deleteSystemPort(const std::string name);

        //Getting added components and component names
        Component* getComponent(std::string name);
        Component* getSubComponent(std::string name);
        ComponentSystem* getSubComponentSystem(std::string name);
        std::vector<std::string> getSubComponentNames();
        bool haveSubComponent(std::string name);

        //connecting components
        bool connect(Port *pPort1, Port *pPort2);
        bool connect(std::string compname1, std::string portname1, std::string compname2, std::string portname2);
        bool disconnect(std::string compname1, std::string portname1, std::string compname2, std::string portname2);
        void disconnect(Port *pPort1, Port *pPort2);

        //initializeand simulate
        bool isSimulationOk();
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
        void adjustTimestep(double timestep, std::vector<Component*> componentPtrs);

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
        std::vector<Node*> mSubNodePtrs;
        NodeFactory mpNodeFactory;

        bool mStop;
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
