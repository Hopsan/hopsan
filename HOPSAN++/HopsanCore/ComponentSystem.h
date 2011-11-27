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
//! @brief Contains Component base classes as well as Component Parameter class
//!
//$Id$

#ifndef COMPNENTSYSTEM_H
#define COMPNENTSYSTEM_H

#include "Component.h"
#include <list>

namespace tbb {
    class mutex;
}

namespace hopsan {

    class ConnectionAssistant
    {
    public:
        ConnectionAssistant(ComponentSystem *pComponentSystem);

        bool createNewNodeConnection(Port *pPort1, Port *pPort2, Node *&rpCreatedNode);
        bool mergeOrJoinNodeConnection(Port *pPort1, Port *pPort2, Node *&rpCreatedNode);
        bool deleteNodeConnection(Port *pPort1, Port *pPort2);
        bool splitNodeConnection(Port *pPort1, Port *pPort2);

        void determineWhereToStoreNodeAndStoreIt(Node* pNode);
        void clearSysPortNodeTypeIfEmpty(Port *pPort);

        bool ensureNotCrossConnecting(Port *pPort1, Port *pPort2);
        bool ensureSameNodeType(Port *pPort1, Port *pPort2);
        bool ensureConnectionOK(Node *pNode, Port *pPort1, Port *pPort2);

        void ifMultiportAddSubportAndSwapPtr(Port *&rpPort, Port *&rpOrignialPort);
        void ifMultiportCleanupAfterConnect(Port *pSubPort, Port *pMultiPort, const bool wasSucess);
        void ifMultiportPrepareForDissconnect(Port *&rpPort1, Port *&rpPort2, Port *&rpMultiPort1, Port *&rpMultiPort2);
        void ifMultiportCleanupAfterDissconnect(Port *&rpSubPort, Port *pMultiPort, const bool wasSucess);

    private:
        void recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode);
        Port* findMultiportSubportFromOtherPort(const Port *pMultiPort, Port *pOtherPort);
        ComponentSystem *mpComponentSystem; //The system to assist
    };

    class DLLIMPORTEXPORT ComponentSystem :public Component
    {
        friend class ConnectionAssistant;

    public:
        //==========Public functions==========
        //Constructor - Destructor
        ComponentSystem();

        //Set the subsystem CQS type
        void setTypeCQS(typeCQS cqs_type, bool doOnlyLocalSet=false);
        bool changeTypeCQS(const std::string name, const typeCQS newType);
        void determineCQSType();

        //adding removing and renaming components
        void addComponents(std::vector<Component*> components);
        void addComponent(Component *pComponent);
        void renameSubComponent(std::string old_name, std::string new_name);
        void removeSubComponent(std::string name, bool doDelete=false);
        void removeSubComponent(Component *pComponent, bool doDelete=false);
        std::string reserveUniqueName(std::string desiredName);
        void unReserveUniqueName(std::string name);

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
        bool disconnect(Port *pPort1, Port *pPort2);

        // Convenience functions for enable and dissable data logging
        void setAllNodesDoLogData(const bool logornot);

        //initialize and simulate
        bool isSimulationOk();
        void loadStartValues();
        void loadStartValuesFromSimulation();
        bool initialize(const double startT, const double stopT, const size_t nSamples=2048);
        void simulateMultiThreadedOld(const double startT, const double stopT);
        void simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads = 0);
        #ifdef USETBB
        void simulateAndMeasureTime(size_t steps = 1);
        void sortComponentVectorsByMeasuredTime();
        int getNumberOfThreads(size_t nDesiredThreads);
        void distributeCcomponents(std::vector< std::vector<Component*> > &rSplitCVector, size_t nThreads);
        void distributeQcomponents(std::vector< std::vector<Component*> > &rSplitQVector, size_t nThreads);
        void distributeSignalcomponents(std::vector< std::vector<Component*> > &rSplitSignalVector, size_t nThreads);
        void distributeNodePointers(std::vector< std::vector<Node*> > &rSplitNodeVector, size_t nThreads);
        #endif
        void simulate(const double startT, const double stopT);
        void finalize(const double startT, const double stopT);

        void logAllNodes(const double time);

        //Set and get desired timestep
        void setDesiredTimestep(const double timestep);
        void setInheritTimestep(const bool inherit=true);
        bool doesInheritTimestep();
        double getDesiredTimeStep();

        //Stop a running init or simulation
        void stopSimulation();

        #ifdef USETBB
        tbb::mutex *mpStopMutex;
        #endif

        //System parameters
        Parameters &getSystemParameters();

    private:
        //==========Private functions==========
        //Time specific functions
        void setTimestep(const double timestep);
        void adjustTimestep(std::vector<Component*> componentPtrs);

        //log specific functions
        void preAllocateLogSpace(const double startT, const double stopT, const size_t nSamples = 2048);

        //Add and Remove sub nodes
        void addSubNode(Node* node_ptr);
        void removeSubNode(Node* node_ptr);

        //Add and Remove subcomponent ptrs from storage vectors
        void addSubComponentPtrToStorage(Component* pComponent);
        void removeSubComponentPtrFromStorage(Component* pComponent);

        void sortComponentVector(std::vector<Component*> &rOldSignalVector);
        bool componentVectorContains(std::vector<Component*> vector, Component *pComp);

        //UniqueName specific functions
        std::string determineUniquePortName(std::string portname);
        std::string determineUniqueComponentName(std::string name);

        //==========Prvate member variables==========
        typedef std::map<std::string, Component*> SubComponentMapT;
        typedef std::map<std::string, int> ReservedNamesT;
        SubComponentMapT mSubComponentMap;
        ReservedNamesT mReservedNames;
        std::vector<Component*> mComponentSignalptrs;
        std::vector<Component*> mComponentQptrs;
        std::vector<Component*> mComponentCptrs;
        std::vector<Component*> mComponentUndefinedptrs;

        std::vector<Node*> mSubNodePtrs;

        bool volatile mStopSimulation;
//Finns i Component        Parameters *mSystemParameters;
    };
}


#endif // COMPNENTSYSTEM_H
