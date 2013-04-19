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
//! @file   ComponentSystem.cc
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains the subsystem component class and connection assistant help class
//!
//$Id$

#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"
#include "HopsanEssentials.h"
#include "CoreUtilities/MultiThreadingUtilities.h"
#include "CoreUtilities/CoSimulationUtilities.h"
#include "CoreUtilities/HmfLoader.h"

#ifdef USETBB
#include "mutex.h"
#include "atomic.h"
#include "tick_count.h"
#include "task_group.h"
#endif

using namespace std;
using namespace hopsan;


bool SimulationHandler::initializeSystem(const double startT, const double stopT, ComponentSystem* pSystem)
{
    if (pSystem->checkModelBeforeSimulation())
    {
        return pSystem->initialize(startT, stopT);
    }
    return false;
}

bool SimulationHandler::initializeSystem(const double startT, const double stopT, std::vector<ComponentSystem*> &rSystemVector)
{
    //No multicore init support
    bool isOk = true;
    for (size_t i=0; i<rSystemVector.size(); ++i)
    {
        isOk = isOk && initializeSystem(startT, stopT, rSystemVector[i]);
        if (!isOk)
        {
            break;
        }
    }
    return isOk;
}

bool SimulationHandler::simulateSystem(const double startT, const double stopT, const int nDesiredThreads, ComponentSystem* pSystem, bool noChanges)
{
    if (nDesiredThreads < 0)
    {
        pSystem->simulate(stopT);
    }
    else
    {
        pSystem->simulateMultiThreaded(startT, stopT, nDesiredThreads, noChanges);
    }

    return !pSystem->wasSimulationAborted();
}

bool SimulationHandler::simulateSystem(const double startT, const double stopT, const int nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges)
{
    if (rSystemVector.size() > 1)
    {
        if (nDesiredThreads >= 0)
        {
            return simulateMultipleSystemsMultiThreaded(startT, stopT, nDesiredThreads, rSystemVector, noChanges);
        }
        else
        {
            return simulateMultipleSystems(stopT, rSystemVector);
        }
    }
    else if (rSystemVector.size() == 1)
    {
        return simulateSystem(startT, stopT, nDesiredThreads, rSystemVector[0], noChanges);
    }

    return false;
}

void SimulationHandler::finalizeSystem(ComponentSystem* pSystem)
{
    pSystem->finalize();
}

void SimulationHandler::finalizeSystem(std::vector<ComponentSystem*> &rSystemVector)
{
    //No multicore finalize
    for (size_t i=0; i<rSystemVector.size(); ++i)
    {
        finalizeSystem(rSystemVector[i]);
    }
}

void SimulationHandler::runCoSimulation(ComponentSystem *pSystem)
{
#ifdef USEBOOST

    std::vector<std::string> inputComponents;
    std::vector<std::string> inputPorts;
    std::vector<int> inputData;

    std::vector<std::string> outputComponents;
    std::vector<std::string> outputPorts;
    std::vector<int> outputData;

    //////////////////////////

    std::vector<std::string> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        Component *pComponent = pSystem->getSubComponent(names[i]);
        if(pComponent->getTypeName() == "SignalInputInterface")
        {
            inputComponents.push_back(names[i]);
            inputPorts.push_back("out");
            inputData.push_back(0);
        }
        else if(pComponent->getTypeName() == "SignalOutputInterface")
        {
            outputComponents.push_back(names[i]);
            outputPorts.push_back("in");
            outputData.push_back(0);
        }
    }

    //////////////////////////

    std::vector<double*> inputSockets;
    std::vector<double*> outputSockets;

    //Initialize shared memory sockets

    //Simulate
    double *sim_socket = getDoubleSharedMemoryPointer("hopsan_sim");

    //Stop
    bool *stop_socket = getBoolSharedMemoryPointer("hopsan_stop");


    //Input
    for(int i=0; i<inputData.size(); ++i)
    {
        std::stringstream ss;
        ss << "hopsan_in" << i;
        inputSockets.push_back(getDoubleSharedMemoryPointer(ss.str().c_str()));
    }


    //Output
    for(int i=0; i<inputData.size(); ++i)
    {
        std::stringstream ss;
        ss << "hopsan_out" << i;
        outputSockets.push_back(getDoubleSharedMemoryPointer(ss.str().c_str()));
    }

    //Initialize simulation
    //! @todo We must be able to log data without knowing the stop time
    pSystem->initialize(0, 100);

    (*stop_socket) = false;
    (*sim_socket) = 0;

    (*inputSockets[0]) = 34.125;

    std::stringstream ss;
    ss << "Input pointer address = " << inputSockets[0];
    std::string temp = ss.str();


    while(!(*stop_socket))
    {
        if((*sim_socket)>5)
        {
            //Set input variables
            for(int i=0; i<inputSockets.size(); ++i)
            {
                pSystem->getSubComponent(inputComponents[i])->getPort(inputPorts[i])->writeNode(inputData[i], (*inputSockets[i]));
            }

            //Simulate one step
            double time = pSystem->getTime();
            double timestep = pSystem->getDesiredTimeStep();
            pSystem->simulate(time, time+timestep);

            //Write back output variables
            for(int i=0; i<outputSockets.size(); ++i)
            {
                (*outputSockets[i]) = pSystem->getSubComponent(outputComponents[i])->getPort(outputPorts[i])->readNode(outputData[i]);
            }

            //Reset simulation flag
            (*sim_socket) = 0;
        }
    }

#endif
}

//! @brief Distributes component system pointers evenly over one vector per thread, depending on their simulation time
//! @param systemVector Vector to distribute
//! @param nThreads Number of threads to distribute for
vector< vector<ComponentSystem *> > SimulationHandler::distributeSystems(const std::vector<ComponentSystem *> &rSystemVector, size_t nThreads)
{
    vector< vector<ComponentSystem *> > splitSystemVector;
    vector<double> timeVector;

    nThreads = min(nThreads, rSystemVector.size()); //Prevent adding for more threads then systems
    splitSystemVector.resize(nThreads);
    timeVector.resize(nThreads,0);
    size_t sysNum=0;
    while(true)         //! @todo Poor algorithm for distributing, will not give optimal results
    {
        for(size_t t=0; t<nThreads; ++t)
        {
            if(sysNum == rSystemVector.size())
                break;
            splitSystemVector[t].push_back(rSystemVector[sysNum]);
            timeVector[t] += rSystemVector[sysNum]->getMeasuredTime();
            ++sysNum;
        }
        if(sysNum == rSystemVector.size())
            break;
    }
    return splitSystemVector;
}

//Constructor
ComponentSystem::ComponentSystem() : Component(), mAliasHandler(this)
{
    mTypeName = "ComponentSystem";
    mTypeCQS = Component::UndefinedCQSType;
    mName = mTypeName; //Make sure intial name is same as typename
    mDesiredTimestep = 0.001;
    mInheritTimestep = true;
    mKeepStartValues = false;
    mRequestedNumLogSamples = 2048;
#ifdef USETBB
    mpStopMutex = new tbb::mutex();
#else
    mpStopMutex = 0;
#endif

    // Set default (disabled) values for log data
    disableLog();
}

ComponentSystem::~ComponentSystem()
{
    // Clear the contents of the system
    clear();
#ifdef USETBB
    delete mpStopMutex;
#endif
}

void ComponentSystem::configure()
{
    //Does nothing
}

Component::CQSEnumT ComponentSystem::getTypeCQS() const
{
    return mTypeCQS;
}

double ComponentSystem::getDesiredTimeStep() const
{
    return mDesiredTimestep;
}

//! @brief Set the desired number of log samples
void ComponentSystem::setNumLogSamples(const size_t nLogSamples)
{
    mRequestedNumLogSamples = nLogSamples;
}

//! @brief Returns the desired number of log samples
size_t ComponentSystem::getNumLogSamples() const
{
    return mRequestedNumLogSamples;
}

//! @brief Returns the number of actually logged data samples
//! @return Number of availible logged data samples in storage
size_t ComponentSystem::getNumActuallyLoggedSamples() const
{
    // This assumes that the logCtr has been incremented after each saved log step
    return mLogCtr;
}


//! @brief Sets a bool which is looked at in initialization and simulation loops.
//! This method can be used by users e.g. GUIs to stop an start a initialization/simulation process
void ComponentSystem::stopSimulation()
{
#ifdef USETBB
    mpStopMutex->lock();
    mStopSimulation = true;
    mpStopMutex->unlock();
#else
    mStopSimulation = true;
#endif
    // Now propagate stop signal upwards, to parent
    if (mpSystemParent != 0)
    {
        mpSystemParent->stopSimulation();
    }
}

//! @brief Check if the simulation was aborted
//! @returns true if Initialize, Simulate, or Finalize was aborted
bool ComponentSystem::wasSimulationAborted()
{
    return mStopSimulation;
}


//! @brief Adds a search path that can be used by its components to look for external files, e.g. area curves
//! @param searchPath the search path to be added
void ComponentSystem::addSearchPath(const std::string searchPath)
{
    std::string fixedSearchString;
    fixedSearchString = searchPath;
    if (!fixedSearchString.empty())
    {
        while((!fixedSearchString.empty()) && ((*fixedSearchString.rbegin() == '/') || (*fixedSearchString.rbegin() == '\\')))
        {
            fixedSearchString = fixedSearchString.substr (0,fixedSearchString.length()-1);
        }
    }

    bool contain = false;
    for(size_t i=0; i<mSearchPaths.size();++i)
    {
        if(mSearchPaths[i] == fixedSearchString)
            contain = true;
    }
    if(!contain)
        mSearchPaths.push_back(fixedSearchString);
}


//! @todo this one (if it should even exist) should be in component as parameter map is there, best is if we can code around having one
Parameters &ComponentSystem::getSystemParameters()
{
    return *mpParameters;
}

//!
bool ComponentSystem::setSystemParameter(const std::string name, const std::string value, const std::string type, const std::string description, const std::string unit, const bool force)
{
    bool success;
    if(mpParameters->exist(name))
    {
        success = mpParameters->setParameter(name, value, description, unit, type, force);
    }
    else
    {
        if (this->hasReservedUniqueName(name) || !isNameValid(name))
        {
            addErrorMessage(string("The desired system parameter name: ") + name + string(" is invalid or already used by somthing else"));
            success=false;
        }
        else
        {
            success = mpParameters->addParameter(name, value, description, unit, type, false, 0, force);
            if (success)
            {
                reserveUniqueName(name,UniqueSysparamNameType);
            }
        }
    }

    return success;
}

void ComponentSystem::unRegisterParameter(const string name)
{
    Component::unRegisterParameter(name);
    unReserveUniqueName(name);
}

//! @brief Add multiple components to the system
void ComponentSystem::addComponents(std::vector<Component*> &rComponents)
{
    std::vector<Component*>::iterator itx;
    for(itx = rComponents.begin(); itx != rComponents.end(); ++itx)
    {
        addComponent(*itx);
    }
}

//! @brief Add a component to the system
void ComponentSystem::addComponent(Component *pComponent)
{
    // Prevent adding null ptr
    if (pComponent)
    {
        // First check if the name already exists, in that case change the suffix
        string modname = this->reserveUniqueName(pComponent->getName(), UniqueComponentNameType);
        pComponent->setName(modname);

        // Add to the cqs component vectors
        addSubComponentPtrToStorage(pComponent);

        // Set system parent and model system depth hierarcy
        pComponent->setSystemParent(this);
        pComponent->mModelHierarchyDepth = this->mModelHierarchyDepth+1; //Set the ModelHierarchyDepth counter

        // Go thorugh the components ports and take ownership of any dummy nodes
        //! @todo what happens if I take ownership of an other systems components (shouldnt we take ownership of all ports nodes by default) Not sure!! especially difficult with system border nodes
        //! @todo maybe node ownership should be decided early in initialize instead to make this less complicated
        std::vector<Port*> ports = pComponent->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if (ports[i]->getPortType() < MultiportType)
            {
                if (ports[i]->getNodePtr()->getNumConnectedPorts() == 1)
                {
                    this->addSubNode(ports[i]->getNodePtr());
                }
            }
        }
    }
    else
    {
        addErrorMessage("Trying to add NULL component to system");
    }
}


//! @brief Rename a sub component and automatically fix unique names
void ComponentSystem::renameSubComponent(string oldname, string newname)
{
    // First find the post in the map where the old name resides, copy the data stored there
    SubComponentMapT::iterator it = mSubComponentMap.find(oldname);
    Component* temp_comp_ptr;
    if (it != mSubComponentMap.end())
    {
        // If found, erase old record
        temp_comp_ptr = it->second;
        mSubComponentMap.erase(it);

        // Insert new (with new name)
        string mod_new_name = this->reserveUniqueName(newname, UniqueComponentNameType);
        this->unReserveUniqueName(oldname);

        mSubComponentMap.insert(pair<string, Component*>(mod_new_name, temp_comp_ptr));

        // Now change the actual component name, without trying to do rename (we are in rename now, would cause infinite loop)
        temp_comp_ptr->mName = mod_new_name;
    }
    else
    {
        addErrorMessage("No component with old_name: "+oldname+" found when renaming!");
    }
}


//! @brief Remove a dub component from a system, can also be used to actually delete the component
//! @param[in] name The name of the component to remove from the system
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(string name, bool doDelete)
{
    Component* pComp = getSubComponent(name);
    removeSubComponent(pComp, doDelete);
}


//! @brief Remove a sub component from a system, can also be used to actually delete the component
//! @param[in] pComponent A pointer to the component to remove
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(Component* pComponent, bool doDelete)
{
    std::string compName = pComponent->getName();

    // Disconnect all ports before erase from system
    PortPtrMapT::iterator ports_it;
    vector<Port*>::iterator conn_ports_it;
    for (ports_it = pComponent->mPortPtrMap.begin(); ports_it != pComponent->mPortPtrMap.end(); ++ports_it)
    {
        //! @todo what about multiports here
        vector<Port*> connected_ports = ports_it->second->getConnectedPorts(); //Get a copy of the connected ports ptr vector
        //We can not use an iterator directly connected to the vector inside the port as this will be changed by the disconnect calls
        for (conn_ports_it = connected_ports.begin(); conn_ports_it != connected_ports.end(); ++conn_ports_it)
        {
            disconnect(ports_it->second, *conn_ports_it);
        }
    }

    // Remove from storage
    removeSubComponentPtrFromStorage(pComponent);

    // Remove any dummy node ptrs
    //! @todo (shouldnt we remove ownership of all port nodes by default) Not sure!! especially difficult with system border nodes
    std::vector<Port*> ports = pComponent->getPortPtrVector();
    for (size_t i=0; i<ports.size(); ++i)
    {
        if (ports[i]->getPortType() < MultiportType)
        {
            if (ports[i]->getNodePtr()->getNumConnectedPorts() == 1)
            {
                this->removeSubNode(ports[i]->getNodePtr());
            }
        }
    }


    // Shall we also delete the component completely
    if (doDelete)
    {
        mpHopsanEssentials->removeComponent(pComponent);
    }

    // Unreserve the name
    unReserveUniqueName(compName);

    addDebugMessage("Removed component: \"" + compName + "\" from system: \"" + this->getName() + "\"", "removedcomponent");
}

//! @brief Reserves a unique name in the system
//! @param [in] desiredName The desired name to reserve
//! @returns The actual name reserved
string ComponentSystem::reserveUniqueName(const string desiredName, const UniqeNameEnumT type)
{
    string newname = this->determineUniqueComponentName(desiredName);
    mTakenNames.insert(std::pair<std::string, UniqeNameEnumT>(newname, type));
    return newname;
}

//! @brief unReserves a unique name in the system
//! @param [in] name The name to unreserve
void ComponentSystem::unReserveUniqueName(const string name)
{
    mTakenNames.erase(name);
}

void ComponentSystem::addSubComponentPtrToStorage(Component* pComponent)
{
    switch (pComponent->getTypeCQS())
    {
    case Component::CType :
        mComponentCptrs.push_back(pComponent);
        break;
    case Component::QType :
        mComponentQptrs.push_back(pComponent);
        break;
    case Component::SType :
        mComponentSignalptrs.push_back(pComponent);
        break;
    case Component::UndefinedCQSType :
        mComponentUndefinedptrs.push_back(pComponent);
        break;
    default :
        addErrorMessage("Trying to add module with unspecified CQS type: "+pComponent->getTypeCQSString()+", (Not added)");
        return;
    }

    mSubComponentMap.insert(pair<string, Component*>(pComponent->getName(), pComponent));
}

void ComponentSystem::removeSubComponentPtrFromStorage(Component* pComponent)
{
    SubComponentMapT::iterator it = mSubComponentMap.find(pComponent->getName());
    if (it != mSubComponentMap.end())
    {
        vector<Component*>::iterator cit; //Component iterator
        switch (it->second->getTypeCQS())
        {
        case Component::CType :
            for (cit = mComponentCptrs.begin(); cit != mComponentCptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentCptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::QType :
            for (cit = mComponentQptrs.begin(); cit != mComponentQptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentQptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::SType :
            for (cit = mComponentSignalptrs.begin(); cit != mComponentSignalptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentSignalptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::UndefinedCQSType :
            for (cit = mComponentUndefinedptrs.begin(); cit != mComponentUndefinedptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentUndefinedptrs.erase(cit);
                    break;
                }
            }
            break;
        default :
            addFatalMessage("In removeSubComponentPtrFromStorage(): Component is not of CType, QType, SType or UndefinedCQSType.");
        }
        mSubComponentMap.erase(it);
    }
    else
    {
        addErrorMessage("The component you are trying to remove: "+pComponent->getName()+" does not exist (Does Nothing)");
    }
}

//! @brief Clear all the contents of a system (deleting any remaning components and connections)
void ComponentSystem::clear()
{
    // Remove and delete every subcomponent, one by one
    while (!mSubComponentMap.empty())
    {
        removeSubComponent((*mSubComponentMap.begin()).second, true);
    }
}


//! @brief Sorts a component vector
//! Components are sorted so that they are always simulated after the components they receive signals from. Algebraic loops can be detected, in that case this function does nothing.
bool ComponentSystem::sortComponentVector(std::vector<Component*> &rComponentVector)
{
    std::vector<Component*> newComponentVector;

    bool didSomething = true;
    while(didSomething)
    {
        didSomething = false;
        std::vector<Component*>::iterator it;
        for(it=rComponentVector.begin(); it!=rComponentVector.end(); ++it)  //Loop through the unsorted signal component vector
        {
            if(!componentVectorContains(newComponentVector, (*it)))    //Ignore components that are already added to the new vector
            {
                bool readyToAdd=true;
                std::vector<Port*>::iterator itp;
                std::vector<Port*> portVector = (*it)->getPortPtrVector();
                for(itp=portVector.begin(); itp!=portVector.end(); ++itp) //Ask each port for its node, then ask the node for its write port component
                {
                    Component* requiredComponent=0;
                    if(((*itp)->getPortType() == ReadPortType || (*itp)->getPortType() == ReadMultiportType) && (*itp)->isConnected())
                    {
                        requiredComponent = (*itp)->getNodePtr()->getWritePortComponentPtr();
                    }
                    if(requiredComponent != 0 && requiredComponent->getTypeName() != "SignalUnitDelay")
                    {
                        if(requiredComponent->mpSystemParent == this)
                        {
                            if(!componentVectorContains(newComponentVector, requiredComponent) &&
                               componentVectorContains(rComponentVector, requiredComponent)
                               /*requiredComponent->getTypeCQS() == (*itp)->getComponent()->getTypeCQS()*//*Component::S*/)
                            {
                                readyToAdd = false;     //Depending on normal component which has not yet been added
                            }
                        }
                        else
                        {
                            if(!componentVectorContains(newComponentVector, requiredComponent->mpSystemParent) &&
                               requiredComponent->mpSystemParent->getTypeCQS() == (*itp)->getComponent()->getTypeCQS() &&
                               componentVectorContains(rComponentVector,requiredComponent->mpSystemParent))
                            {
                                readyToAdd = false;     //Depending on subsystem component which has not yer been added
                            }
                        }
                    }
                }
                if(readyToAdd)  //Add the component if all required write port components was already added
                {
                    newComponentVector.push_back((*it));
                    didSomething = true;
                }
            }
        }
    }

    if(newComponentVector.size() == rComponentVector.size())   //All components moved to new vector = success!
    {
        rComponentVector = newComponentVector;
        if(newComponentVector.size() > 0 && newComponentVector[0]->getTypeCQS() == SType)
        {
            stringstream ss;
            std::vector<Component*>::iterator it;
            for(it=newComponentVector.begin(); it!=newComponentVector.end(); ++it)
                ss << (*it)->getName() << "\n";                                                                                               //DEBUG
            addDebugMessage("Sorted components successfully!\nSignal components will be simulated in the following order:\n" + ss.str());
        }
    }
    else    //Something went wrong, all components were not moved. This is likely due to an algebraic loop.
    {
        addErrorMessage("Initialize: Algebraic loops was found, signal components could not be sorted.");
        if(!newComponentVector.empty())
            addInfoMessage("Last component that was successfully sorted: " + newComponentVector.back()->getName());
        addInfoMessage("Initialize: Hint: Use unit delay components to resolve loops.");
        return false;
    }

    return true;
}


//! @brief Figures out whether or not a component vector contains a certain component
bool ComponentSystem::componentVectorContains(std::vector<Component*> vector, Component *pComp)
{
    std::vector<Component*>::iterator it;
    for(it=vector.begin(); it!=vector.end(); ++it)
    {
        if((*it) == pComp)
        {
            return true;
        }
    }
    return false;
}


//! @brief Overloaded function that behaves slightly different when determining unique port names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports dont have the same name as a subcomponent
std::string ComponentSystem::determineUniquePortName(std::string portname)
{
    return this->reserveUniqueName(portname, UniqueSysportNameTyp);
}

//! @brief Overloaded function that behaves slightly different when determining unique component names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports dont have the same name as a subcomponent
//! @todo the determineUniquePortNAme and ComponentName looks VERY similar maybe we could use the same function for both
std::string ComponentSystem::determineUniqueComponentName(std::string name)
{
    return findUniqueName<TakenNamesMapT>(mTakenNames, name);
}

bool ComponentSystem::hasReservedUniqueName(const string &rName) const
{
    return (mTakenNames.find(rName) != mTakenNames.end());
}


//! @brief Get a Component ptr to the component with supplied name, can also return a ptr to self if no subcomponent found but systemport with name found
//! @details For this to work we need to make sure that the sub components and systemports have unique names
Component* ComponentSystem::getSubComponentOrThisIfSysPort(string name)
{
    // First try to find among subcomponents
    Component *tmp = getSubComponent(name);
    if (tmp == 0)
    {
        // Now try to find among systemports
        Port* pPort = this->getPort(name);
        if (pPort != 0)
        {
            if (pPort->getPortType() == SystemPortType)
            {
                // Return the systemports owner (the system component)
                tmp = pPort->getComponent();
            }
        }
    }
    return tmp;
}


Component* ComponentSystem::getSubComponent(string name)
{
    SubComponentMapT::iterator it = mSubComponentMap.find(name);
    if (it != mSubComponentMap.end())
    {
        return it->second;
    }
    else
    {
        addLogMess("ComponentSystem::getSubComponent(): The requested component does not exist.");
        return 0;
    }
}


ComponentSystem* ComponentSystem::getSubComponentSystem(string name)
{
    return dynamic_cast<ComponentSystem*>(getSubComponent(name));
}


vector<string> ComponentSystem::getSubComponentNames()
{
    //! @todo for now create a vector of the component names, later maybe we should return a pointer to the real internal map
    vector<string> names;
    SubComponentMapT::iterator it;
    for (it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
    {
        names.push_back(it->first);
    }

    return names;
}

//! @brief Check if a system has a subcomponent with given name
//! @param name The name to check for
//! @returns true or false
bool ComponentSystem::haveSubComponent(const string name) const
{
    return (mSubComponentMap.count(name) > 0);
}

//! @brief Checks if a system is empty (if there are no components or systemports)
bool ComponentSystem::isEmpty() const
{
    return ((mSubComponentMap.size() + mPortPtrMap.size()) == 0);
}

AliasHandler &ComponentSystem::getAliasHandler()
{
    return mAliasHandler;
}


//! @brief Add a node as subnode in the system, if the node is already owned by someone else, trasfere owneship to this system
void ComponentSystem::addSubNode(Node* pNode)
{
    if (pNode->getOwnerSystem() != 0)
    {
        pNode->getOwnerSystem()->removeSubNode(pNode);
    }
    mSubNodePtrs.push_back(pNode);
    pNode->mpOwnerSystem = this;
}


//! @brief Removes a previously added node
void ComponentSystem::removeSubNode(Node* pNode)
{
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        if (*it == pNode)
        {
            pNode->mpOwnerSystem = 0;
            mSubNodePtrs.erase(it);
            break;
        }
    }
}


//! @brief preAllocates log space (to speed up later access for log writing)
void ComponentSystem::preAllocateLogSpace(const double startT, const double stopT, const size_t nSamples)
{
    bool success = true;
//    //cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;
//    this->setLogSettingsNSamples(nSamples, startT, stopT, mTimestep);
    //! @todo Fixa /Peter
    mLogCtr = 0;
    if (mEnableLogData)
    {
        try
        {
            mTimeStorage.resize(mnLogSlots, 0);

            // Allocate log data memory for subnodes
            //! @todo we should have an other vector with those nodes that should be logged, if we make individual nodes possible to disable logging
            vector<Node*>::iterator it;
            for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
            {
                // Abort if we are told to stop or if memmory allocation fails
                if (mStopSimulation || !success)
                    break;

                // Prepare the node log data allocation and determine if loggings should be on
                //! @todo What if we want to use one of the other ways of setting logsample time steps

                // Now try to allocate log memmory for each node
                try
                {
                    // If the node is in a read port and if that port is not connected (node only have one connected port)
                    // Then we should disable logging for that node as loging the startvalue does not make sense
                    if ( ((*it)->getNumConnectedPorts() < 2) && ((*it)->getNumberOfPortsByType(ReadPortType) == 1) )
                    {
                        (*it)->disableLog();
                    }
                    else
                    {
                        (*it)->enableLog();
                        (*it)->preAllocateLogSpace(mnLogSlots);
                    }
                    success = true;
                }
                catch (exception &e)
                {
                    //cout << "preAllocateLogSpace: Standard exception: " << e.what() << endl;
                    addErrorMessage("Failed to allocate log data memmory, try reducing the amount of log data", "FailedMemmoryAllocation");
                    (*it)->disableLog();
                    success = false;
                }
            }
        }
        catch (exception &e)
        {
            addErrorMessage("Failed to allocate log data memmory, try reducing the amount of log data", "FailedMemmoryAllocation");
            disableLog();
            success = false;
        }
    }

    // If we faild to allocate log memory then stop simulation
    if (!success)
    {
        mStopSimulation = true;
    }
}


void ComponentSystem::logTimeAndNodes(const size_t simStep)
{
    if (mEnableLogData)
    {
        if (mLogTheseTimeSteps[mLogCtr] ==  simStep)
        {
            mTimeStorage[mLogCtr] = mTime;   //We log the "real"  simulation time for the sample

            //! @todo we should have an other vector with those nodes that should be logged, if we make individual nodes possible to disable logging
            vector<Node*>::iterator it;
            for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
            {
                (*it)->logData(mLogCtr);
            }
            ++mLogCtr;
        }
    }
}

//! @brief Rename a system parameter
bool ComponentSystem::renameParameter(const std::string oldName, const std::string newName)
{
    if (hasReservedUniqueName(newName))
    {
        addErrorMessage(string("The desired system parameter name: ") + newName + string(" is already used"));
    }
    else if (mpParameters->renameParameter(oldName, newName))
    {
        unReserveUniqueName(oldName);
        reserveUniqueName(newName);
        return true;
    }
    return false;
}

//! @brief Adds a transparent SubSystemPort
Port* ComponentSystem::addSystemPort(string portName)
{
    // Force default portname p, if nothing else specified
    if (portName.empty())
    {
        portName = "p";
    }

    return addPort(portName, SystemPortType, "NodeEmpty", Port::Required);
}


//! @brief Rename system port
string ComponentSystem::renameSystemPort(const string oldname, const string newname)
{
    string newmodename = renamePort(oldname,newname);
    if (newmodename != oldname)
    {
        unReserveUniqueName(oldname);
    }
    return newmodename;
}


//! @brief Delete a System port from the component
//! @param [in] name The name of the port to delete
void ComponentSystem::deleteSystemPort(const string name)
{
    deletePort(name);
    unReserveUniqueName(name);
}


//! @brief Set the type C, Q, or S of the subsystem
void ComponentSystem::setTypeCQS(CQSEnumT cqs_type, bool doOnlyLocalSet)
{
    //! @todo should really try to figure out a better way to do this
    //! @todo need to do erro checking, and make sure that the specified type really is valid, first and last component should be of this type (i think)

    //If type same as before do nothing
    if (cqs_type !=  mTypeCQS)
    {
        //Do we have a system parent
        if ( !this->isTopLevelSystem() && !doOnlyLocalSet )
        {
            //Request change by our parent (som parent changes are neeeded)
            mpSystemParent->changeSubComponentSystemTypeCQS(mName, cqs_type);
        }
        else
        {
            switch (cqs_type)
            {
            case Component::CType :
                mTypeCQS = Component::CType;
                break;

            case Component::QType :
                mTypeCQS = Component::QType;
                break;

            case Component::SType :
                mTypeCQS = Component::SType;
                break;

            case Component::UndefinedCQSType :
                mTypeCQS = Component::UndefinedCQSType;
                break;

            default :
                addWarningMessage("Specified type: "+getTypeCQSString()+" does not exist!, System CQStype unchanged");
            }
        }
    }
}

//! @brief Change the cqs type of a stored subsystem component
bool ComponentSystem::changeSubComponentSystemTypeCQS(const string name, const CQSEnumT newType)
{
    //First get the componentsystem ptr and check if we are requesting new type
    ComponentSystem* tmpptr = getSubComponentSystem(name);
    if (tmpptr != 0)
    {
        // If the ptr was not = 0 then we have found a subsystem, lets change the type
        if (newType != tmpptr->getTypeCQS())
        {
            //Remove old version
            this->removeSubComponentPtrFromStorage(tmpptr);

            //Change cqsType localy in the subcomponent, make sure to set true to avoid looping back to this rename
            tmpptr->setTypeCQS(newType, true);

            //readd to system
            this->addSubComponentPtrToStorage(tmpptr);
        }
        return true;
    }
    return false;
}

//! @brief This function automatically determines the CQS type depending on the what has been connected to the systemports
//! @todo This function will go through all conected ports every time it is run, maybe a quicker version would only be run on the port beeing connected or disconnectd, in the connect and disconnect function
void ComponentSystem::determineCQSType()
{
    size_t c_ctr=0;
    size_t q_ctr=0;
    size_t s_ctr=0;

    PortPtrMapT::iterator ppmit;
    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        //all ports should be system ports in a subsystem, dont check for other port types
        if( ppmit->second->getPortType() == SystemPortType )
        {
            //! @todo I dont think that I really need to ask for ALL connected subports here, as it is actually only the component that is directly connected to the system port that is interesting
            //! @todo this means that I will be able to UNDO the Port getConnectedPorts madness, maybe, if we dont want it in some other place
            vector<Port*> connectedPorts = (*ppmit).second->getConnectedPorts(-1); //Make a copy of connected ports
            vector<Port*>::iterator cpit;
            for (cpit=connectedPorts.begin(); cpit!=connectedPorts.end(); ++cpit)
            {
                if ( (*cpit)->getComponent()->getSystemParent() == this )
                {
                    if( (*cpit)->getPortType() == ReadPortType || (*cpit)->getPortType() == WritePortType)
                    {
                        ++s_ctr;
                        continue;
                    }

                    switch ((*cpit)->getComponent()->getTypeCQS())
                    {
                    case CType :
                        ++c_ctr;
                        break;
                    case QType :
                        ++q_ctr;
                        break;
                    case SType :
                        ++s_ctr;
                        break;
                    default :
                        ;
                        //Do nothing, (connecting a port from a system with no cqs type set yet)
                    }
                }
            }
        }
    }

    //Ok now lets determine if we have a valid CQS type or not
    if ( (c_ctr > 0) && (q_ctr == 0) )
    {
        this->setTypeCQS(CType);
    }
    else if ( (q_ctr > 0) && (c_ctr == 0) )
    {
        this->setTypeCQS(QType);
    }
    else if ( (s_ctr > 0) && (c_ctr==0) && (q_ctr==0) )
    {
        this->setTypeCQS(SType);
    }
    else
    {
//        //If we swap from valid type then give warning
//        if (this->getTypeCQS() != UNDEFINEDCQSTYPE)
//        {
//            addWarningMessage(string("Your action has caused the CQS type to become invalid in system: ")+this->getName(), "invalidcqstype");
//        }
        this->setTypeCQS(UndefinedCQSType);
    }
}

bool ComponentSystem::isTopLevelSystem() const
{
    return (mpSystemParent==0);
}


//! @brief Connect two commponents, string version
//! @param [in] compname1 The name of the first component
//! @param [in] portname1 The name of the port on the first component
//! @param [in] compname2 The name of the second component
//! @param [in] portname2 The name of the port on the second component
//! @returns True if success else False
bool ComponentSystem::connect(const string compname1, const string portname1, const string compname2, const string portname2)
{
    // Check if the components exist (and can be found)
    Component* pComp1 = getSubComponentOrThisIfSysPort(compname1);
    Component* pComp2 = getSubComponentOrThisIfSysPort(compname2);

    if (pComp1 == 0)
    {
        addErrorMessage("Component1: '"+compname1+"' can not be found when attempting connect", "connectwithoutcomponent");
        return false;
    }

    if (pComp2 == 0)
    {
        addErrorMessage("Component2: '"+compname2+"' can not be found when attempting connect", "connectwithoutcomponent");
        return false;
    }

    // Check if commponents have specified ports
    Port *pPort1, *pPort2;
    if (!pComp1->getPort(portname1, pPort1))
    {
        addErrorMessage("Component: '"+pComp1->getName()+"' does not have a port named '"+portname1+"'", "portdoesnotexist");
        return false;
    }

    if (!pComp2->getPort(portname2, pPort2)) //Not else if because pPort2 has to be set in getPort
    {
        addErrorMessage("Component: '"+pComp2->getName()+"' does not have a port named '"+portname2+"'", "portdoesnotexist");
        return false;
    }

    // Ok components and ports exist, lets atempt the connect
    return connect( pPort1, pPort2 );
}


bool ConnectionAssistant::ensureSameNodeType(Port *pPort1, Port *pPort2)
{
    // Check if both ports have the same node type specified
    if (pPort1->getNodeType() != pPort2->getNodeType())
    {
        stringstream ss;
        ss << "You can not connect a {" << pPort1->getNodeType() << "} port to a {" << pPort2->getNodeType()  << "} port." <<
              " When connecting: {" << pPort1->getComponent()->getName() << "::" << pPort1->getName() << "} to {" << pPort2->getComponent()->getName() << "::" << pPort2->getName() << "}";
        mpComponentSystem->addErrorMessage(ss.str());
        return false;
    }
    return true;
}

//! @note requires that input ports are not multiports (they can be subports in multiports)
bool ConnectionAssistant::mergeNodeConnection(Port *pPort1, Port *pPort2)
{
    if (!ensureSameNodeType(pPort1, pPort2))
    {
        return false;
    }

    Node *pOldNode1 = pPort1->getNodePtr();
    Node *pOldNode2 = pPort2->getNodePtr();

    // Check for very rare occurance, (Looping a subsystem, and connecting an out port to an in port that are actually directly connected to each other)
    if (pOldNode1 == pOldNode2)
    {
        mpComponentSystem->addErrorMessage("This connection would mean that a node is joined with it self, this does not make any sense and is not allowed");
        return false;
    }

    // Create a new node and recursively set in all ports
    Node *pNewNode = mpComponentSystem->getHopsanEssentials()->createNode(pPort1->getNodeType());
    recursivelySetNode(pPort1, 0, pNewNode);
    recursivelySetNode(pPort2, 0, pNewNode);

    // Let the ports know about each other
    pPort1->addConnectedPort(pPort2);
    pPort2->addConnectedPort(pPort1);

    // Now delete the old nodes
    removeNode(pOldNode1);
    removeNode(pOldNode2);

    // Update the node placement
    determineWhereToStoreNodeAndStoreIt(pNewNode);

    if (ensureConnectionOK(pNewNode, pPort1, pPort2))
    {
        return true;
    }
    else
    {
        splitNodeConnection(pPort1, pPort2); //Undo connection
        return false;
    }
}

void ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(Node* pNode)
{
    //node ptr should not be zero
    if(pNode == 0)
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(): Node pointer is zero.");
        return;
    }

    vector<Port*>::iterator pit;
    Component *pMinLevelComp=0;
    //size_t min = std::numeric_limits<size_t>::max();
    size_t min = (size_t)-1;
    for (pit=pNode->mConnectedPorts.begin(); pit!=pNode->mConnectedPorts.end(); ++pit)
    {
        if ((*pit)->getComponent()->getModelHierarchyDepth() < min)
        {
            min = (*pit)->getComponent()->getModelHierarchyDepth();
            pMinLevelComp = (*pit)->getComponent();
        }
    }

    //Now add the node at the minimum level, if minimum is a system (we are connecting to our system parant) then dyncast the pointer
    //! @todo what if we are connecting only subsystems within the same lavel AND they have different timesteps
    if (pMinLevelComp==0)
    {
        mpComponentSystem->addSubNode(pNode);
    }
    else if (pMinLevelComp->isComponentSystem())
    {
        ComponentSystem *pRootSys = dynamic_cast<ComponentSystem*>(pMinLevelComp);
        pRootSys->addSubNode(pNode);
    }
    else
    {
        pMinLevelComp->getSystemParent()->addSubNode(pNode);
    }
}

void ConnectionAssistant::recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode)
{
    pPort->setNode(pNode);
    vector<Port*>::iterator pit;
    for (pit=pPort->getConnectedPorts().begin(); pit!=pPort->getConnectedPorts().end(); ++pit)
    {
        //dont recures back to parent will get stuck in infinate recursion
        if (*pit == pParentPort)
        {
            continue;
        }
        recursivelySetNode(*pit, pPort, pNode);
    }
}

Port* ConnectionAssistant::findMultiportSubportFromOtherPort(const Port *pMultiPort, Port *pOtherPort)
{
    if(pOtherPort->getPortType() >= MultiportType)
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::findMultiportSubportFromOtherPort(): Other port shall not be a multiport.");
        return 0;
    }

    std::vector<Port*> otherConnPorts = pOtherPort->getConnectedPorts();
    for (size_t i=0; i<otherConnPorts.size(); ++i)
    {
        // We assume that a port can not be connected multiple times to the same multiport
        if (otherConnPorts[i]->mpParentPort == pMultiPort)
        {
            return otherConnPorts[i];
        }
    }
    return 0;
}


//! @note Requires that the input ports are not multiports
bool ConnectionAssistant::splitNodeConnection(Port *pPort1, Port *pPort2)
{
    if ((pPort1==0) || (pPort2==0))
    {
        mpComponentSystem->addFatalMessage("splitNodeConnection(): One of the ports is NULL");
        return false;
    }

    Node *pOldNode = pPort1->getNodePtr();
    Node *pNewNode1 = mpComponentSystem->getHopsanEssentials()->createNode(pOldNode->getNodeType());
    Node *pNewNode2 = mpComponentSystem->getHopsanEssentials()->createNode(pOldNode->getNodeType());

    // Make the ports forget about each other, If the ports becomes empty the nodes will be reset
    pPort1->eraseConnectedPort(pPort2);
    pPort2->eraseConnectedPort(pPort1);

    // Recursievly set new nodes
    recursivelySetNode(pPort1, 0, pNewNode1);
    recursivelySetNode(pPort2, 0, pNewNode2);

    // Remove the old node
    removeNode(pOldNode);

    // Now determine what system should own the node
    determineWhereToStoreNodeAndStoreIt(pNewNode1);
    determineWhereToStoreNodeAndStoreIt(pNewNode2);

    return true;
}


ConnectionAssistant::ConnectionAssistant(ComponentSystem *pComponentSystem)
{
    mpComponentSystem = pComponentSystem;
}

//! Helpfunction that clears the nodetype in empty systemports, It will not clear the type if the port is not empty or if the port is not a systemport
void ConnectionAssistant::clearSysPortNodeTypeIfEmpty(Port *pPort)
{
    if ( (pPort->getPortType() == SystemPortType) && (!pPort->isConnected()) )
    {
        removeNode(pPort->getNodePtr());
        pPort->setNode(mpComponentSystem->getHopsanEssentials()->createNode("NodeEmpty"));
        pPort->mNodeType = "NodeEmpty";
    }
}

//! @brief Connect two components with specified ports to each other
//! @param [in] pPort1 A pointer to the first port
//! @param [in] pPort2 A pointer to the second port
//! @returns True if success, False if failed
bool ComponentSystem::connect(Port *pPort1, Port *pPort2)
{
    if ((pPort1==0) || (pPort2==0))
    {
        addErrorMessage("Trying to connect NULL port(s)", "nullport");
        return false;
    }

    // Prevent connection with self
    if (pPort1 == pPort2)
    {
        addErrorMessage("You can not connect a port to it self", "selfconnection");
        return false;
    }

    ConnectionAssistant connAssist(this);
    Component* pComp1 = pPort1->getComponent();
    Component* pComp2 = pPort2->getComponent();
    bool sucess=false;

    // Prevent connection between two multiports
    //! @todo we might want to allow this in the future, right now disconnecting two multiports is also not implemented
    if ( pPort1->isMultiPort() && pPort2->isMultiPort() )
    {
        addErrorMessage("You are not allowed to connect two MultiPorts to each other, (this may be allowed in the future)");
        return false;
    }


    // Prevent connection if ports are already connected to each other
    //! @todo What will happend with multiports
    if (pPort1->isConnectedTo(pPort2))
    {
        addErrorMessage("These two ports are already connected to each other", "allreadyconnected");
        return false;
    }

    // Preven crossconnection between systems
    if (!connAssist.ensureNotCrossConnecting(pPort1, pPort2))
    {
        addErrorMessage("You can not cross-connect between systems", "crossconnection");
        return false;
    }

    // Prevent connection of two blank systemports
    if ( (pPort1->getPortType() == SystemPortType) && (pPort2->getPortType() == SystemPortType) )
    {
        if ( (!pPort1->isConnected()) && (!pPort2->isConnected()) )
        {
            addErrorMessage("You are not allowed to connect two blank systemports to each other");
            return false;
        }
    }

    // Prevent connection of readport to multiport, (what do you really want to read problem)
    if (pPort1->isMultiPort() || pPort2->isMultiPort())
    {
        if ( (pPort1->getPortType() == ReadPortType) || (pPort2->getPortType() == ReadPortType) )
        {
            addErrorMessage("You are not allowed to connect a readport to a multiport, (undefined what you will actually read). Connect to an ordinary port instead.");
            return false;
        }
    }

    // Now lets find out if one of the ports is a blank systemport
    //! @todo better way to find out if systemports are blank might give more clear code
    if ( ( (pPort1->getPortType() == SystemPortType) && (!pPort1->isConnected()) ) || ( (pPort2->getPortType() == SystemPortType) && (!pPort2->isConnected()) ) )
    {
        // Now lets find out wich of the ports that is a blank systemport
        Port *pBlankSysPort;
        Port *pOtherPort;

        //! @todo write help function
        if ( (pPort1->getPortType() == SystemPortType) && (!pPort1->isConnected()) )
        {
            pBlankSysPort = pPort1;
            pOtherPort = pPort2;
        }
        else if ( (pPort2->getPortType() == SystemPortType) && (!pPort2->isConnected()) )
        {
            pBlankSysPort = pPort2;
            pOtherPort = pPort1;
        }

        pBlankSysPort->mNodeType = pOtherPort->getNodeType(); //set the nodetype in the sysport

        // Check if we are connecting multiports, in that case add new subport, remember original portPointer though so that we can clean up if failure
        Port *pActualPort = connAssist.ifMultiportAddSubport(pOtherPort);

        sucess = connAssist.mergeNodeConnection(pBlankSysPort, pActualPort);

        // Handle multiport connection sucess or failure
        connAssist.ifMultiportCleanupAfterConnect(pOtherPort, pActualPort, sucess);
    }
    // Non of the ports  are blank systemports
    else
    {
        // Check if we are connecting multiports, in that case add new subport, remember original portPointer though so that we can clean up if failure
        Port *pActualPort1 = connAssist.ifMultiportAddSubport(pPort1);
        Port *pActualPort2 = connAssist.ifMultiportAddSubport(pPort2);

        sucess = connAssist.mergeNodeConnection(pActualPort1, pActualPort2);

        // Handle multiport connection sucess or failure
        connAssist.ifMultiportCleanupAfterConnect(pPort1, pActualPort1, sucess);
        connAssist.ifMultiportCleanupAfterConnect(pPort2, pActualPort2, sucess);
    }

    // Abort connection if there was a connect failure
    if (!sucess)
    {
        return false;
    }

    // Update the CQS type
    this->determineCQSType();

    // Update parent cqs-type
    //! @todo we should only do this if we are actually connected directly to our parent, but I dont know what will take the most time, to ckeach if we are connected to parent or to just allways refresh parent
    if (!this->isTopLevelSystem())
    {
        this->mpSystemParent->determineCQSType();
    }

    addDebugMessage("Connected: {"+pComp1->getName()+"::"+pPort1->getName()+"} and {"+pComp2->getName()+"::"+pPort2->getName()+"}", "succesfulconnect");
    return true;
}



bool ConnectionAssistant::ensureConnectionOK(Node *pNode, Port *pPort1, Port *pPort2)
{
    size_t n_ReadPorts = 0;
    size_t n_WritePorts = 0;
    size_t n_PowerPorts = 0;
    size_t n_SystemPorts = 0;
    size_t n_OwnSystemPorts = 0; //Number of systemports that belong to the connecting system
    //size_t n_MultiPorts = 0;

    size_t n_Ccomponents = 0;
    size_t n_Qcomponents = 0;
    size_t n_SYScomponentCs = 0;
    size_t n_SYScomponentQs = 0;

    //Count the different kind of ports and C,Q components in the node
    vector<Port*>::iterator it;
    for (it=(*pNode).mConnectedPorts.begin(); it!=(*pNode).mConnectedPorts.end(); ++it)
    {
        if ((*it)->getPortType() == ReadPortType)
        {
            n_ReadPorts += 1;
        }
        else if ((*it)->getPortType() == WritePortType)
        {
            n_WritePorts += 1;
        }
        else if ((*it)->getPortType() == PowerPortType)
        {
            n_PowerPorts += 1;
        }
        else if ((*it)->getPortType() == SystemPortType)
        {
            n_SystemPorts += 1;
            if ((*it)->getComponent() == mpComponentSystem)
            {
                n_OwnSystemPorts += 1;
            }
        }
//        else if((*it)->getPortType() > MULTIPORT)
//        {
//            n_MultiPorts += 1;
//        }
        if ((*it)->getComponent()->isComponentC())
        {
            n_Ccomponents += 1;
            if ((*it)->getComponent()->isComponentSystem())
            {
                n_SYScomponentCs += 1;
            }
        }
        else if ((*it)->getComponent()->isComponentQ())
        {
            n_Qcomponents += 1;
            if ((*it)->getComponent()->isComponentSystem())
            {
                n_SYScomponentQs += 1;
            }
        }
    }

    //Check the kind of ports in the components subjected for connection
    //Dont count port if it is already conected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort1) )
    {
        if ( pPort1->getPortType() == ReadPortType )
        {
            n_ReadPorts += 1;
        }
        if ( pPort1->getPortType() == WritePortType )
        {
            n_WritePorts += 1;
        }
        if ( pPort1->getPortType() == PowerPortType )
        {
            n_PowerPorts += 1;
        }
        if ( pPort1->getPortType() == SystemPortType )
        {
            n_SystemPorts += 1;
        }
//        if( pPort1->getPortType() > MULTIPORT)
//        {
//            n_MultiPorts += 1;
//        }
        if ( pPort1->getComponent()->isComponentC() )
        {
            n_Ccomponents += 1;
            if ( pPort1->getComponent()->isComponentSystem() )
            {
                n_SYScomponentCs += 1;
            }
        }
        if ( pPort1->getComponent()->isComponentQ() )
        {
            n_Qcomponents += 1;
            if ( pPort1->getComponent()->isComponentSystem() )
            {
                n_SYScomponentQs += 1;
            }
        }
    }

    //Dont count port if it is already conected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort2) )
    {
        if ( pPort2->getPortType() == ReadPortType )
        {
            n_ReadPorts += 1;
        }
        if ( pPort2->getPortType() == WritePortType )
        {
            n_WritePorts += 1;
        }
        if ( pPort2->getPortType() == PowerPortType )
        {
            n_PowerPorts += 1;
        }
        if ( pPort2->getPortType() == SystemPortType )
        {
            n_SystemPorts += 1;
        }
        if ( pPort2->getComponent()->isComponentC() )
        {
            n_Ccomponents += 1;
            if ( pPort2->getComponent()->isComponentSystem() )
            {
                n_SYScomponentCs += 1;
            }
        }
        if ( pPort2->getComponent()->isComponentQ() )
        {
            n_Qcomponents += 1;
            if ( pPort2->getComponent()->isComponentSystem() )
            {
                n_SYScomponentQs += 1;
            }
        }
    }

    //  Check if there are some problems with the connection

    if ((n_PowerPorts > 0) && (n_OwnSystemPorts > 1))
    {
        mpComponentSystem->addErrorMessage("Trying to connect one powerport to two systemports, this is not allowed");
        return false;
    }
//    if(n_MultiPorts > 1)
//    {
//        addErrorMessage("Trying to connect two MultiPorts to each other");
//        return false;
//    }
    if (n_PowerPorts > 2)
    {
        mpComponentSystem->addErrorMessage("Trying to connect more than two PowerPorts to same node");
        return false;
    }
    if (n_WritePorts > 1)
    {
        mpComponentSystem->addErrorMessage("Trying to connect more than one WritePort to same node");
        return false;
    }
    if ((n_PowerPorts > 0) && (n_WritePorts > 0))
    {
        mpComponentSystem->addErrorMessage("Trying to connect WritePort and PowerPort to same node");
        return false;
    }
    if ((n_PowerPorts == 0) && (n_WritePorts == 0) && (n_SystemPorts == 0))
    {
        cout << "Trying to connect only ReadPorts" << endl;
        mpComponentSystem->addErrorMessage("Trying to connect only ReadPorts");
        return false;
    }

    //cout << "nQ: " << n_Qcomponents << " nC: " << n_Ccomponents << endl;

    // Normaly we want at most one c and one q component but if there happen to be a subsystem in the picture allow one extra
    // This is only true if at least one powerport is connected - signal connecetions can be between any types of components
    //! @todo not 100% sure that this will work allways. Only work if we assume that the subsystem has the correct cqs type when connecting
    if ((n_Ccomponents > 1+n_SYScomponentCs) && (n_PowerPorts > 0))
    {
        mpComponentSystem->addErrorMessage("You can not connect two C-Component power ports to each other");
        return false;
    }
    if ((n_Qcomponents > 1+n_SYScomponentQs) && (n_PowerPorts > 0))
    {
        mpComponentSystem->addErrorMessage("You can not connect two Q-Component power ports to each other");
        return false;
    }
//    if ((pPort1->getPortType() == Port::READPORT) &&  (pPort2->getPortType() == Port::READPORT))
//    {
//        addErrorMessage("Trying to connect ReadPort to ReadPort");
//        return false;
//    }
//    if( ((pPort1->getPortType() == Port::READPORT) && pPort2->getPortType() == Port::POWERPORT && n_PowerPorts > 1) or
//        ((pPort2->getPortType() == Port::READPORT) && pPort1->getPortType() == Port::POWERPORT && n_PowerPorts > 1) )
//    {
//        addErrorMessage("Trying to connect one ReadPort to more than one PowerPort");
//        return false;
//    }

    // It seems to be OK!
    return true;
}

bool ConnectionAssistant::ensureNotCrossConnecting(Port *pPort1, Port *pPort2)
{
    // Check so that both components to connect have been added to the same system (or we are connecting to parent system)
    if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()->getSystemParent()) )
    {
        if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()) && (pPort2->getComponent()->getSystemParent() != pPort1->getComponent()) )
        {
            mpComponentSystem->addErrorMessage("The components, {"+pPort1->getComponentName()+"} and {"+pPort2->getComponentName()+"}, "+"must belong to the same subsystem");
            return false;
        }
    }
    return true;
}

//! @brief Detects if a port is a multiport and then adds adn returns a subport
//! @param [in] pMaybeMultiport A pointer to the port that may be a multiport
//! @returns A pointer to a new subport in teh multiport, or the pMaybeMultiport itself if it was not a multiport
Port *ConnectionAssistant::ifMultiportAddSubport(Port *pMaybeMultiport)
{
    // If the port is a multiport then create a new subport and then return it (as the actual port)
    if (pMaybeMultiport->getPortType() >= MultiportType)
    {
        return pMaybeMultiport->addSubPort();
    }

    // As the port was not a multiport lets return it
    return pMaybeMultiport;
}

void ConnectionAssistant::ifMultiportPrepareDissconnect(Port *pMaybeMultiport1, Port *pMaybeMultiport2, Port *&rpActualPort1, Port *&rpActualPort2)
{
    if ((pMaybeMultiport1->getPortType() >= MultiportType) && (pMaybeMultiport2->getPortType() >= MultiportType))
    {
        mpComponentSystem->addFatalMessage("ifMultiportFindActualPort():Both ports can not be multiports");
        rpActualPort1 = 0;
        rpActualPort2 = 0;
        return;
    }

    // if pMaybeMultiport1 is a multiport, but not other port
    if (pMaybeMultiport1->getPortType() >= MultiportType)
    {
        rpActualPort1 = findMultiportSubportFromOtherPort(pMaybeMultiport1, pMaybeMultiport2);
        rpActualPort2 = pMaybeMultiport2;
        if(rpActualPort1 == 0)
        {
            mpComponentSystem->addFatalMessage("ifMultiportFindActualPort(): pActualPort1 == 0");
        }
    }


    // if pMaybeMultiport2 is a multiport, but not other port
    if (pMaybeMultiport2->getPortType() >= MultiportType)
    {
        rpActualPort1 = pMaybeMultiport1;
        rpActualPort2 = findMultiportSubportFromOtherPort(pMaybeMultiport2, pMaybeMultiport1);
        if(rpActualPort2 == 0)
        {
            mpComponentSystem->addFatalMessage("ifMultiportFindActualPort(): pActualPort2 == 0");
        }
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterConnect(Port *pMaybeMultiport, Port *pActualPort, const bool wasSucess)
{
    if (pMaybeMultiport == pActualPort->getParentPort())
    {
        if (wasSucess)
        {
            //! @todo What do we need to do to handle sucess
        }
        else
        {
            //We need to remove the last created subport
            pMaybeMultiport->removeSubPort(pActualPort);
        }
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterDissconnect(Port *pMaybeMultiport, Port *pActualPort, const bool wasSucess)
{
    if (pMaybeMultiport == pActualPort->getParentPort())
    {
        if (wasSucess)
        {
            //If sucessful we should remove the empty port
            pMaybeMultiport->removeSubPort(pActualPort);
            pActualPort = 0;
        }
        else
        {
            //! @todo What do we need to do to handle failure, nothing maybe
        }
    }
}

void ConnectionAssistant::removeNode(Node *pNode)
{
    if (pNode->getOwnerSystem())
    {
        pNode->getOwnerSystem()->removeSubNode(pNode);
    }
    mpComponentSystem->getHopsanEssentials()->removeNode(pNode);
}

////! @brief Prepares port pointers for multiport disconnections,
//void ConnectionAssistant::ifMultiportPrepareForDisconnect(Port *&rpPort1, Port *&rpPort2, Port *&rpMultiSubPort1, Port *&rpMultiSubPort2)
//{
//    // First make sure that multiport pointers are zero if no multiports are beeing connected
//    rpMultiSubPort1=0;
//    rpMultiSubPort2=0;

//    // Port 1 is a multiport, but not port2
//    if (rpPort1->getPortType() >= MultiportType && rpPort2->getPortType() < MultiportType )
//    {
//        rpMultiSubPort1 = findMultiportSubportFromOtherPort(rpPort1, rpPort2);
//        if(rpMultiSubPort1 == 0)
//        {
//            mpComponentSystem->addFatalMessage("ifMultiportPrepareForDisconnect(): rpMultiSubPort1 == 0");
//        }
//    }
//    // Port 2 is a multiport, but not port1
//    else if (rpPort1->getPortType() < MultiportType && rpPort2->getPortType() >= MultiportType )
//    {
//        rpMultiSubPort2 = findMultiportSubportFromOtherPort(rpPort2, rpPort1);
//        if(rpMultiSubPort2 == 0)
//        {
//            mpComponentSystem->addFatalMessage("ifMultiportPrepareForDisconnect(): rpMultiSubPort2 == 0");
//        }
//    }
//    // both ports are multiports
//    else if (rpPort1->getPortType() >= MultiportType && rpPort2->getPortType() >= MultiportType )
//    {
//        mpComponentSystem->addFatalMessage("ifMultiportPrepareForDisconnect(): Multiport <-> Multiport disconnection has not been implemented yet.");
//        //! @todo need to search around to find correct subports
//    }
//}


//! @brief Disconnect two ports, string version
//! @param [in] compname1 The name of the first component
//! @param [in] portname1 The name of the port on the first component
//! @param [in] compname2 The name of the second component
//! @param [in] portname2 The name of the port on the second component
//! @returns True if success, False if failed
bool ComponentSystem::disconnect(const string compname1, const string portname1, const string compname2, const string portname2)
{
    Component *pComp1, *pComp2;
    Port *pPort1, *pPort2;

    pComp1 = getSubComponentOrThisIfSysPort(compname1);
    pComp2 = getSubComponentOrThisIfSysPort(compname2);

    if ( (pComp1!=0) && (pComp2!=0) )
    {
        pPort1 = pComp1->getPort(portname1);
        pPort2 = pComp2->getPort(portname2);

        if ( (pComp1!=0) && (pComp2!=0) )
        {
            return disconnect(pPort1, pPort2);
        }
    }

    stringstream ss;
    ss << "Disconnect: Could not find either " << compname1 << "->" << portname1 << " or " << compname2 << "->" << portname2 << endl;
    addDebugMessage(ss.str());
    return false;
}

//! @brief Disconnects two ports and remove node if no one is using it any more.
//! @param pPort1 Pointer to first port
//! @param pPort2 Pointer to second port
bool ComponentSystem::disconnect(Port *pPort1, Port *pPort2)
{
    cout << "disconnecting " << pPort1->getComponentName() << " " << pPort1->getName() << "  and  " << pPort2->getComponentName() << " " << pPort2->getName() << endl;

    ConnectionAssistant disconnAssistant(this);
    stringstream ss;
    bool success = false;
    //! @todo some more advanced error handling (are the ports really connected to each other and such)

    if (pPort1->isConnected() && pPort2->isConnected())
    {
        // If non of the ports are multiports
        if ( !(pPort1->isMultiPort() || pPort2->isMultiPort()) )
        {
            success = disconnAssistant.splitNodeConnection(pPort1, pPort2);
        }
        // If one of the ports is a multiport
        else if ( pPort1->isMultiPort() || pPort2->isMultiPort() )
        {
            //! @todo what happens if we disconnect a multiport from a port with multiple connections (can that even happen)
            if(pPort1->isMultiPort() && pPort2->isMultiPort())
            {
                addFatalMessage("ComponentSystem::disconnect(): Trying to disconnect two multiports.");
                return false;
            }

            // Handle multiports
            Port *pActualPort1, *pActualPort2;
            disconnAssistant.ifMultiportPrepareDissconnect(pPort1, pPort2, pActualPort1, pActualPort2);

            success = disconnAssistant.splitNodeConnection(pActualPort1, pActualPort2);

            // Handle multiport connection sucess or failure
            disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort1, pActualPort1, success);
            disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort2, pActualPort2, success);
        }

        disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort1);
        disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort2);
        //! @todo maybe incorporate the clear checks into delete node and unmerge
    }
    else
    {
        addWarningMessage("In disconnect: At least one of the ports do not seem to be connected, (does nothing)");
    }

    //Update the CQS type
    this->determineCQSType();

    //Update parent cqs-type
    //! @todo we should only do this if we are actually connected directly to our parent, but I dont know what will take the most time, to ckeach if we are connected to parent or to just allways refresh parent
    if (!this->isTopLevelSystem())
    {
        this->mpSystemParent->determineCQSType();
    }

    ss << "Disconnected: {"<< pPort1->getComponent()->getName() << "::" << pPort1->getName() << "} and {" << pPort2->getComponent()->getName() << "::" << pPort2->getName() << "}";
    cout << ss.str() << endl;
    addDebugMessage(ss.str(), "succesfuldisconnect");

    return success;
}


//! @brief Sets the desired time step in a component system.
//! @brief param timestep New desired time step
void ComponentSystem::setDesiredTimestep(const double timestep)
{
    mDesiredTimestep = timestep;
    setTimestep(timestep);
}


void ComponentSystem::setInheritTimestep(const bool inherit)
{
    mInheritTimestep = inherit;
}


bool ComponentSystem::doesInheritTimestep() const
{
    return mInheritTimestep;
}


//void ComponentSystem::setTimestep(const double timestep)
//{
//    mTimestep = timestep;
//
//    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
//    {
//        if (!(mComponentSignalptrs[s]->isComponentSystem()))
//        {
//            mComponentSignalptrs[s]->setTimestep(timestep);
//        }
//    }
//
//    //C components
//    for (size_t c=0; c < mComponentCptrs.size(); ++c)
//    {
//        if (!(mComponentCptrs[c]->isComponentSystem()))
//        {
//            mComponentCptrs[c]->setTimestep(timestep);
//        }
//    }
//
//    //Q components
//    for (size_t q=0; q < mComponentQptrs.size(); ++q)
//    {
//        if (!(mComponentQptrs[q]->isComponentSystem()))
//        {
//            mComponentQptrs[q]->setTimestep(timestep);
//        }
//    }
//}


//! @brief Sets the time step in a component system.
//! Also propagates it to all contained components
//! @brief param timestep New time step
void ComponentSystem::setTimestep(const double timestep)
{
    mTimestep = timestep;

    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (!(mComponentSignalptrs[s]->isComponentSystem()))
        {
            mComponentSignalptrs[s]->setTimestep(timestep);
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (!(mComponentCptrs[c]->isComponentSystem()))
        {
            mComponentCptrs[c]->setTimestep(timestep);
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (!(mComponentQptrs[q]->isComponentSystem()))
        {
            mComponentQptrs[q]->setTimestep(timestep);
        }
    }
}


//! @brief Figure out which timestep to use for all sub systems
//! @param componentPtrs Vector with pointers to all sub components
void ComponentSystem::adjustTimestep(vector<Component*> componentPtrs)
{
    for (size_t c=0; c < componentPtrs.size(); ++c)
    {
        if (componentPtrs[c]->isComponentSystem())
        {
            if(componentPtrs[c]->doesInheritTimestep()) //Inherit timestep from parent system
            {
                componentPtrs[c]->setTimestep(mTimestep);
            }
            else    //Use desired timestep, and adjust it if necessary
            {
                double subTs = componentPtrs[c]->mDesiredTimestep;

                //If a subsystem's timestep is larger than this sytem's
                //timestep change it to this system's timestep
                if ((subTs > mTimestep) || (subTs < -0.0))
                {
                    subTs = mTimestep;
                }
                //Check that subRs is a multiple of timestep
                else// if ((timestep/subTs - floor(timestep/subTs)) > 0.00001*subTs)
                {
                    //subTs should get the nearest multiple of timestep as possible,
                    subTs = mTimestep/floor(mTimestep/subTs+0.5);
                }
                componentPtrs[c]->setTimestep(subTs);
            }
        }
        else
        {
            componentPtrs[c]->setTimestep(mTimestep);
        }
    }
}

void ComponentSystem::setupLogTimesteps(const double startT, const double stopT, const double Ts, const size_t nLogSamples)
{
    // We do not want to log negative time
    const double logStartT = max(startT,0.0);

    // Calc logDt and
    mLogTimeDt = (stopT-logStartT)/double(nLogSamples-1);

    // Figure out at which samples logging should happen
    double logT=logStartT;
    double simT=startT;

    mLogTheseTimeSteps.clear();

    if (nLogSamples > 0)
    {
        mLogTheseTimeSteps.reserve(nLogSamples);

        // Figure out the first simulation step to log (the one where simT >= logT)
        size_t n = (logT-simT)/double(Ts)+0.5;
        mLogTheseTimeSteps.push_back(n);
        // Fastforward simT
        simT += double(n)*Ts;

        // Now Calculate which additional simulation steps should be logged
        while (mLogTheseTimeSteps.size() < nLogSamples)
        {
            logT += mLogTimeDt;
            n = size_t((logT-simT)/Ts+0.5);
            simT += double(n)*Ts;

            //cout << "SimT: " << simT << " logT: " << logT << " logT-simT: " << logT-simT << endl;
            mLogTheseTimeSteps.push_back(mLogTheseTimeSteps.back() + n);
        }

        //! @todo sanity check on log slots
        if (mnLogSlots != mLogTheseTimeSteps.size())
        {
            cout << "Error: mnLogSlots: " << mnLogSlots << " mLogTheseTimeSteps.size(): " << mLogTheseTimeSteps.size() << endl;
        }

        //cout << "n: " << n << endl;
        cout << "mNumSimulationSteps: " << size_t((stopT-logStartT)/Ts+0.5) << endl;
        cout << "mLastStepToLog: " << mLogTheseTimeSteps.back() << endl;
        cout << "mLogTimeDt: " << mLogTimeDt << " mTimeStepsToLog.size(): " << mLogTheseTimeSteps.size() << endl;
    }
    else
    {
        mEnableLogData = false;
    }
//    for (int i=0; i<mTimeStepsToLog.size(); ++i)
//    {
//        cout << mTimeStepsToLog[i] << " ";
//    }
//    cout << endl;
}

//! @brief Determines if all subnodes and subsystems subnodes should log data, Turn ALL ON or OFF
//! @todo name of this function is bad, this is a toggle function
void ComponentSystem::setAllNodesDoLogData(const bool logornot)
{
    // Do this systems nodes
    if (logornot)
    {

        for (size_t i=0; i<mSubNodePtrs.size(); ++i)
        {
            mSubNodePtrs[i]->enableLog();
        }
    }
    else
    {
        for (size_t i=0; i<mSubNodePtrs.size(); ++i)
        {
            mSubNodePtrs[i]->disableLog();
        }
    }

    // Do all subsystems
    SubComponentMapT::iterator scit;
    for (scit=mSubComponentMap.begin(); scit!=mSubComponentMap.end(); ++scit)
    {
        if (scit->second->isComponentSystem())
        {
            //!< @todo maybe should use static cast (quicker) or overloaded function in Component instead of casting
            dynamic_cast<ComponentSystem*>(scit->second)->setAllNodesDoLogData(logornot);
        }
    }
}


//! @brief Returns if start values should be loaded before simulation. If not, old simulation results is used as startvalues.
bool ComponentSystem::doesKeepStartValues()
{
    return mKeepStartValues;
}


//! @brief Set if or not start values should be loaded before simulation. If not, old simulation results is used as startvalues.
void ComponentSystem::setLoadStartValues(bool load)
{
    mKeepStartValues = load;
}


//! @brief Checks that everything is OK before simulation
bool ComponentSystem::checkModelBeforeSimulation()
{
    //Make sure that there are no components or systems with an undefined cqs_type present
    if (mComponentUndefinedptrs.size() > 0)
    {

        for (size_t i=0; i<mComponentUndefinedptrs.size(); ++i)
        {
            addErrorMessage(string("The component {") + mComponentUndefinedptrs[i]->getName() + string("} has an invalid CQS-type: ") + mComponentUndefinedptrs[i]->getTypeCQSString());
        }
        return false;
    }

    //Check this systems own SystemPorts, are they connected (they must be)
    vector<Port*> ports = getPortPtrVector();
    for (size_t i=0; i<ports.size(); ++i)
    {
        if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
        {
            addErrorMessage("Port " + ports[i]->getName() + " in " + getName() + " is not connected!");
            return false;
        }
        else if( ports[i]->isConnected() )
        {
            if(ports[i]->getNodePtr()->getNumberOfPortsByType(PowerPortType) == 1)
            {
                addErrorMessage("Port " + ports[i]->getName() + " in " + ports[i]->getComponentName() + " is connected to a node with only one attached power port!");
                return false;
            }
        }
    }

    //Check all subcomponents to make sure that all requirements for simulation are met
    //scmit = The subcomponent map iterator
    SubComponentMapT::iterator scmit = mSubComponentMap.begin();
    for ( ; scmit!=mSubComponentMap.end(); ++scmit)
    {
        Component* pComp = scmit->second; //Component pointer

        //Check that ALL ports that MUST be connected are connected
        vector<Port*> ports = pComp->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
            {
                addErrorMessage("Port " + ports[i]->getName() + " on " + pComp->getName() + " is not connected!");
                return false;
            }
            else if( ports[i]->isConnected() )
            {
                if(ports[i]->getNodePtr()->getNumberOfPortsByType(PowerPortType) == 1)
                {
                    addErrorMessage("Port " + ports[i]->getName() + " in " + ports[i]->getComponentName() + " is connected to a node with only one power port!");
                    return false;
                }
            }

            //Check parameters in subcomponents
            std::string errParName;
            if(!(pComp->checkParameters(errParName)))
            {
                addErrorMessage("The parameter " + errParName + " in system " + getName() + " and component " + pComp->getName() + " can not be evaluated, a system parameter has maybe been deleted or re-typed.");
                return false;
            }
        }

        //Check parameters in system
        std::string errParName;
        if(!(checkParameters(errParName)))
        {
            addErrorMessage("The system parameter " + errParName + " in system " + getName() + " can not be evaluated, it maybe depend on a deleted system parameter.");
            return false;
        }

        //Recures testing into subsystems
        if (pComp->isComponentSystem())
        {
            if (!pComp->checkModelBeforeSimulation())
            {
                return false;
            }
        }

        //! @todo check that all C-component required ports are connected to Q-component ports

        //! @todo check more stuff
    }

    return true;
}

//! @brief Load start values by telling each component to load their start values
void ComponentSystem::loadStartValues()
{
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
}


//! @brief Load start values from last simulation to start value container
void ComponentSystem::loadStartValuesFromSimulation()
{
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
}


void ComponentSystem::loadParameters(std::string filePath)
{
    loadHopsanParameterFile(filePath, getHopsanEssentials(), this);
}


void ComponentSystem::loadParameters(std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > > parameterMap)
{
    std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >::iterator it;
    for(it=parameterMap.begin(); it!=parameterMap.end(); ++it)
    {
        std::string name = it->first;
        if(this->haveSubComponent(name))
        {
            std::vector<std::string> parNames = it->second.first;
            std::vector<std::string> parValues = it->second.second;
            for(size_t i=0; i<parNames.size(); ++i)
            {
                this->getSubComponent(name)->setParameterValue(parNames[i], parValues[i]);
            }
        }
    }
}


//! @brief Initializes a system and all its contained components before a simulation.
//! Also allocates log data memory.
//! @param startT Start time of simulation
//! @param stopT Stop time of simulation
//! @param nSamples Number of log samples
bool ComponentSystem::initialize(const double startT, const double stopT)
{
    addLogMess("ComponentSystem::initialize()");

    //cout << "Initializing SubSystem: " << this->mName << endl;
    mStopSimulation = false; //This variable cannot be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.

    // Set initial time
    mTime = startT;
    mTotalTakenSimulationSteps=0;

    // Make sure timestep is not to low
    if (mTimestep < 10*(std::numeric_limits<double>::min)())
    {
        addErrorMessage("The timestep is to low");
        return false;
    }

    //cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;
    this->setLogSettingsNSamples(mRequestedNumLogSamples, startT, stopT, mTimestep); //! @todo make it possible to use other logtimestep methods
    this->setupLogTimesteps(startT, stopT, mTimestep, mnLogSlots);

    // preAllocate local logspace
    this->preAllocateLogSpace(startT, stopT, mnLogSlots);

    // If we failed allocation then abort
    if (mStopSimulation)
    {
        return false;
    }

    adjustTimestep(mComponentSignalptrs);
    adjustTimestep(mComponentCptrs);
    adjustTimestep(mComponentQptrs);

    if(!this->sortComponentVector(mComponentSignalptrs))
    {
        return false;
    }
    this->sortComponentVector(mComponentCptrs);
    this->sortComponentVector(mComponentQptrs);

    // Only set startvalues from top-level system, else they will be set again in the subsystem initialize calls
    if (this->isTopLevelSystem())
    {
        if(!mKeepStartValues)
        {
            loadStartValues();
        }
    }

    //Init
    updateParameters();
    //Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentSignalptrs[s]->initializeAutoSignalNodeDataPtrs();
        mComponentSignalptrs[s]->updateParameters();

        if (mComponentSignalptrs[s]->isComponentSystem())
        {
            //! @todo should we use our own nSamples or the subsystems own ?
            static_cast<ComponentSystem*>(mComponentSignalptrs[s])->setNumLogSamples(mRequestedNumLogSamples);
        }
        else
        {
            mComponentSignalptrs[s]->initializeDynamicParameters();
            mComponentSignalptrs[s]->updateDynamicParameterValues();
        }

        if(!mComponentSignalptrs[s]->initialize(startT, stopT))
        {
            stopSimulation();
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentCptrs[c]->initializeAutoSignalNodeDataPtrs();
        mComponentCptrs[c]->updateParameters();

        if (mComponentCptrs[c]->isComponentSystem())
        {
            //! @todo should we use our own nSamples ore the subsystems own ?
            static_cast<ComponentSystem*>(mComponentCptrs[c])->setNumLogSamples(mRequestedNumLogSamples);
        }
        else
        {
            mComponentCptrs[c]->initializeDynamicParameters();
            mComponentCptrs[c]->updateDynamicParameterValues();
        }

        if(!mComponentCptrs[c]->initialize(startT, stopT))
        {
            stopSimulation();
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentQptrs[q]->initializeAutoSignalNodeDataPtrs();
        mComponentQptrs[q]->updateParameters();

        if (mComponentQptrs[q]->isComponentSystem())
        {
            //! @todo should we use our own nSamples ore the subsystems own ?
            static_cast<ComponentSystem*>(mComponentQptrs[q])->setNumLogSamples(mRequestedNumLogSamples);
        }
        else
        {
            mComponentQptrs[q]->initializeDynamicParameters();
            mComponentQptrs[q]->updateDynamicParameterValues();
        }

        if(!mComponentQptrs[q]->initialize(startT, stopT))
        {
            stopSimulation();
        }
    }

    if (mStopSimulation)
    {
        return false;
    }

    logTimeAndNodes(mTotalTakenSimulationSteps); // Log the startvalues

    // We seems to have initialized successfully
    return true;
}


#ifdef USETBB






//! @brief Simulate function for multi-threaded simulations.
//! @param startT Start time of simulation
//! @param stopT Stop time of simulation
//! @param nDesiredThreads Desired amount of simulation threads
//void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges)
//{
//    mTime = startT;
//    double stopTsafe = stopT - mTimestep/2.0;                   //Calculate the "actual" stop time, minus half a timestep is here to ensure that no numerical issues occur

//    logTimeAndNodes(mTime);                                         //Log the first time step

//    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);      //Calculate how many threads to actually use

//    if(!noChanges)
//    {
//        mSplitCVector.clear();
//        mSplitQVector.clear();
//        mSplitSignalVector.clear();
//        mSplitNodeVector.clear();

//        simulateAndMeasureTime(5);                                  //Measure time
//        sortComponentVectorsByMeasuredTime();                       //Sort component vectors

//        for(size_t q=0; q<mComponentQptrs.size(); ++q)
//        {
//            std::stringstream ss;
//            ss << "Time for " << mComponentQptrs.at(q)->getName() << ": " << mComponentQptrs.at(q)->getMeasuredTime();
//            addDebugMessage(ss.str());
//        }
//        for(size_t c=0; c<mComponentCptrs.size(); ++c)
//        {
//            std::stringstream ss;
//            ss << "Time for " << mComponentCptrs.at(c)->getName() << ": " << mComponentCptrs.at(c)->getMeasuredTime();
//            addDebugMessage(ss.str());
//        }
//        for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
//        {
//            std::stringstream ss;
//            ss << "Time for " << mComponentSignalptrs.at(s)->getName() << ": " << mComponentSignalptrs.at(s)->getMeasuredTime();
//            addDebugMessage(ss.str());
//        }

//        distributeCcomponents(mSplitCVector, nThreads);              //Distribute components and nodes
//        distributeQcomponents(mSplitQVector, nThreads);
//        distributeSignalcomponents(mSplitSignalVector, nThreads);
//        distributeNodePointers(mSplitNodeVector, nThreads);

//        //! @todo Reiniti

//    }

//    tbb::task_group *simTasks;                                  //Initialize TBB routines for parallel  simulation
//    simTasks = new tbb::task_group;

//    //Execute simulation
//#define BARRIER_SYNC
//#ifdef BARRIER_SYNC
//    mvTimePtrs.push_back(&mTime);
//    BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
//    BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
//    BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
//    BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

//    simTasks->run(taskSimMaster(this, mSplitSignalVector[0], mSplitCVector[0], mSplitQVector[0],             //Create master thread
//                                mSplitNodeVector[0], mvTimePtrs, mTime, mTimestep, stopTsafe, nThreads, 0,
//                                pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));

//    for(size_t t=1; t < nThreads; ++t)
//    {
//        simTasks->run(taskSimSlave(mSplitSignalVector[t], mSplitCVector[t], mSplitQVector[t],          //Create slave threads
//                                   mSplitNodeVector[t], mTime, mTimestep, stopTsafe, nThreads, t,
//                                   pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));
//    }

//    simTasks->wait();                                           //Wait for all tasks to finish

//    delete(simTasks);                                           //Clean up
//    delete(pBarrierLock_S);
//    delete(pBarrierLock_C);
//    delete(pBarrierLock_Q);
//    delete(pBarrierLock_N);
//#else
//    vector<Component*> tempVector;
//    for(int i=mComponentSignalptrs.size()-1; i>-1; --i)
//    {
//        tempVector.push_back(mComponentSignalptrs[i]);
//    }
//    mComponentSignalptrs = tempVector;
//    tempVector.clear();
//    for(int i=mComponentCptrs.size()-1; i>-1; --i)
//    {
//        tempVector.push_back(mComponentCptrs[i]);
//    }
//    mComponentCptrs = tempVector;
//    tempVector.clear();
//    for(int i=mComponentQptrs.size()-1; i>-1; --i)
//    {
//        tempVector.push_back(mComponentQptrs[i]);
//    }
//    mComponentQptrs = tempVector;

//    cout << "Creating task pools!" << endl;

//    TaskPool<Component> *sPool = new TaskPool<Component>(mComponentSignalptrs, nThreads);
//    TaskPool<Component> *qPool = new TaskPool<Component>(mComponentQptrs, nThreads);
//    TaskPool<Component> *cPool = new TaskPool<Component>(mComponentCptrs, nThreads);
//    TaskPool<Node> *nPool = new TaskPool<Node>(mSubNodePtrs, nThreads);

//    cout << "Starting task threads!";
//   // assert("Starting task threads"==0);

//    for(size_t t=0; t < nThreads; ++t)
//    {
//        simTasks->run(taskSimPool(sPool, qPool, cPool, nPool, mTime, mTimestep, stopTsafe, t, this));
//    }
//    simTasks->wait();                                           //Wait for all tasks to finish

//    delete(simTasks);                                           //Clean up
//#endif
//}

void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges)
{
    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);      //Calculate how many threads to actually use

    if(!noChanges)
    {
        mSplitCVector.clear();
        mSplitQVector.clear();
        mSplitSignalVector.clear();
        mSplitNodeVector.clear();

        simulateAndMeasureTime(5);                                  //Measure time
        sortComponentVectorsByMeasuredTime();                       //Sort component vectors

        for(size_t q=0; q<mComponentQptrs.size(); ++q)
        {
            std::stringstream ss;
            ss << "Time for " << mComponentQptrs.at(q)->getName() << ": " << mComponentQptrs.at(q)->getMeasuredTime();
            addDebugMessage(ss.str());
        }
        for(size_t c=0; c<mComponentCptrs.size(); ++c)
        {
            std::stringstream ss;
            ss << "Time for " << mComponentCptrs.at(c)->getName() << ": " << mComponentCptrs.at(c)->getMeasuredTime();
            addDebugMessage(ss.str());
        }
        for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
        {
            std::stringstream ss;
            ss << "Time for " << mComponentSignalptrs.at(s)->getName() << ": " << mComponentSignalptrs.at(s)->getMeasuredTime();
            addDebugMessage(ss.str());
        }

        distributeCcomponents(mSplitCVector, nThreads);              //Distribute components and nodes
        distributeQcomponents(mSplitQVector, nThreads);
        distributeSignalcomponents(mSplitSignalVector, nThreads);
        distributeNodePointers(mSplitNodeVector, nThreads);

        // Re-initialize the system to reset values and timers
        //! @note This only work for top level systems where the simulateMultiThreaded will not be called nmore than once
        this->initialize(startT, stopT);
    }


    size_t nSteps = calcNumSimSteps(startT, stopT);
    tbb::task_group *simTasks;                                  //Initialize TBB routines for parallel  simulation
    simTasks = new tbb::task_group;

    //Execute simulation
#define BARRIER_SYNC
#ifdef BARRIER_SYNC
    mvTimePtrs.push_back(&mTime);
    BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
    BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
    BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
    BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

    simTasks->run(taskSimMaster(this, mSplitSignalVector[0], mSplitCVector[0], mSplitQVector[0],             //Create master thread
                                mSplitNodeVector[0], mvTimePtrs, mTime, mTimestep, nSteps, nThreads, 0,
                                pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));

    for(size_t t=1; t < nThreads; ++t)
    {
        simTasks->run(taskSimSlave(this, mSplitSignalVector[t], mSplitCVector[t], mSplitQVector[t],          //Create slave threads
                                   mSplitNodeVector[t], mTime, mTimestep, nSteps, nThreads, t,
                                   pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));
    }

    simTasks->wait();                                           //Wait for all tasks to finish

    delete(simTasks);                                           //Clean up
    delete(pBarrierLock_S);
    delete(pBarrierLock_C);
    delete(pBarrierLock_Q);
    delete(pBarrierLock_N);
#else
    vector<Component*> tempVector;
    for(int i=mComponentSignalptrs.size()-1; i>-1; --i)
    {
        tempVector.push_back(mComponentSignalptrs[i]);
    }
    mComponentSignalptrs = tempVector;
    tempVector.clear();
    for(int i=mComponentCptrs.size()-1; i>-1; --i)
    {
        tempVector.push_back(mComponentCptrs[i]);
    }
    mComponentCptrs = tempVector;
    tempVector.clear();
    for(int i=mComponentQptrs.size()-1; i>-1; --i)
    {
        tempVector.push_back(mComponentQptrs[i]);
    }
    mComponentQptrs = tempVector;

    cout << "Creating task pools!" << endl;

    TaskPool<Component> *sPool = new TaskPool<Component>(mComponentSignalptrs, nThreads);
    TaskPool<Component> *qPool = new TaskPool<Component>(mComponentQptrs, nThreads);
    TaskPool<Component> *cPool = new TaskPool<Component>(mComponentCptrs, nThreads);
    TaskPool<Node> *nPool = new TaskPool<Node>(mSubNodePtrs, nThreads);

    cout << "Starting task threads!";
   // assert("Starting task threads"==0);

    for(size_t t=0; t < nThreads; ++t)
    {
        simTasks->run(taskSimPool(sPool, qPool, cPool, nPool, mTime, mTimestep, nSteps, t, this));
    }
    simTasks->wait();                                           //Wait for all tasks to finish

    delete(simTasks);                                           //Clean up
#endif
}




//! @brief Helper function that simulates all components and measure their average time requirements.
//! @param steps How many steps to simulate
//! @todo Could we use the other tictoc to avoid tbb dependency, then we could use it as a bottleneck finder even if tbb not present
bool ComponentSystem::simulateAndMeasureTime(const size_t nSteps)
{
    // Reset all measured times first
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
        mComponentSignalptrs[s]->setMeasuredTime(0);
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
        mComponentCptrs[c]->setMeasuredTime(0);
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
        mComponentQptrs[q]->setMeasuredTime(0);


    // Measure time for each component during specified amount of steps
    double time=mTime; // Init time
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
    {
        tbb::tick_count comp_start = tbb::tick_count::now();
        for(size_t t=0; t<nSteps; ++t)
        {
            time += mTimestep;
            mComponentSignalptrs[s]->simulate(time);
        }
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentSignalptrs[s]->setMeasuredTime((comp_end-comp_start).seconds());
    }

    time=mTime; // Reset time
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        tbb::tick_count comp_start = tbb::tick_count::now();
        for(size_t t=0; t<nSteps; ++t)
        {
            time += mTimestep;
            mComponentCptrs[c]->simulate(time);
        }
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentCptrs[c]->setMeasuredTime((comp_end-comp_start).seconds());
    }

    time=mTime; // Reset time
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        tbb::tick_count comp_start = tbb::tick_count::now();
        for(size_t t=0; t<nSteps; ++t)
        {
            time += mTimestep;
            mComponentQptrs[q]->simulate(time);
        }
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentQptrs[q]->setMeasuredTime((comp_end-comp_start).seconds());
    }

    return true;
}


//! @brief Returns the total sum of the measured time of the components in the system
double ComponentSystem::getTotalMeasuredTime()
{
    double time = 0;
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
    {
        time += mComponentSignalptrs[s]->getMeasuredTime();
    }
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        time += mComponentCptrs[c]->getMeasuredTime();
    }
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        time += mComponentQptrs[q]->getMeasuredTime();
    }

    return time;
}


//! @brief Sorts a vector of component system pointers by their required simulation time
//! @param [in out]systemVector Vector with system pointers to sort
void SimulationHandler::sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector)
{
    size_t i, j;
    bool didSwap = true;
    ComponentSystem *tempSystem;
    for(i = 1; i < rSystemVector.size(); ++i)
    {
        didSwap = false;
        for (j=0; j < (rSystemVector.size()-1); ++j)
        {
            if (rSystemVector[j+1]->getTotalMeasuredTime() > rSystemVector[j]->getTotalMeasuredTime())
            {
                tempSystem = rSystemVector[j];             //Swap elements
                rSystemVector[j] = rSystemVector[j+1];
                rSystemVector[j+1] = tempSystem;
                didSwap = true;               //Indicates that a swap occurred
            }
        }

        if(!didSwap)
        {
            break;
        }
    }
}


//! @brief Helper function that sorts C- and Q- component vectors by simulation time for each component.
//! @todo This function uses bubblesort. Maybe change to something faster.
void ComponentSystem::sortComponentVectorsByMeasuredTime()
{
        //Sort the components from longest to shortest time requirement
    size_t i, j;
    bool flag = true;
    Component *tempC;
    for(i = 1; (i < mComponentCptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentCptrs.size()-1); ++j)
        {
            if (mComponentCptrs[j+1]->getMeasuredTime() > mComponentCptrs[j]->getMeasuredTime())
            {
                tempC = mComponentCptrs[j];             //Swap elements
                mComponentCptrs[j] = mComponentCptrs[j+1];
                mComponentCptrs[j+1] = tempC;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }
    flag = true;
    Component *tempQ;
    for(i = 1; (i < mComponentQptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentQptrs.size()-1); ++j)
        {
            if (mComponentQptrs[j+1]->getMeasuredTime() > mComponentQptrs[j]->getMeasuredTime())
            {
                tempQ = mComponentQptrs[j];             //Swap elements
                mComponentQptrs[j] = mComponentQptrs[j+1];
                mComponentQptrs[j+1] = tempQ;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }
}


////! @brief Helper function that decides how many thread to use.
////! User specifies desired amount, but it is limited by how many cores the processor has.
////! @param nDesiredThreads How many threads the user wants
//int ComponentSystem::getNumberOfThreads(size_t nDesiredThreads)
//{
//        //Obtain number of processor cores from environment variable, or use user specified value if not zero
//    size_t nThreads;
//    size_t nCores;
//#ifdef WIN32
//    if(getenv("NUMBER_OF_PROCESSORS") != 0)
//    {
//        string temp = getenv("NUMBER_OF_PROCESSORS");
//        nCores = atoi(temp.c_str());
//    }
//    else
//    {
//        nCores = 1;               //If non-Windows system, make sure there is at least one thread
//    }
//#else
//    nCores = max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
//#endif
//    if(nDesiredThreads != 0)
//    {
//        nThreads = nDesiredThreads;              //If user specifides a number of threads, attempt to use this number
//        if(nThreads > nCores)       //But limit number of threads to the number of system cores
//        {
//            nThreads = nCores;
//        }
//    }
//    else
//    {
//        nThreads = nCores;          //User specified nothing, so use one thread per core
//    }

//    return nThreads;
//}


//! @brief Helper function that distributes C componente equally over one vector per thread
//! Greedy algorithm is used. This does not guarantee an optimal solution, but is gives a 4/3-approximation.
//! @param rSplitCVector Reference to vector with vectors of components (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeCcomponents(vector< vector<Component*> > &rSplitCVector, size_t nThreads)
{
    vector<double> timeVector;
    timeVector.resize(nThreads);
    for(size_t i=0; i<nThreads; ++i)
    {
        timeVector[i] = 0;
    }
    rSplitCVector.resize(nThreads);

    //Cycle components from largest to smallest
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        //Find index of vector with smallest total time
        size_t smallestIndex=0;
        double smallestTime=timeVector.at(smallestIndex);
        for(size_t t=1; t<nThreads; ++t)
        {
            if(timeVector.at(t) < smallestTime)
            {
                smallestTime = timeVector.at(t);
                smallestIndex = t;
            }
        }

        //Insert current component to vector with smallest time
        rSplitCVector[smallestIndex].push_back(mComponentCptrs[c]);
        timeVector[smallestIndex] += mComponentCptrs[c]->getMeasuredTime();
    }

    for(size_t i=0; i<nThreads; ++i)
    {
        stringstream ss;
        ss << timeVector[i]*1000;
        addDebugMessage("Creating C-type thread vector, measured time = " + ss.str() + " ms", "cvector");
    }

        //Finally we sort each component vector, so that
        //signal components are simlated in correct order:
    for(size_t i=0; i<rSplitCVector.size(); ++i)
    {
        sortComponentVector(rSplitCVector[i]);
    }
}


//! @brief Helper function that distributes Q componente equally over one vector per thread
//! Greedy algorithm is used. This does not guarantee an optimal solution, but is gives a 4/3-approximation.
//! @param rSplitQVector Reference to vector with vectors of components (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeQcomponents(vector< vector<Component*> > &rSplitQVector, size_t nThreads)
{
    vector<double> timeVector;
    timeVector.resize(nThreads);
    for(size_t i=0; i<nThreads; ++i)
    {
        timeVector[i] = 0;
    }
    rSplitQVector.resize(nThreads);

    //Cycle components from largest to smallest
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        //Find index of vector with smallest total time
        size_t smallestIndex=0;
        double smallestTime=timeVector.at(smallestIndex);
        for(size_t t=1; t<nThreads; ++t)
        {
            if(timeVector.at(t) < smallestTime)
            {
                smallestTime = timeVector.at(t);
                smallestIndex = t;
            }
        }

        //Insert current component to vector with smallest time
        rSplitQVector[smallestIndex].push_back(mComponentQptrs[q]);
        timeVector[smallestIndex] += mComponentQptrs[q]->getMeasuredTime();
    }

    for(size_t i=0; i<nThreads; ++i)
    {
        stringstream ss;
        ss << timeVector[i]*1000;
        addDebugMessage("Creating Q-type thread vector, measured time = " + ss.str() + " ms", "qvector");
    }

        //Finally we sort each component vector, so that
        //signal components are simlated in correct order:
    for(size_t i=0; i<rSplitQVector.size(); ++i)
    {
        sortComponentVector(rSplitQVector[i]);
    }
}


//! @brief Helper function that distributes signal components over one vector per thread.
//! @param rSplitSignalVector Reference to vector with vectors of components (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeSignalcomponents(vector< vector<Component*> > &rSplitSignalVector, size_t nThreads)
{

        //First we want to divide the components into groups,
        //depending on who they are connected to.

    std::map<Component *, size_t> groupMap;     //Maps each component to a group number
    size_t curMax = 0;                          //Highest used group number

    //Loop through all signal components
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
    {
        bool foundOneConnection=false;

        //Loop through all ports in each signal component
        for(size_t p=0; p<mComponentSignalptrs[s]->getPortPtrVector().size(); ++p)
        {
            //Loop through all connected ports to each port in each signal component
            for(size_t c=0; c<mComponentSignalptrs[s]->getPortPtrVector()[p]->getConnectedPorts().size(); ++c)
            {
                foundOneConnection=true;

                //Compare group number between current signal component and each connected component
                Component *A = mComponentSignalptrs[s];
                Component *B = mComponentSignalptrs[s]->getPortPtrVector()[p]->getConnectedPorts()[c]->getComponent();
                if(!groupMap.count(A) && !groupMap.count(B))        //Neither component has a number, so give current component a new number
                {
                    groupMap.insert(std::pair<Component *, size_t>(A, curMax));
                    ++curMax;
                }
                else if(!groupMap.count(A) && groupMap.count(B))    //Connected port has a number, so give current component same number
                {
                    groupMap.insert(std::pair<Component *, size_t>(A, groupMap.find(B)->second));
                }
                else if(groupMap.count(A) && groupMap.count(B))     //Both component have numbers, so merge current components group with the other one
                {
                    //Merge A's value with B's
                    size_t Aval = groupMap.find(A)->second;
                    size_t BVal = groupMap.find(B)->second;
                    std::map<Component *, size_t>::iterator it;
                    for(it=groupMap.begin(); it!=groupMap.end(); ++it)
                    {
                        if((*it).second == Aval)
                        {
                            (*it).second = BVal;
                        }
                    }
                }
            }
        }

        //If not connections were found, this is a lonely component, so add it to its own group
        if(!foundOneConnection)
        {
            groupMap.insert(std::pair<Component *, size_t>(mComponentSignalptrs[s], curMax));
            ++curMax;
        }
    }


        //Now we assign each component to a simulation thread vector.
        //We keep grouped components together, and always fill thread
        //with least measured time first.

    rSplitSignalVector.resize(nThreads);
    size_t i=0;                                             //Group number
    size_t currentVector=0;                                 //The vector to which we are currently adding components
    size_t nAddedComponents=0;                              //Total amount of added components
    std::vector<double> vectorTime;                           //Contains total measured time in each vector
    for(size_t j=0; j<rSplitSignalVector.size(); ++j)
    {
        vectorTime.push_back(0.0);
    }

    while(nAddedComponents < groupMap.size())               //Loop while there are still components to add
    {
        std::map<Component *, size_t>::iterator it;
        for(it=groupMap.begin(); it!=groupMap.end(); ++it)
        {
            if((*it).second == i)                           //Add all components with group number i to current vector
            {
                rSplitSignalVector[currentVector].push_back((*it).first);
                vectorTime[currentVector] += (*it).first->getMeasuredTime();
                ++nAddedComponents;
            }
        }

        //Find vector with smallest amount of data, to use next loop
        for(size_t j=0; j<vectorTime.size(); ++j)
        {
            if(vectorTime[j] < vectorTime[currentVector])
                currentVector = j;
        }
        ++i;
    }

//    // DEBUG
//    for(size_t i=0; i<vectorTime.size(); ++i)
//    {
//        std::stringstream ss;
//        ss << 1000*vectorTime[i];
//        addDebugMessage("Creating S-type thread vector, measured time = " + ss.str() + " ms", "svector");
//    }
//    // END DEBUG


        //Finally we sort each component vector, so that
        //signal components are simlated in correct order:
    for(size_t i=0; i<rSplitSignalVector.size(); ++i)
    {
        sortComponentVector(rSplitSignalVector[i]);
    }
}


//! @brief Helper function that distributes node pointers equally over one vector per thread
//! @param rSplitNodeVector Reference to vector with vectors of node pointers (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeNodePointers(vector< vector<Node*> > &rSplitNodeVector, size_t nThreads)
{
    for(size_t c=0; c<nThreads; ++c)
    {
        vector<Node*> tempVector;
        rSplitNodeVector.push_back(tempVector);
    }
    size_t thread = 0;
    for(size_t n=0; n<mSubNodePtrs.size(); ++n)
    {
        rSplitNodeVector.at(thread).push_back(mSubNodePtrs.at(n));
        ++thread;
        if(thread>nThreads-1)
            thread = 0;
    }
}

#else

        //This overrides the multi-threaded simulation call with a single-threaded simulation if TBB is not installed.
//! @brief Simulate function that overrides multi-threaded simulation call with a single-threaded call
//! In case multi-threaded support is not available
void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t /*nThreads*/, const bool /*noChanges*/)
{
    this->addErrorMessage("Multi-threaded simulation not available, TBB library is not present.");
    this->simulate(stopT);
}


bool ComponentSystem::simulateAndMeasureTime(const size_t steps)
{
    this->addErrorMessage("Unable to measure simulation time without TBB library.");
    return false;
}

double ComponentSystem::getTotalMeasuredTime()
{
    this->addErrorMessage("Time measurement results not available without TBB library.");
    return 0;
}


void SimulationHandler::sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector)
{
    if(rSystemVector.size() > 0)
    {
        rSystemVector[0]->addErrorMessage("Sorting systems by measured time is not possible without the TBB library.");
    }
    return;
}


void ComponentSystem::distributeCcomponents(vector< vector<Component*> > &rSplitCVector, size_t nThreads)
{

}


void ComponentSystem::distributeQcomponents(vector< vector<Component*> > &rSplitQVector, size_t nThreads)
{

}


void ComponentSystem::distributeSignalcomponents(vector< vector<Component*> > &rSplitSignalVector, size_t nThreads)
{

}


void ComponentSystem::distributeNodePointers(vector< vector<Node*> > &rSplitNodeVector, size_t nThreads)
{

}

#endif


//! @brief Simulate function for single-threaded simulations.
//! @param startT Start time of simulation
//! @param stopT Stop time of simulation
//void ComponentSystem::simulate(const double startT, const double stopT)
//{
//    mTime = startT;

//    //Simulate
//    double stopTsafe = stopT - mTimestep/2.0; //minus half a timestep is here to ensure that no numerical issues occure

//    while ((mTime < stopTsafe) && (!mStopSimulation))
//    {
//        //! @todo maybe use iterators instead
//        //Signal components
//        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
//        {
//            mComponentSignalptrs[s]->simulate(mTime, mTime+mTimestep);
//        }

//        //C components
//        for (size_t c=0; c < mComponentCptrs.size(); ++c)
//        {
//            mComponentCptrs[c]->simulate(mTime, mTime+mTimestep);
//        }
//        //! @todo this will log q and p from last Ts but c Zc from this Ts, this is strange
//        logTimeAndNodes(mTime); //MOVED HERE BECAUSE C-COMP ARE SETTING START TIMES

//        //Q components
//        for (size_t q=0; q < mComponentQptrs.size(); ++q)
//        {
//            mComponentQptrs[q]->simulate(mTime, mTime+mTimestep);
//        }

//        mTime += mTimestep;
//    }
//}

void ComponentSystem::simulate(const double stopT)
{
    // Round to nearest, we may not get exactly the stop time that we want
    size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

    //Simulate
    for (size_t i=0; i<numSimulationSteps; ++i)
    {
        if (mStopSimulation)
        {
            break;
        }

        mTime += mTimestep; //mTime is updated here before the simulation,
                            //mTime is the current time during the simulateOneTimestep

        //! @todo maybe use iterators instead
        //Signal components
        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
        {
            mComponentSignalptrs[s]->simulate(mTime);
        }

        //C components
        for (size_t c=0; c < mComponentCptrs.size(); ++c)
        {
            mComponentCptrs[c]->simulate(mTime);
        }

        //Q components
        for (size_t q=0; q < mComponentQptrs.size(); ++q)
        {
            mComponentQptrs[q]->simulate(mTime);
        }

        ++mTotalTakenSimulationSteps;

        logTimeAndNodes(mTotalTakenSimulationSteps);
    }
}


//! @brief Simulates several systems sequentially
//! @param startT Start time for all systems
//! @param stopT Stop time for all systems
//! @param systemVector Vector of pointers to component systems
bool SimulationHandler::simulateMultipleSystems(const double stopT, vector<ComponentSystem*> &rSystemVector)
{
    bool aborted = false;
    for(size_t i=0; i<rSystemVector.size(); ++i)
    {
        rSystemVector[i]->simulate(stopT);
        aborted = aborted && rSystemVector[i]->wasSimulationAborted(); //!< @todo this will give abort=true if one the systems fail, maybe we should abort entierly when one do
    }
    return !aborted;
}


//! @brief Finalizes a system component and all its contained components after a simulation.
void ComponentSystem::finalize()
{
    //Finalize
    //Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        mComponentSignalptrs[s]->finalize();
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        mComponentCptrs[c]->finalize();
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        mComponentQptrs[q]->finalize();
    }

    //loadStartValuesFromSimulation();
}

//! @brief This function will set the number of log data slots for preallocation and logDt based on the number of samples that should be loged
//! @param [in] nSamples The desired number of log data samples, <=0 (disableLog)
void ComponentSystem::setLogSettingsNSamples(int nSamples, double start, double stop, double sampletime)
{
    if (nSamples > 0)
    {
        enableLog();

        start = max(start, 0.0);  // Do not log data for negative time

        // Make sure we dont try to log more samples than we will simulate
        //! @todo may need som rounding tricks here
        if ( ((stop - start) / sampletime + 1) < nSamples )
        {
            mnLogSlots = size_t((stop - start) / sampletime + 1);
            std::stringstream ss;
            ss << "Requested nLogSamples: " << nSamples << " but this is more than the total simulation samples, limiting to: " << mnLogSlots;
            addWarningMessage(ss.str(), "toofewsamples");
        }
        else
        {
            mnLogSlots = nSamples;
        }

        //mLogTimeDt = (stop - start) / double(mnLogSlots); //! @todo remove this
        //mLastLogTime = start-mLogTimeDt; //! @todo remove this
    }
    else
    {
        disableLog();
    }
}


//! @brief This function will set the number of log data slots for preallocation and logDt based on a skip factor to the sample time
//! @param [in] factor The timestep skip factor, minimum 1.0, but if < 0 then disableLog
void ComponentSystem::setLogSettingsSkipFactor(double factor, double start, double stop,  double sampletime)
{
    if (factor > 0)
    {
        enableLog();
        //! @todo maybe only use integer factors
        //make sure factor is not less then 1.0
        factor = max(1.0, factor);
        mLogTimeDt = sampletime * factor;
        //mLastLogTime = start-mLogTimeDt;
        mnLogSlots = (size_t)((stop-start)/mLogTimeDt+0.5); //Round to nearest
        //! @todo FIXA /Peter
    }
    else
    {
        disableLog();
    }
}


//! @brief This function will set the number of log data slots for preallocation and logDt
//! @param [in] log_dt The desired log timestep, if log_dt < 0 then disableLog
void ComponentSystem::setLogSettingsSampleTime(double log_dt, double start, double stop,  double sampletime)
{
    if (log_dt > 0)
    {
        enableLog();
        // Make sure that we dont have log_dt lower than sampletime ( we cant log more then we calc)
        log_dt = max(sampletime,log_dt);
        mLogTimeDt = log_dt;
        //mLastLogTime = start-mLogTimeDt;
        mnLogSlots = size_t((stop-start)/log_dt+0.5); //Round to nearest
        //! @todo FIXA /Peter
    }
    else
    {
        disableLog();
    }
}

//! @brief Enable node data logging
void ComponentSystem::enableLog()
{
    mEnableLogData = true;
}


//! @brief Disable node data logging
void ComponentSystem::disableLog()
{
    mEnableLogData = false;

    // If log dissabled then free memory if something has been previously allocated
    mTimeStorage.clear();
    //mDataStorage.clear();
    mLogTheseTimeSteps.clear();

    mLogTimeDt = -1.0;
    //mLastLogTime = 0.0; //Initial valus should not matter, will be overwritten when selecting log amount
    mnLogSlots = 0;
    mLogCtr = 0;
}

vector<double> *ComponentSystem::getLogTimeVector()
{
    return &mTimeStorage;
}

//! @brief Simulates a vector of component systems in parallel, by assigning one or more system to each simluation thread
//! @param startT Start time for all systems
//! @param stopT Stop time for all systems
//! @param nDesiredThreads Desired number of threads (may change due to hardware limitations)
//! @param systemVector Vector of pointers to systems to simulate
bool SimulationHandler::simulateMultipleSystemsMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, vector<ComponentSystem*> &rSystemVector, bool noChanges)
{
#ifdef USETBB
    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);              //Calculate how many threads to actually use

//    for(size_t i=0; i<rSystemVector.size(); ++i)                         //Loop through the systems, set start time, log nodes and measure simulation time
//    {
//        double *pTime = rSystemVector.at(i)->getTimePtr();
//        *pTime = startT;
////        rSystemVector.at(i)->logTimeAndNodes(*pTime);                        //Log the first time step
//        rSystemVector.at(i)->logTimeAndNodes(0);                        //Log the first time step
//    }

    if(!noChanges)
    {
        mSplitSystemVector.clear();
        for(size_t i=0; i<rSystemVector.size(); ++i)                     //Loop through the systems, set start time, log nodes and measure simulation time
        {
            rSystemVector.at(i)->simulateAndMeasureTime(5);              //Measure time
        }
        sortSystemsByTotalMeasuredTime(rSystemVector);                   //Sort systems by total measured time
        mSplitSystemVector = distributeSystems(rSystemVector, nThreads); //Distribute systems evenly over split vectors
    }

    tbb::task_group *simTasks;                                          //Initialize TBB routines for parallel simulation
    simTasks = new tbb::task_group;
    for(size_t t=0; t < nThreads; ++t)                                  //Execute simulation
    {
        simTasks->run(taskSimWholeSystems(mSplitSystemVector[t], stopT));
    }
    simTasks->wait();                                                   //Wait for all tasks to finish
    delete(simTasks);

    bool aborted=false;
    for(size_t i=0; i<rSystemVector.size(); ++i)
    {
        aborted = aborted && rSystemVector[i]->wasSimulationAborted();
    }
    return !aborted;
#else
    // Use single core simulation if no TBB support
    return simulateMultipleSystems(stopT, rSystemVector);
#endif
}

AliasHandler::AliasHandler(ComponentSystem *pSystem)
{
    mpSystem = pSystem;
}

//! @todo maybe this should be the default version, right now search comp/port twice
bool AliasHandler::setVariableAlias(const std::string alias, const std::string compName, const std::string portName, const std::string varName)
{
    Component *pComp = mpSystem->getSubComponent(compName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(portName);
        if (pPort)
        {
            int id = pPort->getNodeDataIdFromName(varName);
            return setVariableAlias(alias, compName, portName, id);
        }
    }
    return false;
}

bool AliasHandler::setVariableAlias(const string alias, const string compName, const string portName, const int varId)
{
    if (!isNameValid(alias))
    {
        mpSystem->addErrorMessage("Invalid characters in requested alias name: "+alias);
        return false;
    }

    //! @todo must check if existing alias is set for the same component that already have it to avoid warning
    // Check if alias already exist
    if (hasAlias(alias))
    {
        string comp,port;
        int var;
        getVariableFromAlias(alias,comp,port,var);
        if ( (comp==compName) && (port==portName) && (var==varId) )
        {
            // We are setting the same alias again, skip without warning
            return true;
        }
        else
        {
            // The alias already exist somwhere else
            mpSystem->addErrorMessage("Alias: "+alias+" already exist");
            return false;
        }
    }

    if (mpSystem->hasReservedUniqueName(alias))
    {
        mpSystem->addErrorMessage("The alias: " + alias + " is already used as some other name");
        return false;
    }

    // Set the alias for the given component port and var
    Component *pComp = mpSystem->getSubComponent(compName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(portName);
        if (pPort)
        {
            // First unregister the old alias (if it exists)
            string prevAlias = pPort->getVariableAlias(varId);
            if (!prevAlias.empty())
            {
                //! @todo the remove will search for port agin all the way, maybe have a special remove to use when we know the port and id already
                removeAlias(prevAlias);
            }

            // If alias is non empty, set it
            if (!alias.empty())
            {
                //! @todo do we need to check if this is OK ??
                pPort->setVariableAlias(alias, varId);

                ParamOrVariableT data = {Variable, compName, portName};
                mAliasMap.insert(std::pair<string, ParamOrVariableT>(alias, data));
                mpSystem->reserveUniqueName(alias);
            }
            return true;
        }
    }
    mpSystem->addErrorMessage("Component or Port not found when setting alias");
    return false;
}

bool AliasHandler::setParameterAlias(const string alias, const string compName, const string parameterName)
{
    mpSystem->addErrorMessage("AliasHandler::setParameterAlias has not been implemented");
    return false;
}

void AliasHandler::componentRenamed(const string oldCompName, const string newCompName)
{
    std::map<std::string, ParamOrVariableT>::iterator it;
    for (it=mAliasMap.begin(); it!=mAliasMap.end(); ++it)
    {
        if (it->second.componentName == oldCompName)
        {
            string alias = it->first;
            ParamOrVariableT data = it->second;
            mAliasMap.erase(it);
            data.componentName = newCompName;

            // Re-insert data (with new comp name)
            mAliasMap.insert(std::pair<string, ParamOrVariableT>(alias, data));

            // Restart search for more components
            it = mAliasMap.begin();
        }
    }
}

void AliasHandler::portRenamed(const string compName, const string oldPortName, const string newPortName)
{
    mpSystem->addErrorMessage("AliasHandler::portRenamed has not been implemented");
}

void AliasHandler::componentRemoved(const string compName)
{
    std::map<std::string, ParamOrVariableT>::iterator it;
    for (it=mAliasMap.begin(); it!=mAliasMap.end(); ++it)
    {
        if (it->second.componentName == compName)
        {
            mAliasMap.erase(it);
            it = mAliasMap.begin(); //Restart search for more components
        }
    }
}

void AliasHandler::portRemoved(const string compName, const string portName)
{
    mpSystem->addErrorMessage("AliasHandler::portRemoved has not been implemented");
}

bool AliasHandler::hasAlias(const string alias)
{
    if (mAliasMap.count(alias)>0)
    {
        return true;
    }
    return false;
}

bool AliasHandler::removeAlias(const string alias)
{
    std::map<std::string, ParamOrVariableT>::iterator it = mAliasMap.find(alias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            Component *pComp = mpSystem->getSubComponent(it->second.componentName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(it->second.name);
                if (pPort)
                {
                    int id = pPort->getVariableIdByAlias(alias);
                    pPort->setVariableAlias("",id); //Remove variable alias
                }
            }
        }
        mAliasMap.erase(it);
        mpSystem->unReserveUniqueName(alias);
        return true;
    }
    return false;
}

std::vector<std::string> AliasHandler::getAliases() const
{
    vector<string> aliasNames;
    aliasNames.reserve(mAliasMap.size());

    std::map<std::string, ParamOrVariableT>::const_iterator it;
    for (it=mAliasMap.begin(); it!=mAliasMap.end(); ++it)
    {
        aliasNames.push_back(it->first);
    }
    return aliasNames;
}

void AliasHandler::getVariableFromAlias(const string alias, string &rCompName, string &rPortName, int &rVarId)
{
    // Clear return vars to indicate any failure
    rCompName.clear(); rPortName.clear(); rVarId=-1;

    // Search through map for specified alias
    std::map<std::string, ParamOrVariableT>::iterator it;
    it = mAliasMap.find(alias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            rCompName = it->second.componentName;
            rPortName = it->second.name;

            // Lookup varName from port
            Component* pComp = mpSystem->getSubComponent(rCompName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(rPortName);
                if (pPort)
                {
                    rVarId = pPort->getVariableIdByAlias(alias);
                }
            }
        }
    }
}

void AliasHandler::getVariableFromAlias(const string alias, string &rCompName, string &rPortName, string &rVarName)
{
    // Clear return vars to indicate any failure
    rCompName.clear(); rPortName.clear(); rVarName.clear();

    // Search through map for specified alias
    AliasMapT::iterator it = mAliasMap.find(alias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            rCompName = it->second.componentName;
            rPortName = it->second.name;

            // Lookup varName from port
            Component* pComp = mpSystem->getSubComponent(rCompName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(rPortName);
                if (pPort)
                {
                    int id = pPort->getVariableIdByAlias(alias);
                    rVarName = pPort->getNodeDataDescription(id)->name;
                }
            }
        }
    }
}

void AliasHandler::getParameterFromAlias(const string alias, string &rCompName, string &rParameterName)
{
    mpSystem->addErrorMessage("AliasHandler::getParameterFromAlias has not been implemented");
}
