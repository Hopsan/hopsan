/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ComponentSystem.h
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains the subsystem component class and connection assistant help class
//!
//$Id$

#ifndef COMPONENTSYSTEM_H
#define COMPONENTSYSTEM_H

#include "Component.h"

namespace tbb {
    class mutex;
}

namespace hopsan {

    class ConnectionAssistant
    {
    public:
        ConnectionAssistant(ComponentSystem *pComponentSystem);

        bool mergeNodeConnection(Port *pPort1, Port *pPort2);
        bool splitNodeConnection(Port *pPort1, Port *pPort2);

        void determineWhereToStoreNodeAndStoreIt(Node* pNode);
        void clearSysPortNodeTypeIfEmpty(Port *pPort);

        bool ensureNotCrossConnecting(Port *pPort1, Port *pPort2);
        bool ensureSameNodeType(Port *pPort1, Port *pPort2);
        bool ensureConnectionOK(Node *pNode, Port *pPort1, Port *pPort2);

        Port* ifMultiportAddSubport(Port *pMaybeMultiport);
        void ifMultiportPrepareDissconnect(Port *pMaybeMultiport1, Port *pMaybeMultiport2, Port *&rpActualPort1, Port *&rpActualPort2);

        void ifMultiportCleanupAfterConnect(Port *pMaybeMultiport, Port *pActualPort, const bool wasSucess);
        void ifMultiportCleanupAfterDissconnect(Port *pMaybeMultiport, Port *pActualPort, const bool wasSucess);

    private:
        void removeNode(Node *pNode);
        void recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode);
        Port* findMultiportSubportFromOtherPort(const Port *pMultiPort, Port *pOtherPort);
        ComponentSystem *mpComponentSystem; //The system to assist
    };

    class DLLIMPORTEXPORT AliasHandler
    {
    public:
        AliasHandler(ComponentSystem *pSystem);
        bool setVariableAlias(const std::string alias, const std::string compName, const std::string portName, const std::string varName);
        bool setVariableAlias(const std::string alias, const std::string compName, const std::string portName, const int varId);
        bool setParameterAlias(const std::string alias, const std::string compName, const std::string parameterName);
        void componentRenamed(const std::string oldCompName, const std::string newCompName);
        void portRenamed(const std::string compName, const std::string oldPortName, const std::string newPortName);
        void componentRemoved(const std::string compName);
        void portRemoved(const std::string compName, const std::string portName);
        bool hasAlias(const std::string alias);
        bool removeAlias(const std::string alias);

        std::vector<std::string> getAliases() const;

        void getVariableFromAlias(const std::string alias, std::string &rCompName, std::string &rPortName, int &rVarId);
        void getVariableFromAlias(const std::string alias, std::string &rCompName, std::string &rPortName, std::string &rVarName);
        void getParameterFromAlias(const std::string alias, std::string &rCompName, std::string &rParameterName);

    private:
        enum {Parameter, Variable};
        typedef struct _ParamOrVariable
        {
            int type;
            std::string componentName;
            std::string name;
        } ParamOrVariableT;

        typedef std::map<std::string, ParamOrVariableT> AliasMapT;
        AliasMapT mAliasMap;
        ComponentSystem *mpSystem;
    };

    class DLLIMPORTEXPORT SimulationHandler
    {
    public:
        enum SimulationErrorTypesT {NotRedy, InitFailed, SimuFailed, FiniFailed};

        //! @todo a doitall function
        //! @todo use the error enums
        bool initializeSystem(const double startT, const double stopT, ComponentSystem* pSystem);
        bool initializeSystem(const double startT, const double stopT, std::vector<ComponentSystem*> &rSystemVector);

        bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, ComponentSystem* pSystem, bool noChanges=false);
        bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false);

        void finalizeSystem(ComponentSystem* pSystem);
        void finalizeSystem(std::vector<ComponentSystem*> &rSystemVector);

        void runCoSimulation(ComponentSystem *pSystem);

    private:
        bool simulateMultipleSystemsMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false);
        bool simulateMultipleSystems(const double startT, const double stopT, std::vector<ComponentSystem*> &rSystemVector);

        std::vector< std::vector<ComponentSystem*> > distributeSystems(const std::vector<ComponentSystem*> &rSystemVector, size_t nThreads);
        void sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector);

        std::vector< std::vector<ComponentSystem*> > mSplitSystemVector;
    };

    class DLLIMPORTEXPORT ComponentSystem :public Component
    {
        friend class ConnectionAssistant;
        friend class AliasHandler;

    public:
        enum UniqeNameEnumT {UniqueComponentNameType, UniqueSysportNameTyp, UniqueSysparamNameType, UniqueAliasNameType, UniqueReservedNameType};

        //==========Public functions==========
        virtual ~ComponentSystem();
        static Component* Creator(){ return new ComponentSystem(); }
        virtual void configure();

        // Subsystem CQS type methods
        bool isComponentSystem() const {return true;}
        CQSEnumT getTypeCQS() const;
        void setTypeCQS(CQSEnumT cqs_type, bool doOnlyLocalSet=false);
        bool changeSubComponentSystemTypeCQS(const std::string name, const CQSEnumT newType);
        void determineCQSType();

        // Adding removing and renaming components
        void addComponents(std::vector<Component*> &rComponents);
        void addComponent(Component *pComponent);
        void renameSubComponent(std::string old_name, std::string new_name);
        void removeSubComponent(std::string name, bool doDelete=false);
        void removeSubComponent(Component *pComponent, bool doDelete=false);
        std::string reserveUniqueName(const std::string desiredName, const UniqeNameEnumT type=UniqueReservedNameType);
        void unReserveUniqueName(const std::string name);

        // System Parameter functions
        bool renameParameter(const std::string oldName, const std::string newName);

        // Handle system ports
        Port* addSystemPort(std::string portName);
        std::string renameSystemPort(const std::string oldname, const std::string newname);
        void deleteSystemPort(const std::string name);

        // Getting added components and component names
        Component* getSubComponentOrThisIfSysPort(std::string name);
        Component* getSubComponent(std::string name);
        ComponentSystem* getSubComponentSystem(std::string name);
        std::vector<std::string> getSubComponentNames();
        bool haveSubComponent(const std::string name) const;
        bool isEmpty() const;

        // Alias handler
        AliasHandler &getAliasHandler();

        // Connecting and disconnecting components
        bool connect(Port *pPort1, Port *pPort2);
        bool connect(const std::string compname1, const std::string portname1, const std::string compname2, const std::string portname2);
        bool disconnect(const std::string compname1, const std::string portname1, const std::string compname2, const std::string portname2);
        bool disconnect(Port *pPort1, Port *pPort2);

        // Convenience functions for enable and dissable data logging
        void setAllNodesDoLogData(const bool logornot);

        // Startvalue loading
        bool doesKeepStartValues();
        void setLoadStartValues(bool load);
        void loadStartValues();
        void loadStartValuesFromSimulation();

        // Parameter loading
        void loadParameters(std::string filePath);
        void loadParameters(std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > > parameterMap);

        // Initialize and simulate
        bool checkModelBeforeSimulation();
        bool initialize(const double startT, const double stopT);
        void simulate(const double startT, const double stopT);
        void simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads = 0, bool noChanges=false);
        void finalize();

        bool simulateAndMeasureTime(const size_t nSteps);
        double getTotalMeasuredTime();
        void sortComponentVectorsByMeasuredTime();
        void distributeCcomponents(std::vector< std::vector<Component*> > &rSplitCVector, size_t nThreads);
        void distributeQcomponents(std::vector< std::vector<Component*> > &rSplitQVector, size_t nThreads);
        void distributeSignalcomponents(std::vector< std::vector<Component*> > &rSplitSignalVector, size_t nThreads);
        void distributeNodePointers(std::vector< std::vector<Node*> > &rSplitNodeVector, size_t nThreads);

        // Set and get desired timestep
        void setDesiredTimestep(const double timestep);
        void setInheritTimestep(const bool inherit=true);
        bool doesInheritTimestep() const;
        double getDesiredTimeStep() const;

        // Log functions
        //void logTimeAndNodes(const double time);
        void logTimeAndNodes(const size_t simStep);
        void enableLog();
        void disableLog();
        std::vector<double>* getLogTimeVector();
        void setNumLogSamples(const size_t nLogSamples);
        size_t getNumLogSamples() const;
        size_t getNumActuallyLoggedSamples() const;

        // Stop a running init or simulation
        void stopSimulation();
        bool wasSimulationAborted();

        // System parameters
        bool setSystemParameter(const std::string name, const std::string value, const std::string type, const std::string description="", const std::string unit="", const bool force=false);
        void unRegisterParameter(const std::string name);
        Parameters &getSystemParameters();
        void addSearchPath(const std::string searchPath);

        // Add and Remove sub-nodes
        void addSubNode(Node* pNode);
        void removeSubNode(Node* pNode);

    protected:
        // Constructor - Destructor- Creator
        ComponentSystem();

    private:
        //==========Private functions==========
        // Time specific functions
        void setTimestep(const double timestep);
        void adjustTimestep(std::vector<Component*> componentPtrs);
        void setupLogTimesteps(const double startT, const double stopT, const double Ts, const size_t nLogSamples);

        // log specific functions
        void setLogSettingsSampleTime(double log_dt, double start, double stop, double sampletime);
        void setLogSettingsSkipFactor(double factor, double start, double stop, double sampletime);
        void setLogSettingsNSamples(int nSamples, double start, double stop, double sampletime);
        void preAllocateLogSpace(const double startT, const double stopT, const size_t nSamples = 2048);

        // Add and Remove subcomponent ptrs from storage vectors
        void addSubComponentPtrToStorage(Component* pComponent);
        void removeSubComponentPtrFromStorage(Component* pComponent);

        // Celar all contents of the system (use in destructor)
        void clear();

        bool sortComponentVector(std::vector<Component*> &rOldSignalVector);
        bool componentVectorContains(std::vector<Component*> vector, Component *pComp);

        // UniqueName specific functions
        std::string determineUniquePortName(std::string portname);
        std::string determineUniqueComponentName(std::string name);
        bool hasReservedUniqueName(const std::string &rName) const;

        //==========Private member variables==========
        CQSEnumT mTypeCQS;

        typedef std::map<std::string, Component*> SubComponentMapT;
        SubComponentMapT mSubComponentMap;
        std::vector<Component*> mComponentSignalptrs;
        std::vector<Component*> mComponentQptrs;
        std::vector<Component*> mComponentCptrs;
        std::vector<Component*> mComponentUndefinedptrs;
        std::vector<Node*> mSubNodePtrs;

        typedef std::map<std::string, UniqeNameEnumT> TakenNamesMapT;
        TakenNamesMapT mTakenNames;


        bool volatile mStopSimulation;

        // This block of variables are only used whith TBB but they must be incuded allways else
        // components inhereting ComponentSystem will not know that they exist resulting in overwriting memory
        tbb::mutex *mpStopMutex;
        std::vector<double *> mvTimePtrs;
        std::vector< std::vector<Component*> > mSplitCVector;
        std::vector< std::vector<Component*> > mSplitQVector;
        std::vector< std::vector<Component*> > mSplitSignalVector;
        std::vector< std::vector<Node*> > mSplitNodeVector;
        //------------------------------------------------------------------

        bool mKeepStartValues;

        AliasHandler mAliasHandler;

        // Log related variables
        size_t mRequestedNumLogSamples, mnLogSlots, mLogCtr;
        double mLogTimeDt;//, mLastLogTime;
        bool mEnableLogData;
        std::vector<double> mTimeStorage;

        // Log and timestep
        std::vector<size_t> mLogTheseTimeSteps;
        size_t mTotalTakenSimulationSteps;

        //Finns i Component        Parameters *mSystemParameters;
    };
}


#endif // COMPONENTSYSTEM_H
