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
//#include "CoreUtilities/ClassFactory.hpp"
#include "win32dll.h"
#include <string>
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


    class ComponentSystem; //Forward declaration
    class HopsanEssentials;

    class DLLIMPORTEXPORT Component
    {
        friend class ComponentSystem;
        friend class HopsanEssentials; //Need to be able to set typename

    public:
        virtual ~Component();

        enum typeCQS {C, Q, S, UNDEFINEDCQSTYPE};
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
        typeCQS getTypeCQS();
        std::string getTypeCQSString();

        //Parameters
        void listParametersConsole();
        const std::vector<std::string> getParameterNames();
        const std::string getParameterUnit(const std::string name);
        const std::string getParameterDescription(const std::string name);
        double getParameterValue(const std::string name);
        double getDefaultParameterValue(const std::string name);
        double *getParameterValuePtr(const std::string name);
        std::string getParameterValueTxt(const std::string name);
        bool setParameterValue(const std::string name, const double value);
        bool setParameterValue(const std::string parName, const std::string sysParName);

        std::vector<CompParameter> getParameterVector();
        std::map<std::string, double> getParameterMap();

        //Start values
        double getStartValue(Port* pPort, const size_t idx);
        void setStartValue(Port* pPort, const size_t &idx, const double &value);
        void disableStartValue(Port* pPort, const size_t &idx);

        //Ports
        std::vector<Port*> getPortPtrVector();
        Port *getPort(const std::string portname);

        //System parent
        ComponentSystem *getSystemParent();
        size_t getModelHierarchyDepth();

        std::map<std::string, double> mDefaultParameters; //!< @todo should not be public

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
        void addWarningMessage(std::string message);
        void addErrorMessage(std::string message);
        void addInfoMessage(std::string message);

        //Temporarily made public for RT-simulation
        virtual void initialize(); //! @todo Default values are hard set

    protected:
        //==========Protected member functions==========
        //Constructor - Destructor
        Component(std::string name="Component");

        //Virtual functions
        virtual void simulateOneTimestep();
        virtual void finalize();
        virtual void secretFinalize();
        virtual void setTimestep(const double timestep);

        //Stop a running simulation
        void stopSimulation();

        //Parameter functions
        void registerParameter(const std::string name, const std::string description, const std::string unit, double &rValue);

        //Port functions
        Port* addPort(const std::string portname, PORTTYPE porttype, const NodeTypeT nodetype, Port::CONREQ connection_requirement);
        Port* addPowerMultiPort(const std::string portname, const std::string nodetype, Port::CONREQ connection_requirement=Port::REQUIRED);
        Port* addPowerPort(const std::string portname, const std::string nodetype, Port::CONREQ connection_requirement=Port::REQUIRED);
        Port* addReadMultiPort(const std::string portname, const std::string nodetype, Port::CONREQ connection_requirement=Port::REQUIRED);
        Port* addReadPort(const std::string portname, const std::string nodetype, Port::CONREQ connection_requirement=Port::REQUIRED);
        Port* addWritePort(const std::string portname, const std::string nodetype, Port::CONREQ connection_requirement=Port::REQUIRED);
        bool getPort(const std::string portname, Port* &rpPort);
        std::string renamePort(const std::string oldname, const std::string newname);
        void deletePort(const std::string name);

        //NodeData ptr function
        double *getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue=0, int portIdx=-1);
        double *getSafeMultiPortNodeDataPtr(Port* pPort, const int portIdx, const int dataId, const double defaultValue=0);

        //Unique name functions
        virtual std::string determineUniquePortName(std::string portname);

        //==========Protected member variables==========
        typeCQS mTypeCQS;
        double mTimestep, mDesiredTimestep;
        double mTime;
        bool mIsComponentC; //!< @todo we should nou need these bools, we can check type==CQSTYPE in the isComponent*() functions
        bool mIsComponentQ;
        bool mIsComponentSystem;
        bool mIsComponentSignal;

        size_t mModelHierarchyDepth; //This variable containes the depth of the system in the model hierarchy, (used by connect to figure out where to store nodes)

    private:
        typedef std::map<std::string, Port*> PortPtrMapT;
        typedef std::pair<std::string, Port*> PortPtrPairT;

        //Private member functions
        void setSystemParent(ComponentSystem *pComponentSystem);
        void setTypeName(const std::string typeName); //This is suposed to be used by hopsan essentials to set the typename to the same as the registered key value

        //Private member variables
        std::string mName;
        std::string mTypeName;
        std::vector<CompParameter> mParameters;
        std::vector<double*> mDummyNDptrs; //This vector is used by components to store dummy NodeData pointers that are created for non connected optional ports
        ComponentSystem* mpSystemParent;
        PortPtrMapT mPortPtrMap;
        double mMeasuredTime;
    };



    class DLLIMPORTEXPORT ComponentSignal :public Component
    {
    protected:
        ComponentSignal(std::string name="");
    };


    class DLLIMPORTEXPORT ComponentC :public Component
    {
    protected:
        ComponentC(std::string name="");
    };


    class DLLIMPORTEXPORT ComponentQ :public Component
    {
    protected:
        ComponentQ(std::string name="");
    };

    typedef ClassFactory<std::string, Component> ComponentFactory;
    //extern ComponentFactory gCoreComponentFactory;
    //DLLIMPORTEXPORT ComponentFactory* getCoreComponentFactoryPtr();
}

#endif // COMPONENT_H_INCLUDED
