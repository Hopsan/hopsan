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

    enum ParallelAlgorithmT {OfflineSchedulingAlgorithm, TaskPoolAlgorithm, WorkStealingAlgorithm, ParallelForAlgorithm, ParallelForTbbAlgorithm, RandomTaskPoolAlgorithm};

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
        class ConnOKCounters
        {
        public:
            size_t nReadPorts;
            size_t nWritePorts;
            size_t nPowerPorts;
            size_t nSystemPorts;
            size_t nOwnSystemPorts; // Number of systemports that belong to the connecting system
            size_t nInterfacePorts; // This can be system ports or other ports acting as interface ports in systems
            //size_t nMultiPorts;

            size_t nCComponents;
            size_t nQComponents;
            size_t nSYScomponentCs;
            size_t nSYScomponentQs;

            size_t nNonInterfaceQPowerPorts;
            size_t nNonInterfaceCPowerPorts;

            ConnOKCounters()
            {
                nReadPorts = 0;
                nWritePorts = 0;
                nPowerPorts = 0;
                nSystemPorts = 0;
                nOwnSystemPorts = 0;
                nInterfacePorts = 0;
                //nMultiPorts = 0;

                nCComponents = 0;
                nQComponents = 0;
                nSYScomponentCs = 0;
                nSYScomponentQs = 0;

                nNonInterfaceQPowerPorts = 0;
                nNonInterfaceCPowerPorts = 0;
            }
        };

        void checkPort(const Port *pPort, ConnOKCounters &rCounters);
        void removeNode(Node *pNode);
        void recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode);
        Port* findMultiportSubportFromOtherPort(const Port *pMultiPort, Port *pOtherPort);
        ComponentSystem *mpComponentSystem; //The system to assist
    };

    class DLLIMPORTEXPORT AliasHandler
    {
    public:
        AliasHandler(ComponentSystem *pSystem);
        bool setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const HString &rVarName);
        bool setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const int varId);
        bool setParameterAlias(const HString &rAlias, const HString &rCompName, const HString &rParameterName);
        void componentRenamed(const HString &rOldCompName, const HString &rNewCompName);
        void portRenamed(const HString &rCompName, const HString &rOldPortName, const HString &rNewPortName);
        void componentRemoved(const HString &rCompName);
        void portRemoved(const HString &rCompName, const HString &rPortName);
        bool hasAlias(const HString &rAlias);
        bool removeAlias(const HString &rAlias);

        std::vector<HString> getAliases() const;

        void getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, int &rVarId);
        void getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, HString &rVarName);
        void getParameterFromAlias(const HString &rAlias, HString &rCompName, HString &rParameterName);

    private:
        enum {Parameter, Variable};
        typedef struct _ParamOrVariable
        {
            int type;
            HString componentName;
            HString name;
        } ParamOrVariableT;

        typedef std::map<HString, ParamOrVariableT> AliasMapT;
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

        bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, ComponentSystem* pSystem, bool noChanges=false, ParallelAlgorithmT algorithm=OfflineSchedulingAlgorithm);
        bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false, ParallelAlgorithmT algorithm=OfflineSchedulingAlgorithm);

        void finalizeSystem(ComponentSystem* pSystem);
        void finalizeSystem(std::vector<ComponentSystem*> &rSystemVector);

        void runCoSimulation(ComponentSystem *pSystem);

    private:
        bool simulateMultipleSystemsMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false);
        bool simulateMultipleSystems(const double stopT, std::vector<ComponentSystem*> &rSystemVector);

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
        typedef std::map<HString, std::pair<std::vector<HString>, std::vector<HString> > > SetParametersMapT;

        //==========Public functions==========
        virtual ~ComponentSystem();
        static Component* Creator(){ return new ComponentSystem(); }
        virtual void configure();

        // Subsystem CQS type methods
        bool isComponentSystem() const {return true;}
        CQSEnumT getTypeCQS() const;
        void setTypeCQS(CQSEnumT cqs_type, bool doOnlyLocalSet=false);
        bool changeSubComponentSystemTypeCQS(const HString &rName, const CQSEnumT newType);
        void determineCQSType();
        bool isTopLevelSystem() const;

        // Adding removing and renaming components
        void addComponents(std::vector<Component*> &rComponents);
        void addComponent(Component *pComponent);
        void renameSubComponent(const HString &rOldName, const HString &rNewName);
        void removeSubComponent(const HString &rName, bool doDelete=false);
        void removeSubComponent(Component *pComponent, bool doDelete=false);
        HString reserveUniqueName(const HString &rDesiredName, const UniqeNameEnumT type=UniqueReservedNameType);
        void unReserveUniqueName(const HString &rName);

        // System Parameter functions
        bool renameParameter(const HString &rOldName, const HString &rNewName);

        // Handle system ports
        Port* addSystemPort(HString portName, const HString &rDescription="");
        HString renameSystemPort(const HString &rOldname, const HString &rNewname);
        void deleteSystemPort(const HString &rName);

        // Getting added components and component names
        Component* getSubComponentOrThisIfSysPort(const HString &rName);
        Component* getSubComponent(const HString &rName) const;
        const std::vector<Component*> getSubComponents() const;
        ComponentSystem* getSubComponentSystem(const HString &rName) const;
        std::vector<HString> getSubComponentNames() const;
        bool haveSubComponent(const HString &rName) const;
        bool isEmpty() const;

        // Alias handler
        AliasHandler &getAliasHandler();

        // Connecting and disconnecting components
        bool connect(Port *pPort1, Port *pPort2);
        bool connect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2);
        bool disconnect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2);
        bool disconnect(Port *pPort1, Port *pPort2);

        // Convenience functions for enable and dissable data logging
        void setAllNodesDoLogData(const bool logornot);

        // Startvalue loading
        bool doesKeepStartValues();
        void setLoadStartValues(bool load);
        void loadStartValues();
        void loadStartValuesFromSimulation();

        // Parameter loading
        void loadParameters(const HString &rFilePath);
        void loadParameters(const SetParametersMapT &rParameterMap);

        // Initialize and simulate
        bool checkModelBeforeSimulation();
        virtual bool preInitialize();
        bool initialize(const double startT, const double stopT);
        void simulate(const double stopT);
        virtual void simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads = 0, const bool noChanges=false, ParallelAlgorithmT algorithm=OfflineSchedulingAlgorithm);
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
        void logTimeAndNodes(const size_t simStep);
        void enableLog();
        void disableLog();
        std::vector<double>* getLogTimeVector();
        void setNumLogSamples(const size_t nLogSamples);
        double getLogStartTime() const;
        void setLogStartTime(const double logStartTime);
        size_t getNumLogSamples() const;
        size_t getNumActuallyLoggedSamples() const;

        // Stop a running init or simulation
        void stopSimulation(const HString &rReason="");
        bool wasSimulationAborted();

        // System parameters
        bool setSystemParameter(const HString &rName, const HString &rValue, const HString &rType, const HString &rDescription="", const HString &rUnit="", const bool force=false);
        void unRegisterParameter(const HString &name);
        ParameterEvaluatorHandler &getSystemParameters();
        void addSearchPath(const HString &searchPath);

        // Add and Remove sub-nodes
        void addSubNode(Node* pNode);
        void removeSubNode(Node* pNode);

    protected:
        // Constructor - Destructor- Creator
        ComponentSystem();

        // Internal Flags
        //! @brief This bool can be toggled off in programmed subsystems to avoid anoying warnings
        //! @ingroup ComponentPowerAuthorFunctions
        bool mWarnIfUnusedSystemParameters;

        // Log and timestep
        std::vector<size_t> mLogTheseTimeSteps;
        size_t mTotalTakenSimulationSteps;

        typedef std::map<HString, Component*> SubComponentMapT;
        SubComponentMapT mSubComponentMap;

    private:
        //==========Private functions==========
        // Time specific functions
        void setTimestep(const double timestep);
        void adjustTimestep(std::vector<Component*> componentPtrs);

        // log specific functions
        //! @todo restore these in some way
//        void setLogSettingsSampleTime(double log_dt, double start, double stop, double sampletime);
//        void setLogSettingsSkipFactor(double factor, double start, double stop, double sampletime);
        void setupLogSlotsAndTs(const double simStartT, const double simStopT, const double simTs);
        void preAllocateLogSpace();

        // Add and Remove subcomponent ptrs from storage vectors
        void addSubComponentPtrToStorage(Component* pComponent);
        void removeSubComponentPtrFromStorage(Component* pComponent);

        // Celar all contents of the system (use in destructor)
        void clear();

        bool sortComponentVector(std::vector<Component*> &rOldSignalVector);
        bool componentVectorContains(std::vector<Component*> vector, Component *pComp);

        // UniqueName specific functions
        HString determineUniquePortName(const HString &rPortname);
        HString determineUniqueComponentName(const HString &rName) const;
        bool hasReservedUniqueName(const HString &rName) const;

        //==========Private member variables==========
        CQSEnumT mTypeCQS;

        std::vector<Component*> mComponentSignalptrs;
        std::vector<Component*> mComponentQptrs;
        std::vector<Component*> mComponentCptrs;
        std::vector<Component*> mComponentUndefinedptrs;
        std::vector<Node*> mSubNodePtrs;

        typedef std::map<HString, UniqeNameEnumT> TakenNamesMapT;
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
        double mRequestedLogStartTime, mLogTimeDt;
        bool mEnableLogData;
        std::vector<double> mTimeStorage;

        //Finns i Component        Parameters *mSystemParameters;
    };


    class ConditionalComponentSystem : public ComponentSystem
    {
    public:
        static Component* Creator(){ return new ConditionalComponentSystem(); }
        void configure();
        void simulate(const double stopT);
        void simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges, ParallelAlgorithmT algorithm);
    private:
        double *mpCondition;
        bool mAsleep;
    };
}


#endif // COMPONENTSYSTEM_H
