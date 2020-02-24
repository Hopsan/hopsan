/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   ComponentSystem.cpp
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
#include <algorithm>
#include <map>

#include "ComponentSystem.h"
#include "HopsanEssentials.h"
#include "Quantities.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"
#include "CoreUtilities/MultiThreadingUtilities.h"
#include "CoreUtilities/HmfLoader.h"
#include "CoreUtilities/NumHopHelper.h"
#include "CoreUtilities/ConnectionAssistant.h"
#include "ComponentUtilities/num2string.hpp"

using namespace std;

#if (__cplusplus >= 201103L)
#ifdef _WIN32
#include <windows.h>
#endif
#include <functional>
#include <chrono>
#include <ctime>

#ifdef _WIN32
const long long g_Frequency = []() -> long long
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
}();


HighResClock::time_point HighResClock::now()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return time_point(duration(count.QuadPart * static_cast<rep>(period::den) / g_Frequency));
}
#endif //_WIN32

#endif //C++11

#if defined(HOPSANCORE_USEMULTITHREADING)
#include <atomic>
#include <mutex>
#include <thread>
#endif // multithreading

namespace {
//! @brief Figure out whether or not a vector contains a certain "object", exact comparison
//! @param[in] rVector Vector of objects
//! @param[in] rObj Object to find
//! @return Returns true if found else false
template<typename T>
bool vectorContains(const std::vector<T> &rVector, const T &rObj)
{
    typename std::vector<T>::const_iterator it;
    for(it=rVector.begin(); it!=rVector.end(); ++it)
    {
        if((*it) == rObj)
        {
            return true;
        }
    }
    return false;
}
} // anon namespace

namespace hopsan {

class ComponentSystemMultiThreadPrivates {
public:
    std::vector<double *> mvTimePtrs;
    std::vector< std::vector<Component*> > mSplitCVector;
    std::vector< std::vector<Component*> > mSplitQVector;
    std::vector< std::vector<Component*> > mSplitSignalVector;
    std::vector< std::vector<Node*> > mSplitNodeVector;
#if defined(HOPSANCORE_USEMULTITHREADING)
    std::mutex mStopMutex;
#endif

};


//Constructor
ComponentSystem::ComponentSystem() : Component(), mAliasHandler(this)
{
    mTypeName = "ComponentSystem";
    mTypeCQS = Component::UndefinedCQSType;
    mName = mTypeName; //Make sure initial name is same as typename
    mWarnIfUnusedSystemParameters = true;
    mDesiredTimestep = 0.001;
    mInheritTimestep = true;
    mKeepValuesAsStartValues = false;
    mRequestedNumLogSamples = 0; //This has to be 0 since we want logging to be disabled by default
    mRequestedLogStartTime = 0;
    mpMultiThreadPrivates = new ComponentSystemMultiThreadPrivates;
    mpNumHopHelper = 0;

    // Set default (disabled) values for log data
    disableLog();
}


ComponentSystem::~ComponentSystem()
{
    // Clear the contents of the system
    clear();
    delete mpMultiThreadPrivates;
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

double ComponentSystem::getLogStartTime() const
{
    return mRequestedLogStartTime;
}

void ComponentSystem::setLogStartTime(const double logStartTime)
{
    mRequestedLogStartTime = logStartTime;
}

//! @brief Returns the desired number of log samples
size_t ComponentSystem::getNumLogSamples() const
{
    return mRequestedNumLogSamples;
}

//! @brief Returns the number of actually logged data samples
//! @return Number of available logged data samples in storage
size_t ComponentSystem::getNumActuallyLoggedSamples() const
{
    // This assumes that the logCtr has been incremented after each saved log step
    return mLogCtr;
}


//! @brief Set the stop simulation flag to abort the initialization or simulation loops
//! @param[in] rReason An optional HString describing the reason for the stop
//! This method can be used by users e.g. GUIs to stop an start a initialization/simulation process
void ComponentSystem::stopSimulation(const HString &rReason)
{
    HString infoMsg;
    if (rReason.empty())
    {
        infoMsg = "Simulation was stopped at t="+to_hstring(mTime);
    }
    else
    {
        infoMsg = "Simulation was stopped at t="+to_hstring(mTime)+ " : "+rReason;
    }
    addInfoMessage(infoMsg);
    addCoreLogMessage(infoMsg);

#if defined(HOPSANCORE_USEMULTITHREADING)
    mpMultiThreadPrivates->mStopMutex.lock();
    mStopSimulation = true;
    mpMultiThreadPrivates->mStopMutex.unlock();
#else
    mStopSimulation = true;
#endif
    // Now propagate stop signal upwards, to parent
    if (mpSystemParent)
    {
        mpSystemParent->stopSimulation(""); // We use string version here to make sure sub system hierarchy is printed
    }
}

//! @brief Set the stop simulation flag to abort the initialization or simulation loops, (without messages being added)
//! @todo maybe we should only have with messages version
void ComponentSystem::stopSimulation()
{
#if defined(HOPSANCORE_USEMULTITHREADING)
    mpMultiThreadPrivates->mStopMutex.lock();
    mStopSimulation = true;
    mpMultiThreadPrivates->mStopMutex.unlock();
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
bool ComponentSystem::wasSimulationAborted() const
{
    return mStopSimulation;
}

//! @brief Adds a search path that can be used by its components to look for external files, e.g. area curves
//! @param [in] rSearchPath The search path to be added
void ComponentSystem::addSearchPath(HString searchPath)
{
    if (searchPath.empty()) {
        return;
    }

    // Strip trailing slash or back-slash
    while( !searchPath.empty() && ((searchPath.back() == '/') || (searchPath.back() == '\\')) )
    {
        searchPath = searchPath.substr(0,searchPath.size()-1);
    }

    // Prevent adding a search path that already exists
    for(size_t i=0; i<mSearchPaths.size(); ++i)
    {
        if(mSearchPaths[i] == searchPath) {
            return;
        }
    }
    addDebugMessage(HString("Adding asset searchPath: ")+searchPath);
    mSearchPaths.push_back(searchPath);
}


//! @brief Set, add or change a system parameter including all meta data
bool ComponentSystem::setOrAddSystemParameter(const HString &rName, const HString &rValue, const HString &rType, const HString &rDescription, const HString &rUnitOrQuantity, const bool force)
{
    bool success;
    HString quantity, bu;
    if(mpParameters->hasParameter(rName))
    {
        checkIfQuantityOrUnit(rUnitOrQuantity, quantity, bu);
        success = mpParameters->setParameter(rName, rValue, rDescription, quantity, bu, rType, force);
    }
    else
    {
        if (this->hasReservedUniqueName(rName) || !isNameValid(rName, "#"))
        {
            addErrorMessage("The desired system parameter name: "+rName+" is invalid or already used by something else");
            success=false;
        }
        else
        {
            checkIfQuantityOrUnit(rUnitOrQuantity, quantity, bu);
            success = mpParameters->addParameter(rName, rValue, rDescription, quantity, bu, rType, 0, force);
            if (success)
            {
                reserveUniqueName(rName,UniqueSysparamNameType);
            }
        }
    }

    return success;
}


//! @brief Set or change a system parameter including all meta data
bool ComponentSystem::setSystemParameter(const HString &rName, const HString &rValue, const HString &rType, const HString &rDescription, const HString &rUnitOrQuantity, const bool force)
{
    if(mpParameters->hasParameter(rName))
    {
        return setOrAddSystemParameter(rName, rValue, rType, rDescription, rUnitOrQuantity, force);
    }
    addErrorMessage("No such system parameter: "+rName);
    return false;
}

void ComponentSystem::unRegisterParameter(const HString &rName)
{
    Component::unRegisterParameter(rName);
    unReserveUniqueName(rName);
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
        HString modname = this->reserveUniqueName(pComponent->getName(), UniqueComponentNameType);
        pComponent->setName(modname);

        // Add to the cqs component vectors
        addSubComponentPtrToStorage(pComponent);

        // Set system parent and model system depth hierarchy
        pComponent->setSystemParent(this);
        pComponent->mModelHierarchyDepth = mModelHierarchyDepth+1; //Set the ModelHierarchyDepth counter

        // Go through the components ports and take ownership of any dummy nodes
        //! @todo what happens if I take ownership of an other systems components (shouldn't we take ownership of all ports nodes by default) Not sure!! especially difficult with system border nodes
        //! @todo maybe node ownership should be decided early in initialize instead to make this less complicated
        std::vector<Port*> ports = pComponent->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if (ports[i]->getPortType() < MultiportType)
            {
                // Dummy nodes have only one port (the port itself)
                if (ports[i]->getNodePtr()->getNumConnectedPorts() == 1)
                {
                    this->addSubNode(ports[i]->getNodePtr());
                }
            }
        }

        if (pComponent->isExperimental())
        {
            pComponent->addWarningMessage("This component is experimental!", "ExperimentalTag");
        }
        if (pComponent->isObsolete())
        {
            pComponent->addWarningMessage("This component is obsolete and will be removed in the future!", "ObsoleteTag");
        }
    }
    else
    {
        addErrorMessage("Trying to add NULL component to system");
    }
}


//! @brief Rename a sub component and automatically fix unique names
void ComponentSystem::renameSubComponent(const HString &rOldName, const HString &rNewName)
{
    // First find the post in the map where the old name resides, copy the data stored there
    SubComponentMapT::iterator it = mSubComponentMap.find(rOldName);
    if (it != mSubComponentMap.end())
    {
        // If found, erase old record
        Component* pTempComp = it->second;
        mSubComponentMap.erase(it);

        // Insert new (with new name)
        HString mod_new_name = this->reserveUniqueName(rNewName, UniqueComponentNameType);
        this->unReserveUniqueName(rOldName);
        mSubComponentMap.insert(pair<HString, Component*>(mod_new_name, pTempComp));

        // Rename aliases
        mAliasHandler.componentRenamed(rOldName, mod_new_name);

        // Now change the actual component name, without trying to do rename (we are in rename now, would cause infinite loop)
        pTempComp->mName = mod_new_name;
    }
    else
    {
        addErrorMessage("No component with old_name: "+rOldName+" found when renaming!");
    }
}


//! @brief Remove a dub component from a system, can also be used to actually delete the component
//! @param[in] rName The name of the component to remove from the system
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(const HString &rName, bool doDelete)
{
    Component* pComp = getSubComponent(rName);
    removeSubComponent(pComp, doDelete);
}


//! @brief Remove a sub component from a system, can also be used to actually delete the component
//! @param[in] pComponent A pointer to the component to remove
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(Component* pComponent, bool doDelete)
{
    HString compName = pComponent->getName();

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

    // Remove any component aliases
    mAliasHandler.componentRemoved(pComponent->getName());

    // Remove from storage
    removeSubComponentPtrFromStorage(pComponent);

    // Remove any dummy node ptrs
    //! @todo (shouldn't we remove ownership of all port nodes by default) Not sure!! especially difficult with system border nodes
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
//! @param [in] rDesiredName The desired name to reserve
//! @param [in] type The type of entity that the unique name represents
//! @returns The actual name reserved
HString ComponentSystem::reserveUniqueName(const HString &rDesiredName, const UniqeNameEnumT type)
{
    HString newname = this->determineUniqueComponentName(rDesiredName);
    mTakenNames.insert(std::pair<HString, UniqeNameEnumT>(newname, type));
    return newname;
}

//! @brief unReserves a unique name in the system
//! @param [in] rName The name to unreserve
void ComponentSystem::unReserveUniqueName(const HString &rName)
{
    mTakenNames.erase(rName);
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

    mSubComponentMap.insert(pair<HString, Component*>(pComponent->getName(), pComponent));
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

//! @brief Clear all the contents of a system (deleting any remaining components and connections)
void ComponentSystem::clear()
{
    // Remove and delete every subcomponent, one by one
    while (!mSubComponentMap.empty())
    {
        removeSubComponent((*mSubComponentMap.begin()).second, true);
    }

    // Remove the numhop storage if present
    if (mpNumHopHelper)
    {
        delete mpNumHopHelper;
        mpNumHopHelper = 0;
    }
}

#define USENEWSORTCODE
//! @brief Sorts a component vector
//! Components are sorted so that they are always simulated after the components they receive signals from. Algebraic loops can be detected, in that case this function does nothing.
bool ComponentSystem::sortComponentVector(std::vector<Component*> &rComponentVector)
{
    std::vector<Component*> newComponentVector;

#ifndef USENEWSORTCODE
    bool didSomething = true;
    while(didSomething)
    {
        didSomething = false;
        std::vector<Component*>::iterator it;
        //Loop through the unsorted signal component vector
        for(it=rComponentVector.begin(); it!=rComponentVector.end(); ++it)
        {
            // A pointer to an unsorted component
            Component* pUnsrtComp = (*it);
            //Ignore components that are already added to the new vector
            if(!vectorContains<Component*>(newComponentVector, pUnsrtComp))
            {
                bool readyToAdd=true;
                std::vector<Port*>::iterator itp;
                std::vector<Port*> portVector = (*it)->getPortPtrVector();
                // Ask each read port for its node, then ask the node for its write port component
                for(itp=portVector.begin(); itp!=portVector.end(); ++itp)
                {
                    // Take the port pointer (To make code easier to read)
                    Port *pPort = (*itp);
                    Component* pRequiredComponent=0;

                    bool isReadPort = pPort->getPortType() == ReadPortType || pPort->getPortType() == ReadMultiportType ||
                            ( (pUnsrtComp->getTypeName() == SUBSYSTEMTYPENAME) && (pPort->getInternalPortType() == ReadPortType)) ||
                            ( (pUnsrtComp->getTypeName() == CONDITIONALTYPENAME) && (pPort->getInternalPortType() == ReadPortType));
                    if ( isReadPort && pPort->isConnected() )
                    {
                        pRequiredComponent = pPort->getNodePtr()->getWritePortComponentPtr();
                    }
                    if(pRequiredComponent && (pRequiredComponent->getTypeName() != "SignalUnitDelay"))
                    {
                        if(pRequiredComponent->mpSystemParent == this)
                        {
                            if(!vectorContains<Component*>(newComponentVector, pRequiredComponent) &&
                                    vectorContains<Component*>(rComponentVector, pRequiredComponent)
                                    /*requiredComponent->getTypeCQS() == pPort->getComponent()->getTypeCQS()*//*Component::S*/)
                            {
                                readyToAdd = false;     //Depending on normal component which has not yet been added
                            }
                        }
                        else
                        {
                            if(!vectorContains<Component*>(newComponentVector, pRequiredComponent->mpSystemParent) &&
                                    (pRequiredComponent->mpSystemParent->getTypeCQS() == pPort->getComponent()->getTypeCQS()) &&
                                    vectorContains<Component*>(rComponentVector,pRequiredComponent->mpSystemParent))
                            {
                                readyToAdd = false;     //Depending on subsystem component which has not yet been added
                            }
                        }
                    }
                }
                // Add the component if all required write port components was already added
                if(readyToAdd)
                {
                    newComponentVector.push_back(pUnsrtComp);
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
            HString ss;
            std::vector<Component*>::iterator it;
            for(it=newComponentVector.begin(); it!=newComponentVector.end(); ++it)
            {
                ss += (*it)->getName()+"\n";                                                                                               //DEBUG
            }
            addDebugMessage("Sorted components successfully!\nSignal components will be simulated in the following order:\n" + ss);
        }
    }
    else    //Something went wrong, all components were not moved. This is likely due to an algebraic loop.
    {
        addErrorMessage("Initialize: Algebraic loops was found, signal components could not be sorted.");
        if(!newComponentVector.empty())
        {
            addInfoMessage("Last component that was successfully sorted: " + newComponentVector.back()->getName());
        }
        addInfoMessage("Initialize: Hint: Use unit delay components to resolve loops.");
        return false;
    }
#else
    bool didSomething = true;
    while(didSomething)
    {
        didSomething = false;

        // Loop through the unsorted component vector
        for(size_t c=0; c<rComponentVector.size(); ++c)
        {
            // A pointer to an unsorted component
            Component* pUnsrtComp = rComponentVector[c];
            //Ignore components that are already added to the new vector
            if(!vectorContains<Component*>(newComponentVector, pUnsrtComp))
            {
                bool readyToAdd=true;
                std::vector<Port*> portVector = pUnsrtComp->getPortPtrVector();
                // Ask each read port for its node, then ask the node for its write port component
                for(size_t p=0; p<portVector.size(); ++p)
                {
                    // Take the port pointer (To make code easier to read)
                    Port *pPort = portVector[p];
                    Component* pRequiredComponent=0;

                    SortHintEnumT sortHint = pPort->getSortHint();
                    if ((pUnsrtComp->getTypeName() == HOPSAN_BUILTIN_TYPENAME_SUBSYSTEM) ||
                        (pUnsrtComp->getTypeName() == HOPSAN_BUILTIN_TYPENAME_CONDITIONALSUBSYSTEM))
                    {
                        sortHint = pPort->getInternalSortHint();
                    }

                    if ( (sortHint == Destination) && pPort->isConnected() )
                    {
                        Port *pSourcePort = pPort->getNodePtr()->getSortOrderSourcePort();
                        if (pSourcePort && pSourcePort->getComponent())
                        {
                            pRequiredComponent = pSourcePort->getComponent();
                        }
                    }
                    if(pRequiredComponent /*&& (pRequiredComponent->getTypeName() != "SignalUnitDelay")*/)
                    {
                        if(pRequiredComponent->mpSystemParent == this)
                        {
                            if(!vectorContains<Component*>(newComponentVector, pRequiredComponent) &&
                                    vectorContains<Component*>(rComponentVector, pRequiredComponent))
                            {
                                readyToAdd = false;     //Depending on normal component which has not yet been added
                            }
                        }
                        else
                        {
                            if(!vectorContains<Component*>(newComponentVector, pRequiredComponent->mpSystemParent) &&
                                    (pRequiredComponent->mpSystemParent->getTypeCQS() == pPort->getComponent()->getTypeCQS()) &&
                                    vectorContains<Component*>(rComponentVector,pRequiredComponent->mpSystemParent))
                            {
                                readyToAdd = false;     //Depending on subsystem component which has not yet been added
                            }
                        }
                    }
                }
                // Add the component if all required write port components was already added
                if(readyToAdd)
                {
                    newComponentVector.push_back(pUnsrtComp);
                    didSomething = true;
                }
            }
        }
    }

    if(newComponentVector.size() == rComponentVector.size())   //All components moved to new vector = success!
    {
        if(newComponentVector.size() > 0 && newComponentVector[0]->getTypeCQS() == SType)
        {
            HString names;
            for(size_t c=0; c<newComponentVector.size(); ++c)
            {
                names += newComponentVector[c]->getName()+"\n";
            }
            addDebugMessage("Sorted components successfully!\nSignal components will be simulated in the following order:\n" + names);
        }
        rComponentVector.swap(newComponentVector);
    }
    else    //Something went wrong, all components were not moved. This is likely due to an algebraic loop.
    {
        addErrorMessage("Initialize: Algebraic loops was found, signal components could not be sorted.");
        if(!newComponentVector.empty())
        {
            addInfoMessage("Last component that was successfully sorted: " + newComponentVector.back()->getName());
        }
        addInfoMessage("Initialize: Hint: Use unit delay components to resolve loops.");
        return false;
    }
#endif

    return true;
}


//! @brief Overloaded function that behaves slightly different when determining unique port names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports don't have the same name as a subcomponent
HString ComponentSystem::determineUniquePortName(const HString &rPortname)
{
    return this->reserveUniqueName(rPortname, UniqueSysportNameTyp);
}

//! @brief Overloaded function that behaves slightly different when determining unique component names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports don't have the same name as a subcomponent
//! @todo the determineUniquePortNAme and ComponentName looks VERY similar maybe we could use the same function for both
HString ComponentSystem::determineUniqueComponentName(const HString &rName) const
{
    return findUniqueName<TakenNamesMapT>(mTakenNames, rName);
}

bool ComponentSystem::hasReservedUniqueName(const HString &rName) const
{
    return (mTakenNames.find(rName) != mTakenNames.end());
}


//! @brief Get a Component ptr to the component with supplied name, can also return a ptr to self if no subcomponent found but systemport with name found
//! @details For this to work we need to make sure that the sub components and systemports have unique names
Component* ComponentSystem::getSubComponentOrThisIfSysPort(const HString &rName)
{
    // First try to find among subcomponents
    Component *tmp = getSubComponent(rName);
    if (tmp == 0)
    {
        // Now try to find among systemports
        Port* pPort = this->getPort(rName);
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


Component* ComponentSystem::getSubComponent(const HString &rName) const
{
    SubComponentMapT::const_iterator it = mSubComponentMap.find(rName);
    if (it != mSubComponentMap.end())
    {
        return it->second;
    }
    else
    {
        addCoreLogMessage("ComponentSystem::getSubComponent(): The requested component does not exist.");
        return 0;
    }
}

const std::vector<Component *> ComponentSystem::getSubComponents() const
{
    vector<Component *> ptrs;
    SubComponentMapT::const_iterator it;
    for (it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
    {
        ptrs.push_back(it->second);
    }

    return ptrs;
}


ComponentSystem* ComponentSystem::getSubComponentSystem(const HString &rName) const
{
    return dynamic_cast<ComponentSystem*>(getSubComponent(rName));
}


std::vector<HString> ComponentSystem::getSubComponentNames() const
{
    //! @todo for now create a vector of the component names, later maybe we should return a pointer to the real internal map
    vector<HString> names;
    SubComponentMapT::const_iterator it;
    for (it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
    {
        names.push_back(it->first);
    }

    return names;
}

//! @brief Check if a system has a subcomponent with given name
//! @param [in] rName The name to check for
//! @returns true or false
bool ComponentSystem::haveSubComponent(const HString &rName) const
{
    return (mSubComponentMap.count(rName) > 0);
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


//! @brief Add a node as subnode in the system, if the node is already owned by someone else, transfer ownership to this system
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
void ComponentSystem::preAllocateLogSpace()
{
    bool success = true;
    //    //cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;
    //    this->setLogSettingsNSamples(nSamples, startT, stopT, mTimestep);
    //! @todo Fix /Peter
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
                // Abort if we are told to stop or if memory allocation fails
                if (mStopSimulation || !success)
                    break;

                // Prepare the node log data allocation and determine if loggings should be on
                //! @todo What if we want to use one of the other ways of setting logsample time steps

                // Now try to allocate log memory for each node
                try
                {
                    // If the node is in a read port and if that port is not connected (node only have one connected port)
                    // Then we should disable logging for that node as logging the start value does not make sense
                    if ( ((*it)->getNumConnectedPorts() < 2) && ((*it)->getNumberOfPortsByType(ReadPortType) == 1) )
                    {
                        (*it)->setDoLogIfEnabled(false);
                    }
                    else
                    {
                        (*it)->setDoLogIfEnabled(true);
                        (*it)->preAllocateLogSpace(mnLogSlots);
                    }
                    success = true;
                }
                catch (exception &e)
                {
                    //cout << "preAllocateLogSpace: Standard exception: " << e.what() << endl;
                    addErrorMessage("Failed to allocate log data memory, try reducing the amount of log data", "FailedMemoryAllocation");
                    (*it)->setDoLogIfEnabled(false);
                    success = false;
                }
            }
        }
        catch (exception &e)
        {
            addErrorMessage("Failed to allocate log data memory, try reducing the amount of log data", "FailedMemoryAllocation");
            disableLog();
            success = false;
        }
    }

    // If we failed to allocate log memory then stop simulation
    if (!success)
    {
        stopSimulation("Failed to allocate log memory");
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
bool ComponentSystem::renameParameter(const HString &rOldName, const HString &rNewName)
{
    if (hasReservedUniqueName(rNewName))
    {
        addErrorMessage("The desired system parameter name: "+rNewName+" is already used by something else");
    }
    else if (mpParameters->renameParameter(rOldName, rNewName))
    {
        unReserveUniqueName(rOldName);
        reserveUniqueName(rNewName);

        // Now try to change the system parameter value in all sub components
        SubComponentMapT::iterator scmit;
        for (scmit=mSubComponentMap.begin(); scmit!=mSubComponentMap.end(); ++scmit)
        {
            Component* pComponent = scmit->second;
            const std::vector<ParameterEvaluator*> *pParams = pComponent->mpParameters->getParametersVectorPtr();
            for (size_t p=0; p<pParams->size(); ++p)
            {
                ParameterEvaluator *pParam = pParams->at(p);
                // If we find a parameter with the old system par name as value then change that value
                if (pParam->getValue() == rOldName)
                {
                    pComponent->setParameterValue(pParam->getName(), rNewName);
                }
            }
        }

        return true;
    }
    return false;
}

std::list<HString> ComponentSystem::getModelAssets() const
{
    // Get any system parameter assets
    std::list<HString> assets = Component::getModelAssets();
    // If this is an external system, then also consider the external model as an asset
    if (isExternalSystem())
    {
        assets.push_back(getExternalModelFilePath());
    }
    // Append assets from sub components
    SubComponentMapT::const_iterator compIt;
    for(compIt = mSubComponentMap.begin(); compIt!=mSubComponentMap.end(); ++compIt)
    {
        std::list<HString> sub = compIt->second->getModelAssets();
        assets.splice(assets.end(), sub);
    }
    return assets;
}

//! @brief Adds a transparent SubSystemPort
Port* ComponentSystem::addSystemPort(HString portName, const HString &rDescription)
{
    // Force default port name p, if nothing else specified
    if (portName.empty())
    {
        portName = "p";
    }

    return addPort(portName, SystemPortType, "NodeEmpty", rDescription, Port::Required);
}


//! @brief Rename system port
HString ComponentSystem::renameSystemPort(const HString &rOldname, const HString &rNewname)
{
    HString newmodename = renamePort(rOldname,rNewname);
    if (newmodename != rOldname)
    {
        unReserveUniqueName(rOldname);
    }
    return newmodename;
}


//! @brief Delete a System port from the component
//! @param [in] rName The name of the port to delete
void ComponentSystem::deleteSystemPort(const HString &rName)
{
    deletePort(rName);
    unReserveUniqueName(rName);
}


//! @brief Set the type C, Q, or S of the subsystem
void ComponentSystem::setTypeCQS(CQSEnumT cqs_type, bool doOnlyLocalSet)
{
    // If we have a system parent, then tell it to change our CQS type
    if ( !this->isTopLevelSystem() && !doOnlyLocalSet )
    {
        //Request change by our parent (some parent changes are needed)
        mpSystemParent->changeSubComponentSystemTypeCQS(mName, cqs_type);
    }
    else
    {
        switch (cqs_type)
        {
        case Component::CType :
            mTypeCQS = Component::CType;
            for(size_t i=0; i<mPortPtrVector.size(); ++i)   //C-type, create start node for all power ports
            {
                if(mPortPtrVector[i]->getInternalPortType() == PowerPortType)
                {
                    Node *pStartNode = mPortPtrVector[i]->getStartNodePtr();
                    if (!pStartNode || pStartNode->getNodeType() == "NodeEmpty")
                    {
                        mPortPtrVector[i]->createStartNode(mPortPtrVector[i]->getNodeType());
                    }
                }
            }
            break;

        case Component::QType :
            mTypeCQS = Component::QType;
            for(size_t i=0; i<mPortPtrVector.size(); ++i)   //Q-type, remove start node for all powerports
            {
                if(mPortPtrVector[i]->getInternalPortType() == PowerPortType)
                {
                    mPortPtrVector[i]->eraseStartNode();
                }
            }
            break;

        case Component::SType :
            mTypeCQS = Component::SType;
            for(size_t i=0; i<mPortPtrVector.size(); ++i)   //S-type, remove start node for all powerports
            {
                if(mPortPtrVector[i]->getInternalPortType() == PowerPortType)
                {
                    mPortPtrVector[i]->eraseStartNode();
                }
            }
            break;

        case Component::UndefinedCQSType :
            mTypeCQS = Component::UndefinedCQSType;
            break;

        default :
            addWarningMessage("Specified type: "+getTypeCQSString()+" does not exist!, System CQStype unchanged");
        }
    }
}

//! @brief Change the CQS type of a stored subsystem component
bool ComponentSystem::changeSubComponentSystemTypeCQS(const HString &rName, const CQSEnumT newType)
{
    //First get a pointer to the system component
    ComponentSystem* pSubSystem = getSubComponentSystem(rName);
    if (pSubSystem)
    {
        //Remove current pointer
        removeSubComponentPtrFromStorage(pSubSystem);

        //Change cqsType locally in the subcomponent, make sure to set true to avoid looping back to this rename
        pSubSystem->setTypeCQS(newType, true);

        //re-add to pointer system
        addSubComponentPtrToStorage(pSubSystem);

        return true;
    }
    return false;
}

//! @brief This function automatically determines the CQS type depending on the what has been connected to the systemports
//! @todo This function will go through all connected ports every time it is run, maybe a quicker version would only be run on the port being connected or disconnected, in the connect and disconnect function
void ComponentSystem::determineCQSType()
{
    size_t c_ctr=0;
    size_t q_ctr=0;
    size_t s_ctr=0;

    PortPtrMapT::iterator ppmit;
    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        //! @todo I don't think that I really need to ask for ALL connected subports here, as it is actually only the component that is directly connected to the system port that is interesting
        //! @todo this means that I will be able to UNDO the Port getConnectedPorts madness, maybe, if we don't want it in some other place
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

    // Ok now lets determine if we have a valid CQS type or not
    if ( (c_ctr > 0) && (q_ctr == 0) )
    {
        setTypeCQS(CType);
    }
    else if ( (q_ctr > 0) && (c_ctr == 0) )
    {
        setTypeCQS(QType);
    }
    else if ( (s_ctr > 0) && (c_ctr==0) && (q_ctr==0) )
    {
        setTypeCQS(SType);
    }
    else
    {
        setTypeCQS(UndefinedCQSType);
    }
}

bool ComponentSystem::isTopLevelSystem() const
{
    return (mpSystemParent==0);
}

bool ComponentSystem::isExternalSystem() const
{
    return !mExternalModelFilePath.empty();
}

void ComponentSystem::setExternalModelFilePath(const HString &rPath)
{
    mExternalModelFilePath = rPath;
}

HString ComponentSystem::getExternalModelFilePath() const
{
    return mExternalModelFilePath;
}


//! @brief Connect two components, string version
//! @param [in] compname1 The name of the first component
//! @param [in] portname1 The name of the port on the first component
//! @param [in] compname2 The name of the second component
//! @param [in] portname2 The name of the port on the second component
//! @returns True if success else False
bool ComponentSystem::connect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2)
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

    // Check if components have specified ports
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

    // Ok components and ports exist, lets attempt the connect
    return connect( pPort1, pPort2 );
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


    // Prevent connection if ports are already connected to each other, but we make an exception if one of the ports is aread multiport (to allow connecting same signal multiple times to scopes)
    if (!((pPort1->getPortType() == ReadMultiportType) || ((pPort2->getPortType() == ReadMultiportType))) && pPort1->isConnectedTo(pPort2) )
    {
        addErrorMessage("Port: " + pComp1->getName()+"::"+pPort1->getName() + "  is already connected to: " + pComp2->getName()+"::"+pPort2->getName());
        return false;
    }

    // Prevent cross connection between systems
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
        // Now lets find out which of the ports that is a blank systemport
        Port *pBlankSysPort=0;
        Port *pOtherPort=0;

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

        // Handle multiport connection success or failure
        connAssist.ifMultiportCleanupAfterConnect(pOtherPort, &pActualPort, sucess);
    }
    // Non of the ports  are blank systemports
    else
    {
        // Check if we are connecting multiports, in that case add new subport, remember original portPointer though so that we can clean up if failure
        Port *pActualPort1 = connAssist.ifMultiportAddSubport(pPort1);
        Port *pActualPort2 = connAssist.ifMultiportAddSubport(pPort2);

        sucess = connAssist.mergeNodeConnection(pActualPort1, pActualPort2);

        // Handle multiport connection success or failure
        connAssist.ifMultiportCleanupAfterConnect(pPort1, &pActualPort1, sucess);
        connAssist.ifMultiportCleanupAfterConnect(pPort2, &pActualPort2, sucess);
    }

    // Abort connection if there was a connect failure
    if (!sucess)
    {
        return false;
    }

    // Update the CQS type, we need to run this always even if not directly connecting to a systemport
    // In some cases the port we are connecting to may be indirectly connected to the systemport
    this->determineCQSType();

    // Also Update parent CQS-type
    //! @todo we should only do this if we are actually connected directly to our parent, but I don't know what will take the most time, to check if we are connected to parent or to just refresh parent
    if (!this->isTopLevelSystem())
    {
        mpSystemParent->determineCQSType();
    }

    addDebugMessage("Connected: {"+pComp1->getName()+"::"+pPort1->getName()+"} and {"+pComp2->getName()+"::"+pPort2->getName()+"}", "succesfulconnect");
    return true;
}





//! @brief Disconnect two ports, string version
//! @param [in] compname1 The name of the first component
//! @param [in] portname1 The name of the port on the first component
//! @param [in] compname2 The name of the second component
//! @param [in] portname2 The name of the port on the second component
//! @returns True if success, False if failed
bool ComponentSystem::disconnect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2)
{
    Component *pComp1 = getSubComponentOrThisIfSysPort(compname1);
    Component *pComp2 = getSubComponentOrThisIfSysPort(compname2);

    if ( (pComp1!=0) && (pComp2!=0) )
    {
        Port *pPort1 = pComp1->getPort(portname1);
        Port *pPort2 = pComp2->getPort(portname2);

        if ( (pComp1!=0) && (pComp2!=0) )
        {
            return disconnect(pPort1, pPort2);
        }
    }

    addDebugMessage("Disconnect: Could not find either " +compname1+"->"+portname1+" or "+compname2+"->"+portname2);
    return false;
}

//! @brief Disconnects two ports and remove node if no one is using it any more.
//! @param pPort1 Pointer to first port
//! @param pPort2 Pointer to second port
bool ComponentSystem::disconnect(Port *pPort1, Port *pPort2)
{
    // First check if ports not null
    if (pPort1 && pPort2)
    {
        HString msgName1 = pPort1->getComponent()->getName()+"::"+pPort1->getName();
        HString msgName2 = pPort2->getComponent()->getName()+"::"+pPort2->getName();

        ConnectionAssistant disconnAssistant(this);
        //! @todo some more advanced error handling
        if (pPort1->isConnectedTo(pPort2))
        {
            bool success = false;

            // If non of the ports are multiports
            if ( !(pPort1->isMultiPort() || pPort2->isMultiPort()) )
            {
                success = disconnAssistant.splitNodeConnection(pPort1, pPort2);

                // In case we did disconnect an ordinary port and a subport in a multiport, then clear the subport from the multiport
                disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort1->getParentPort(), &pPort1, success);
                disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort2->getParentPort(), &pPort2, success);
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

                // Handle multiport connection success or failure
                disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort1, &pActualPort1, success);
                disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort2, &pActualPort2, success);
            }

            disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort1);
            disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort2);
            //! @todo maybe incorporate the clear checks into delete node and unmerge

            // Update the CQS type, we need to run this always even if not directly connecting to a systemport
            // In some cases the port we are connecting to may be indirectly connected to the systemport
            this->determineCQSType();
            // Also update parent CQS-type
            //! @todo we should only do this if we are actually connected directly to our parent, but I don't know what will take the most time, to check if we are connected to parent or to just always refresh parent
            if (!this->isTopLevelSystem())
            {
                this->mpSystemParent->determineCQSType();
            }

            HString msg;
            msg = "Disconnected: {"+msgName1+"} and {"+msgName2+"}";
            addDebugMessage(msg, "successful disconnect");

            return success;
        }
        else
        {
            addErrorMessage("When attempting disconnect: Port: "+msgName1+" is not connected to: "+msgName2);
            return false;
        }
        addFatalMessage("When attempting disconnect: One of the ports is NULL");
    }
    return false;
}


//! @brief Sets the desired time step in a component system.
//! @param[in] timestep New desired time step
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
        if (!(mComponentSignalptrs[s]->isComponentSystem())/* && mComponentSignalptrs[s]->doesInheritTimestep()*/)
        {
            mComponentSignalptrs[s]->setTimestep(timestep);
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (!(mComponentCptrs[c]->isComponentSystem())/* && mComponentCptrs[c]->doesInheritTimestep()*/)
        {
            mComponentCptrs[c]->setTimestep(timestep);
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (!(mComponentQptrs[q]->isComponentSystem())/* && mComponentQptrs[q]->doesInheritTimestep()*/)
        {
            mComponentQptrs[q]->setTimestep(timestep);
        }
    }
}


//! @brief Figure out which timestep to use for all sub components
//! @param componentPtrs Vector with pointers to all sub components
void ComponentSystem::adjustTimestep(vector<Component*> componentPtrs)
{
    for (size_t c=0; c < componentPtrs.size(); ++c)
    {
        // Check if component should inherit timestep from its parent system (this system)
        if(componentPtrs[c]->doesInheritTimestep())
        {
            componentPtrs[c]->setTimestep(mTimestep);
        }
        // Else use the desired timestep, and adjust it if necessary
        else
        {
            // Prevent negative or zero timesteps
            double subTs = componentPtrs[c]->mDesiredTimestep;
            if (subTs <= 0.0)
            {
                subTs = mTimestep;
            }
            componentPtrs[c]->setTimestep(subTs);
        }
    }
}

size_t limitNumLogSlotsToLogOrSimTimeInterval(const double simStartT, const double simStopT, const double simTs, const double logStartT, const size_t nRequestedLogSamples)
{
    double startT = max(simStartT, logStartT);
    // Saturate startT to stopT to avoid underflow in size_t if someone enters a logT that is higher the startT
    if (startT > simStopT)
    {
        startT = simStopT;
    }

    // Make sure we don't try to log more samples than we will simulate
    //! @todo may need some rounding tricks here
    if ( ((simStopT - startT) / simTs + 1) < nRequestedLogSamples )
    {
        return size_t( (simStopT - startT) / simTs + 1);

    }
    else
    {
        return nRequestedLogSamples;
    }
}

void ComponentSystem::setupLogSlotsAndTs(const double simStartT, const double simStopT, const double simTs)
{
    mnLogSlots = limitNumLogSlotsToLogOrSimTimeInterval(simStartT, simStopT, simTs, mRequestedLogStartTime, mRequestedNumLogSamples);
    if (mnLogSlots != mRequestedNumLogSamples)
    {
        addWarningMessage("Requested nLogSamples: "+to_hstring(mRequestedNumLogSamples)+" but this is more than the total number of simulation samples, limiting to: "+to_hstring(mnLogSlots), "toofewsamples");
    }

    if (mnLogSlots > 0)
    {
        enableLog();

        // We do not want to log before simStartT
        const double logStartT = max(simStartT,mRequestedLogStartTime);

        // Calc logDt and
        mLogTimeDt = (simStopT-logStartT)/double(mnLogSlots-1);

        // Figure out at which samples logging should happen
        double logT=logStartT;
        double simT=simStartT;

        mLogTheseTimeSteps.clear();
        mLogTheseTimeSteps.reserve(mnLogSlots);

        // Figure out the first simulation step to log (the one where simT >= logT)
        size_t n = size_t((logT-simT)/simTs+0.5);
        mLogTheseTimeSteps.push_back(n);
        // Fast forward simT
        simT += double(n)*simTs;

        // Now Calculate which additional simulation steps should be logged
        while (mLogTheseTimeSteps.size() < mnLogSlots)
        {
            logT += mLogTimeDt;
            n = size_t((logT-simT)/simTs+0.5);
            simT += double(n)*simTs;

            //cout << "SimT: " << simT << " logT: " << logT << " logT-simT: " << logT-simT << endl;
            mLogTheseTimeSteps.push_back(mLogTheseTimeSteps.back() + n);
        }

        //! @todo sanity check on log slots
        if (mnLogSlots != mLogTheseTimeSteps.size())
        {
            cout << "Error: mnLogSlots: " << mnLogSlots << " mLogTheseTimeSteps.size(): " << mLogTheseTimeSteps.size() << endl;
        }

        //        //cout << "n: " << n << endl;
        //        cout << "mNumSimulationSteps: " << size_t((stopT-logStartT)/Ts+0.5) << endl;
        //        cout << "mLastStepToLog: " << mLogTheseTimeSteps.back() << endl;
        //        cout << "mLogTimeDt: " << mLogTimeDt << " mTimeStepsToLog.size(): " << mLogTheseTimeSteps.size() << endl;
        //    for (int i=0; i<mTimeStepsToLog.size(); ++i)
        //    {
        //        cout << mTimeStepsToLog[i] << " ";
        //    }
        //    cout << endl;
    }
    else
    {
        disableLog();
    }
}


//! @brief Returns whether or not to keep node values instead of over writing with defaultStartValues
bool ComponentSystem::keepsValuesAsStartValues()
{
    return mKeepValuesAsStartValues;
}


//! @brief Set if node data values should be used as start values instead of the default start values or expressions
//! @details If this is true, start values will not be loaded, if it is false default start values will be loaded
//! @param[in] tf true or false, whether to keep node values instead of over writing with defaultStartValues
void ComponentSystem::setKeepValuesAsStartValues(bool tf)
{
    mKeepValuesAsStartValues = tf;
}


//! @brief Checks that everything is OK before simulation
//! @returns true if everything is OK, else false (simulation not permitted)
bool ComponentSystem::checkModelBeforeSimulation()
{
    // Make sure that there are no components or systems with an undefined cqs_type present
    if (mComponentUndefinedptrs.size() > 0)
    {
        for (size_t i=0; i<mComponentUndefinedptrs.size(); ++i)
        {
            addErrorMessage("The Component:  "+mComponentUndefinedptrs[i]->getName()+"  has an invalid CQS-Type:  "+mComponentUndefinedptrs[i]->getTypeCQSString());
        }
        return false;
    }

    // Check this systems own SystemPorts are connected (if required, they must be)
    vector<Port*> ports = getPortPtrVector();
    for (size_t i=0; i<ports.size(); ++i)
    {
        if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
        {
            addErrorMessage("Port:  "+ports[i]->getName()+"  on SystemComponent:  "+getName()+"  is not connected!");
            return false;
        }
        else if( ports[i]->isConnected() )
        {
            if(ports[i]->getNodePtr()->getNumberOfPortsByType(PowerPortType) == 1)
            {
                addErrorMessage("Port:  "+ports[i]->getName()+"  on Component:  "+ports[i]->getComponentName()+"  is connected to a node with only one attached power port!");
                return false;
            }
        }
    }

    // Generate a list of all system parameters (constants), to check if any are unused
    std::vector<HString> unusedSysParNames;
    const std::vector<ParameterEvaluator*> *pSysParameters = getParametersVectorPtr();
    unusedSysParNames.reserve(pSysParameters->size());
    for (size_t sp=0; sp<pSysParameters->size(); ++sp)
    {
        // We want to ignore those containing # as they are most likely start values in interface ports
        const HString& rName = pSysParameters->at(sp)->getName();
        if (!rName.containes('#'))
        {
            unusedSysParNames.push_back(rName);
        }
    }

    // Check all subcomponents to make sure that all requirements for simulation are met
    // scmit = The subcomponent map iterator
    SubComponentMapT::iterator scmit = mSubComponentMap.begin();
    for ( ; scmit!=mSubComponentMap.end(); ++scmit)
    {
        Component* pComp = scmit->second; //Component pointer
        if(pComp->isDisabled())
            continue;

        // Check that ALL ports that MUST be connected are connected
        vector<Port*> ports = pComp->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
            {
                addErrorMessage("Port:  "+ports[i]->getName()+"  on Component:  " + pComp->getName() + "  is not connected!");
                return false;
            }
            else if( ports[i]->isConnected() )
            {
                size_t numPP = ports[i]->getNodePtr()->getNumberOfPortsByType(PowerPortType);
                if (ports[i]->isInterfacePort() && ports[i]->getPortType()==PowerPortType)
                {
                    if( numPP > 0 && numPP < 3)
                    {
                        addErrorMessage("InterfacePort:  "+ports[i]->getName()+"  on Component:  "+ports[i]->getComponentName()+"  is connected to a node with only two power ports!");
                        return false;
                    }
                }
                else if(numPP == 1)
                {
                    addErrorMessage("Port:  "+ports[i]->getName()+"  on Component:  "+ports[i]->getComponentName()+"  is connected to a node with only one power port!");
                    return false;
                }
            }
        }

        // Check parameters in subcomponents. Note! this will also evaluate the parameter strings, but that should be OK
        HString errParName;
        if(!pComp->checkParameters(errParName))
        {
            HString val;
            pComp->getParameterValue(errParName, val);
            addErrorMessage("The parameter:  "+errParName+"  in System:  "+getName()+"  and Component:  "+pComp->getName()+" with value:  "+val+"  could not be evaluated!");
            return false;
        }

        // Check if component uses a system parameter and remove it from the unused list (if not already removed)
        const std::vector<ParameterEvaluator*> *pCompParameters = pComp->getParametersVectorPtr();
        for(size_t p=0; p<pCompParameters->size(); ++p)
        {
            // Break the loop if we do not have any system parameter names to check
            if (unusedSysParNames.empty())
            {
                break;
            }

            // If a system parameter is used in the component, then erase it from the list.
            // To match only complete parameter names, prepend and append a space to the expression and replace operators by a space.
            // Then find parameter names in expression.
            HString expression = (" " + pCompParameters->at(p)->getValue() + " ")
                    .replace("+", " ")
                    .replace("-", " ")
                    .replace("*", " ")
                    .replace("/", " ")
                    .replace("^", " ")
                    .replace("(", " ")
                    .replace(")", " ")
                    .replace("<", " ")
                    .replace(">", " ")
                    .replace("|", " ")
                    .replace("&", " ");
            for(std::vector<HString>::iterator itp = unusedSysParNames.begin(); itp!=unusedSysParNames.end();)
            {
                if (expression.find(" "+(*itp)+" ")!=std::string::npos)
                {
                    itp =  unusedSysParNames.erase(itp);
                } else {
                    ++itp;
                }
            }

        }

        // Check parameters in system. Note! this will also evaluate the parameter strings, but that should be OK
        errParName.clear();
        if(!checkParameters(errParName))
        {
            addErrorMessage("The system parameter:  "+errParName+"  in System:  "+getName()+"  can not be evaluated, it maybe depend on a deleted system parameter.");
            return false;
        }

        // Recurse testing into subsystems
        if (pComp->isComponentSystem())
        {
            if (!pComp->checkModelBeforeSimulation())
            {
                return false;
            }
        }

        //! @todo check that all C-component required ports are connected to Q-component ports
    }

    // Add warning message if at least one system parameter is unused
    if(mWarnIfUnusedSystemParameters && (unusedSysParNames.size() > 0))
    {
        std::stringstream ss;
        ss << "The following system parameters are not used by any sub component:";
        for(size_t p=0; p<unusedSysParNames.size(); ++p)
        {
            ss << "\n" << unusedSysParNames[p].c_str();
        }
        addWarningMessage(ss.str().c_str());
    }

    return true;
}

//! @todo if we reconnect we should actually run check before simulation, BEFORE simulating, this is not done right now
bool ComponentSystem::preInitialize()
{
    // Recursively run preinitialize
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        if (!(*compIt)->preInitialize())
        {
            return false;
        }
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        if (!(*compIt)->preInitialize())
        {
            return false;
        }
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        if (!(*compIt)->preInitialize())
        {
            return false;
        }
    }
    return true;
}

//! @brief Load start values by telling each component to load their start values
void ComponentSystem::loadStartValues()
{
    // First load start values for all sub components
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

    //Also load start values for disabled components (otherwise final variables from previous simulation will remain in nodes)
    for(compIt = mDisabledSptrs.begin(); compIt != mDisabledSptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mDisabledCptrs.begin(); compIt != mDisabledCptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mDisabledQptrs.begin(); compIt != mDisabledQptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }

    // Now load all start values for the interface ports, they should override internally set start values
    PortPtrMapT::iterator pit;
    for (pit=mPortPtrMap.begin(); pit!=mPortPtrMap.end(); ++pit)
    {
        Port* pPort = pit->second;
        if (pPort->getPortType() == ReadPortType) //! @todo what about readmultiport
        {
            ReadPort *pReadPort = dynamic_cast<ReadPort*>(pPort);
            if (pReadPort)
            {
                // Only write readport start value if it is connected to other readports
                // This is the case of input variable ports on systems that are not externally connected
                if (!pReadPort->isConnectedToWriteOrPowerPort())
                {
                    pReadPort->forceLoadStartValue();
                }
            }
        }
        else
        {
            pPort->loadStartValues();
        }
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

//! @brief Recurse through the model system hierarchy and evaluate all parameters
void ComponentSystem::evaluateParametersRecursively()
{
    // First evaluate our own system parameters
    evaluateParameters();

    // Now evaluate any sub component and subsystem parameters
    std::vector<Component*>::iterator cit;
    for(cit = mComponentSignalptrs.begin(); cit != mComponentSignalptrs.end(); ++cit)
    {
        if ((*cit)->isComponentSystem())
        {
            static_cast<ComponentSystem*>(*cit)->evaluateParametersRecursively();
        }
        else
        {
            (*cit)->evaluateParameters();
        }
    }
    for(cit = mComponentCptrs.begin(); cit != mComponentCptrs.end(); ++cit)
    {
        if ((*cit)->isComponentSystem())
        {
            static_cast<ComponentSystem*>(*cit)->evaluateParametersRecursively();
        }
        else
        {
            (*cit)->evaluateParameters();
        }
    }
    for(cit = mComponentQptrs.begin(); cit != mComponentQptrs.end(); ++cit)
    {
        if ((*cit)->isComponentSystem())
        {
            static_cast<ComponentSystem*>(*cit)->evaluateParametersRecursively();
        }
        else
        {
            (*cit)->evaluateParameters();
        }
    }
}


//! @brief Loads parameters from a file
//! @param[in] rFilePath The file to load from
//! @return Number of changed parameters
size_t ComponentSystem::loadParameterValues(const HString &rFilePath)
{
    // Note! Even though this implementation is identical to the one in Component::loadParameterValues,
    //       this on is needed to ensure that the 'this' pointer points to a ComponentSystem
    return loadHopsanParameterFile(rFilePath, getHopsanEssentials()->getCoreMessageHandler(), this);
}

//! @brief Recurse through the model system hierarchy and evaluate all system-level numhop scripts
//! @returns true if no errors occurred, false otherwise
bool ComponentSystem::evaluateNumHopScriptRecursively()
{
    // First evaluate our own numhop script
    // If we have a Numhop script, then now is the time to run it
    if (!mNumHopScript.empty())
    {
        HString dummy;
        bool evalOK = runNumHopScript(mNumHopScript, true, dummy);
        if (!evalOK)
        {
            addErrorMessage("Numhop script evaluation failed: "+dummy);
            return false;
        }
    }

    // Now recurse down the subsystem hierarchy to evaluate subsystems numhop scripts
    bool evalOK=true;
    std::vector<Component*>::iterator cit;
    for(cit=mComponentSignalptrs.begin(); cit!=mComponentSignalptrs.end(); ++cit)
    {
        if ((*cit)->isComponentSystem())
        {
            evalOK = evalOK && static_cast<ComponentSystem*>(*cit)->evaluateNumHopScriptRecursively();
        }
    }
    for(cit=mComponentCptrs.begin(); cit!=mComponentCptrs.end(); ++cit)
    {
        if ((*cit)->isComponentSystem())
        {
            evalOK = evalOK && static_cast<ComponentSystem*>(*cit)->evaluateNumHopScriptRecursively();
        }
    }
    for(cit=mComponentQptrs.begin(); cit!=mComponentQptrs.end(); ++cit)
    {
        if ((*cit)->isComponentSystem())
        {
            evalOK = evalOK && static_cast<ComponentSystem*>(*cit)->evaluateNumHopScriptRecursively();
        }
    }
    return evalOK;
}

//! @brief Interprets and evaluates (runs) a numhop scripts
//! @param[in] rScript The script string
//! @param[in] printOutput Toggle whether to print output
//! @param[out] rOutput The output string to print to, (if printing activated)
//! @returns true if no errors occurred, false otherwise
bool ComponentSystem::runNumHopScript(const HString &rScript, bool printOutput, HString &rOutput)
{
    // Create if helper does not already exist
    if (!mpNumHopHelper)
    {
        mpNumHopHelper = new NumHopHelper();
        mpNumHopHelper->setSystem(this);
    }
    double dummy;
    return mpNumHopHelper->evalNumHopScript(rScript, dummy, printOutput, rOutput);
}

//! @brief Set the system-level numhop script
//! @param[in] rScript The script string
void ComponentSystem::setNumHopScript(const HString &rScript)
{
    mNumHopScript = rScript;
}

HString ComponentSystem::getNumHopScript() const
{
    return mNumHopScript;
}


//! @brief Initializes a system and all its contained components before a simulation.
//! Also allocates log data memory.
//! @param[in] startT Start time of simulation
//! @param[in] stopT Stop time of simulation
bool ComponentSystem::initialize(const double startT, const double stopT)
{
    //cout << "Initializing SubSystem: " << this->mName << endl;
    addCoreLogMessage("ComponentSystem::initialize() in "+getName());

    //Move all disabled components to temporary vectors
    for(size_t i=0; i<mComponentCptrs.size();)
    {
        if(mComponentCptrs.at(i)->isDisabled())
        {
            mDisabledCptrs.push_back(mComponentCptrs.at(i));
            mComponentCptrs.erase(mComponentCptrs.begin() + i );
        }
        else
        {
            ++i;
        }
    }
    for(size_t i=0; i<mComponentQptrs.size();)
    {
        if(mComponentQptrs.at(i)->isDisabled())
        {
            mDisabledQptrs.push_back(mComponentQptrs.at(i));
            mComponentQptrs.erase(mComponentQptrs.begin() + i );
        }
        else
        {
            ++i;
        }
    }
    for(size_t i=0; i<mComponentSignalptrs.size();)
    {
        if(mComponentSignalptrs.at(i)->isDisabled())
        {
            mDisabledSptrs.push_back(mComponentSignalptrs.at(i));
            mComponentSignalptrs.erase(mComponentSignalptrs.begin() + i );
        }
        else
        {
            ++i;
        }
    }

    if (this->isTopLevelSystem())
    {
        preInitialize();
    }

    mStopSimulation = false; //This variable cannot be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.

    // Set initial time
    mTime = startT;
    mTotalTakenSimulationSteps=0;

    // Make sure timestep is not to low
    if (mTimestep < 10*(std::numeric_limits<double>::min)())
    {
        addErrorMessage("The timestep is too low.");
        return false;
    }

    //cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;
    //this->setLogSettingsNSamples(mRequestedNumLogSamples, startT, stopT, mTimestep);

    // This will calculate the mnLogSlots and other log related variables
    this->setupLogSlotsAndTs(startT, stopT, mTimestep);
    //! @todo make it possible to use other logtimestep methods then nLogSamples

    // Preallocate local log space based on necessary number of log slots
    this->preAllocateLogSpace();

    // If we failed allocation then abort
    if (mStopSimulation)
    {
        return false;
    }

    adjustTimestep(mComponentSignalptrs);
    adjustTimestep(mComponentCptrs);
    adjustTimestep(mComponentQptrs);

    // Sort signal components, if they can not be sorted (algebraic loop), return with failure
    if(!sortComponentVector(mComponentSignalptrs))
    {
        return false;
    }
    // Sort C and Q components
    sortComponentVector(mComponentCptrs);
    sortComponentVector(mComponentQptrs);

    // run top-level system initialization functions
    if (this->isTopLevelSystem())
    {
        // If we have a Numhop script, then now is the time to run it
        if(!evaluateNumHopScriptRecursively())
        {
            return false;
        }

        // If the numhop scripts have changed the values, we need to make sure that the parameters are reevaluated
        // This is also necessary because preInitialize may have done some changes
        evaluateParametersRecursively();

        // Now we set the actual node data variables to the values from the start nodes (copy node values)
        // thereby initializing the system hierarchy with the start values
        // Only set start values from top-level system, else they will be set again in the subsystem initialize calls
        // It is important that all start values are set before initialization of the system hierarchy begins
        // initial calculations may depend on the start values having been set already
        if (!mKeepValuesAsStartValues )
        {
            loadStartValues();
        }
    }

    // Initialization of the system hierarchy
    // Initialize Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentSignalptrs[s]->initializeAutoSignalNodeDataPtrs();
        //mComponentSignalptrs[s]->evaluateParameters();

        if (mComponentSignalptrs[s]->isComponentSystem())
        {
            //! @todo should we use our own nSamples or the subsystems own ?
            static_cast<ComponentSystem*>(mComponentSignalptrs[s])->setNumLogSamples(mRequestedNumLogSamples);
            static_cast<ComponentSystem*>(mComponentSignalptrs[s])->setLogStartTime(mRequestedLogStartTime);
        }

        addCoreLogMessage("ComponentSystem::initialize() Initializing component: "+mComponentSignalptrs[s]->getName());
        if(!mComponentSignalptrs[s]->initialize(startT, stopT))
        {
            stopSimulation("Failed to initialize: "+mComponentSignalptrs[s]->getName());
        }
    }

    // Initialize C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentCptrs[c]->initializeAutoSignalNodeDataPtrs();
        //mComponentCptrs[c]->evaluateParameters();

        if (mComponentCptrs[c]->isComponentSystem())
        {
            //! @todo should we use our own nSamples ore the subsystems own ?
            static_cast<ComponentSystem*>(mComponentCptrs[c])->setNumLogSamples(mRequestedNumLogSamples);
            static_cast<ComponentSystem*>(mComponentCptrs[c])->setLogStartTime(mRequestedLogStartTime);
        }

        addCoreLogMessage("ComponentSystem::initialize() Initializing component: "+mComponentCptrs[c]->getName());
        if(!mComponentCptrs[c]->initialize(startT, stopT))
        {
            stopSimulation("Failed to initialize: "+mComponentCptrs[c]->getName());
        }
    }

    // Initialize Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentQptrs[q]->initializeAutoSignalNodeDataPtrs();
        //mComponentQptrs[q]->evaluateParameters();

        if (mComponentQptrs[q]->isComponentSystem())
        {
            //! @todo should we use our own nSamples ore the subsystems own ?
            static_cast<ComponentSystem*>(mComponentQptrs[q])->setNumLogSamples(mRequestedNumLogSamples);
            static_cast<ComponentSystem*>(mComponentQptrs[q])->setLogStartTime(mRequestedLogStartTime);
        }

        addCoreLogMessage("ComponentSystem::initialize() Initializing component: "+mComponentQptrs[q]->getName());
        if(!mComponentQptrs[q]->initialize(startT, stopT))
        {
            stopSimulation("Failed to initialize: "+mComponentQptrs[q]->getName());
        }
    }

    // If flag is set, then return with failure
    if (mStopSimulation)
    {
        return false;
    }

    // Log the start values
    logTimeAndNodes(mTotalTakenSimulationSteps);

    // We seems to have initialized successfully
    return true;
}


#if defined(HOPSANCORE_USEMULTITHREADING)
void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges, const ParallelAlgorithmT algorithm)
{
    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);      //Calculate how many threads to actually use

    std::stringstream ss;
    ss << nThreads;
    HString threadStr = ss.str().c_str();

    if(!noChanges)
    {
        if(algorithm != TaskStealingAlgorithm)
        {
            mpMultiThreadPrivates->mSplitCVector.clear();
            mpMultiThreadPrivates->mSplitQVector.clear();
            mpMultiThreadPrivates->mSplitSignalVector.clear();
            mpMultiThreadPrivates->mSplitNodeVector.clear();

            simulateAndMeasureTime(100);                                //Measure time
            sortComponentVectorsByMeasuredTime();                       //Sort component vectors

            for(size_t q=0; q<mComponentQptrs.size(); ++q)
            {
                addDebugMessage("Time for "+mComponentQptrs.at(q)->getName()+": "+ to_hstring(mComponentQptrs.at(q)->getMeasuredTime()));
            }
            for(size_t c=0; c<mComponentCptrs.size(); ++c)
            {
                addDebugMessage("Time for "+mComponentCptrs.at(c)->getName()+": "+to_hstring(mComponentCptrs.at(c)->getMeasuredTime()));
            }
            for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
            {
                addDebugMessage("Time for "+mComponentSignalptrs.at(s)->getName()+": "+to_hstring(mComponentSignalptrs.at(s)->getMeasuredTime()));
            }

            distributeCcomponents(mpMultiThreadPrivates->mSplitCVector, nThreads);              //Distribute components and nodes
            distributeQcomponents(mpMultiThreadPrivates->mSplitQVector, nThreads);
            distributeSignalcomponents(mpMultiThreadPrivates->mSplitSignalVector, nThreads);
            distributeNodePointers(mpMultiThreadPrivates->mSplitNodeVector, nThreads);

            // Re-initialize the system to reset values and timers
            //! @note This only work for top level systems where the simulateMultiThreaded will not be called more than once
            this->initialize(startT, stopT);
        }
        else
        {
            mpMultiThreadPrivates->mSplitCVector.clear();
            mpMultiThreadPrivates->mSplitQVector.clear();
            mpMultiThreadPrivates->mSplitSignalVector.clear();

            mpMultiThreadPrivates->mSplitCVector.resize(nThreads);
            mpMultiThreadPrivates->mSplitQVector.resize(nThreads);
            mpMultiThreadPrivates->mSplitSignalVector.resize(nThreads);

            for(size_t c=0; c<mComponentCptrs.size();)
            {
                for(size_t t=0; t<nThreads; ++t)
                {
                    if(c>mComponentCptrs.size()-1)
                        break;
                    mpMultiThreadPrivates->mSplitCVector[t].push_back(mComponentCptrs[c]);
                    ++c;
                }
            }

            for(size_t q=0; q<mComponentQptrs.size();)
            {
                for(size_t t=0; t<nThreads; ++t)
                {
                    if(q>mComponentQptrs.size()-1)
                        break;
                    mpMultiThreadPrivates->mSplitQVector[t].push_back(mComponentQptrs[q]);
                    ++q;
                }
            }

            distributeSignalcomponents(mpMultiThreadPrivates->mSplitSignalVector, nThreads);
        }
    }


    size_t nSteps = calcNumSimSteps(startT, stopT);

    //Execute simulation
    if(algorithm == OfflineSchedulingAlgorithm)
    {
        addInfoMessage("Using offline scheduling algorithm with "+threadStr+" threads.");

        mpMultiThreadPrivates->mvTimePtrs.push_back(&mTime);
        BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
        BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

        std::thread *tt = new std::thread[nThreads];

        tt[0] = std::thread(simMaster,
                            this,
                            std::ref(mpMultiThreadPrivates->mSplitSignalVector[0]),
                            std::ref(mpMultiThreadPrivates->mSplitCVector[0]),
                            std::ref(mpMultiThreadPrivates->mSplitQVector[0]),             //Create master thread
                            std::ref(mpMultiThreadPrivates->mSplitNodeVector[0]),
                            std::ref(mpMultiThreadPrivates->mvTimePtrs),
                            mTime,
                            mTimestep,
                            nSteps,
                            pBarrierLock_S,
                            pBarrierLock_C,
                            pBarrierLock_Q,
                            pBarrierLock_N);

        for (size_t t=1; t<nThreads; ++t)
        {
            tt[t] = std::thread(simSlave,
                                this,
                                std::ref(mpMultiThreadPrivates->mSplitSignalVector[t]),
                                std::ref(mpMultiThreadPrivates->mSplitCVector[t]),
                                std::ref(mpMultiThreadPrivates->mSplitQVector[t]),          //Create slave threads
                                std::ref(mpMultiThreadPrivates->mSplitNodeVector[t]),
                                mTime,
                                mTimestep,
                                nSteps,
                                pBarrierLock_S,
                                pBarrierLock_C,
                                pBarrierLock_Q,
                                pBarrierLock_N);
        }

        for (size_t i = 0; i<nThreads; ++i)                 //Wait for all tasks to finish
        {
            tt[i].join();
        }

        delete[] tt;
        delete(pBarrierLock_S);
        delete(pBarrierLock_C);
        delete(pBarrierLock_Q);
        delete(pBarrierLock_N);
    }
    else if(algorithm == TaskPoolAlgorithm)
    {

        addInfoMessage("Using task pool algorithm with "+threadStr+" threads.");

        TaskPool *pTaskPoolS = new TaskPool(mComponentSignalptrs);
        TaskPool *pTaskPoolC = new TaskPool(mComponentCptrs);
        TaskPool *pTaskPoolQ = new TaskPool(mComponentQptrs);

        std::thread *tt = new std::thread[nThreads];

        std::atomic<double> *pTime = new std::atomic<double>;
        *pTime = mTime;
        std::atomic<bool> *pStop = new std::atomic<bool>;
        *pStop = false;


        for (size_t t=0; t<nThreads-1; ++t)
        {
            tt[t] = std::thread(simPoolSlave,
                                pTaskPoolC,
                                pTaskPoolQ,
                                pTime,
                                pStop);
        }

        Component *pComp;
        for(size_t i=0; i<nSteps; ++i)
        {
            *pTime = *pTime+mTimestep;

            //S-pool
            pTaskPoolS->open();
            pComp = pTaskPoolS->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolS->reportDone();
                pComp = pTaskPoolS->getComponent();
            }
            while(!pTaskPoolS->isReady()) {}
            pTaskPoolS->close();

            //C-pool
            pTaskPoolC->open();
            pComp = pTaskPoolC->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolC->reportDone();
                pComp = pTaskPoolC->getComponent();
            }
            while(!pTaskPoolC->isReady()) {}
            pTaskPoolC->close();

            //Q-pool
            pTaskPoolQ->open();
            pComp = pTaskPoolQ->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolQ->reportDone();
                pComp = pTaskPoolQ->getComponent();
            }
            while(!pTaskPoolQ->isReady()) {}
            pTaskPoolQ->close();

            mTime =  *pTime;
            logTimeAndNodes(i+1);            //Log all nodes
        }
        *pStop=true;


        for (size_t i = 0; i<nThreads-1; ++i)                 //Wait for all tasks to finish
        {
            tt[i].join();
        }

        delete[] tt;
        delete(pTaskPoolS);
        delete(pTaskPoolC);
        delete(pTaskPoolQ);
        delete(pStop);
    }
    else if(algorithm == TaskStealingAlgorithm)
    {
        addInfoMessage("Using task stealing algorithm with "+threadStr+" threads.");

        mpMultiThreadPrivates->mvTimePtrs.push_back(&mTime);
        BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
        BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

        size_t maxSize = mComponentCptrs.size()+mComponentQptrs.size()+mComponentSignalptrs.size();

        std::vector<ThreadSafeVector *> *pVectorsC = new std::vector<ThreadSafeVector *>();
        for(size_t i=0; i<mpMultiThreadPrivates->mSplitCVector.size(); ++i)
        {
            pVectorsC->push_back(new ThreadSafeVector(mpMultiThreadPrivates->mSplitCVector[i], maxSize));
        }

        std::vector<ThreadSafeVector *> *pVectorsQ = new std::vector<ThreadSafeVector *>();
        for(size_t i=0; i<mpMultiThreadPrivates->mSplitQVector.size(); ++i)
        {
            pVectorsQ->push_back(new ThreadSafeVector(mpMultiThreadPrivates->mSplitQVector[i], maxSize));
        }

        std::thread *tt = new std::thread[nThreads];

        tt[0] = std::thread(simStealingMaster,
                            this,
                            std::ref(mComponentSignalptrs),
                            pVectorsC,
                            pVectorsQ,             //Create master thread
                            std::ref(mpMultiThreadPrivates->mvTimePtrs),
                            mTime,
                            mTimestep,
                            nSteps,
                            nThreads,
                            0,
                            pBarrierLock_S,
                            pBarrierLock_C,
                            pBarrierLock_Q,
                            pBarrierLock_N,
                            maxSize);


        for (size_t t=1; t<nThreads; ++t)
        {
            tt[t] = std::thread(simStealingSlave,
                                this,
                                pVectorsC,
                                pVectorsQ,
                                mTime,
                                mTimestep,
                                nSteps,
                                nThreads,
                                t,
                                pBarrierLock_S,
                                pBarrierLock_C,
                                pBarrierLock_Q,
                                pBarrierLock_N,
                                maxSize);
        }

        for (size_t i = 0; i<nThreads; ++i)                 //Wait for all tasks to finish
        {
            tt[i].join();
        }

        delete[] tt;                                           //Clean up
        delete(pBarrierLock_S);
        delete(pBarrierLock_C);
        delete(pBarrierLock_Q);
        delete(pBarrierLock_N);
        delete(pVectorsC);
        delete(pVectorsQ);
    }
    else if(algorithm == ParallelForAlgorithm)
    {
        addInfoMessage("Using parallel for-loop algorithm 1 with unlimited number of threads.");

        // Round to nearest, we may not get exactly the stop time that we want
        size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

        std::thread *tt;

        //Simulate
        for (size_t i=0; i<numSimulationSteps; ++i)
        {
            if (mStopSimulation)
            {
                break;
            }

            mTime += mTimestep; //mTime is updated here before the simulation,
            //mTime is the current time during the simulateOneTimestep

            //Signal components
            for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
            {
                mComponentSignalptrs[s]->simulate(mTime);
            }

            //C components
            tt = new std::thread[mComponentCptrs.size()];
            for (size_t c=0; c < mComponentCptrs.size(); ++c)
            {
                tt[c] = std::thread(simOneComponentOneStep,
                                    mComponentCptrs[c],
                                    mTime);
            }
            for(size_t c=0; c<mComponentCptrs.size(); ++c)
            {
                tt[c].join();
            }
            delete[] tt;

            //Q components
            tt = new std::thread[mComponentQptrs.size()];
            for (size_t q=0; q < mComponentQptrs.size(); ++q)
            {
                tt[q] = std::thread(simOneComponentOneStep,
                                    mComponentQptrs[q],
                                    mTime);
            }
            for(size_t q=0; q<mComponentQptrs.size(); ++q)
            {
                tt[q].join();
            }
            delete[] tt;

            ++mTotalTakenSimulationSteps;

            logTimeAndNodes(mTotalTakenSimulationSteps);
        }
    }
    else if(algorithm == GroupedParallelForAlgorithm)
    {
        addInfoMessage("Using grouped parallel for loop algorithm with unlimited number of threads.");

        // Round to nearest, we may not get exactly the stop time that we want
        size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

        std::thread *tt;

        //Simulate
        for (size_t i=0; i<numSimulationSteps; ++i)
        {
            if (mStopSimulation)
            {
                break;
            }

            mTime += mTimestep; //mTime is updated here before the simulation,
            //mTime is the current time during the simulateOneTimestep

            //Signal components
            for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
            {
                mComponentSignalptrs[s]->simulate(mTime);
            }

            //C components
            tt = new std::thread[mComponentCptrs.size()];
            for (size_t c=0; c < mComponentCptrs.size(); ++c)
            {
                tt[c] = std::thread(simOneStep,
                                    &mpMultiThreadPrivates->mSplitCVector[c],
                                    mTime);
            }
            for(size_t c=0; c<mComponentCptrs.size(); ++c)
            {
                tt[c].join();
            }
            delete[] tt;

            //Q components
            tt = new std::thread[mComponentQptrs.size()];
            for (size_t q=0; q < mComponentQptrs.size(); ++q)
            {
                tt[q] = std::thread(simOneStep,
                                    &mpMultiThreadPrivates->mSplitQVector[q],
                                    mTime);
            }
            for(size_t q=0; q<mComponentQptrs.size(); ++q)
            {
                tt[q].join();
            }
            delete[] tt;

            ++mTotalTakenSimulationSteps;

            logTimeAndNodes(mTotalTakenSimulationSteps);
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
//#ifdef _WIN32
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


//! @brief Helper function that distributes C components equally over one vector per thread
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
        addDebugMessage("Creating C-type thread vector, measured time = " + to_hstring(timeVector[i]*1000) + " ms", "cvector");
    }

    //Finally we sort each component vector, so that
    //signal components are simulated in correct order:
    for(size_t i=0; i<rSplitCVector.size(); ++i)
    {
        sortComponentVector(rSplitCVector[i]);

        for(size_t j=0; j<rSplitCVector[i].size(); ++j)
        {
            addDebugMessage("   "+rSplitCVector[i][j]->getName());
        }
    }
}


//! @brief Helper function that distributes Q components equally over one vector per thread
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
        addDebugMessage("Creating Q-type thread vector, measured time = " + to_hstring(timeVector[i]*1000) + " ms", "qvector");
    }

    //Finally we sort each component vector, so that
    //signal components are simulated in correct order:
    for(size_t i=0; i<rSplitQVector.size(); ++i)
    {
        sortComponentVector(rSplitQVector[i]);

        for(size_t j=0; j<rSplitQVector[i].size(); ++j)
        {
            addDebugMessage("   "+rSplitQVector[i][j]->getName());
        }
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
    //signal components are simulated in correct order:
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

void ComponentSystem::reschedule(size_t nThreads)
{
    mpMultiThreadPrivates->mSplitCVector.clear();
    mpMultiThreadPrivates->mSplitQVector.clear();
    mpMultiThreadPrivates->mSplitSignalVector.clear();
    mpMultiThreadPrivates->mSplitNodeVector.clear();

    simulateAndMeasureTime(10);                                //Measure time
    sortComponentVectorsByMeasuredTime();                       //Sort component vectors

    distributeCcomponents(mpMultiThreadPrivates->mSplitCVector, nThreads);              //Distribute components and nodes
    distributeQcomponents(mpMultiThreadPrivates->mSplitQVector, nThreads);
    distributeSignalcomponents(mpMultiThreadPrivates->mSplitSignalVector, nThreads);
    distributeNodePointers(mpMultiThreadPrivates->mSplitNodeVector, nThreads);
}

#endif

//! @brief Helper function that simulates all components and measure their average time requirements.
//! @param steps How many steps to simulate
bool ComponentSystem::simulateAndMeasureTime(const size_t nSteps)
{
#if (__cplusplus >= 201103L)
    // Reset all measured times first
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
        mComponentSignalptrs[s]->setMeasuredTime(0);
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
        mComponentCptrs[c]->setMeasuredTime(0);
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
        mComponentQptrs[q]->setMeasuredTime(0);


        // Measure time for each component during specified amount of steps
    double time;

    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
    {
        time = mTime; // Init time
#ifdef _WIN32
        HighResClock::time_point t0 = HighResClock::now();
#else
        timespec t0;
        clock_gettime(CLOCK_REALTIME, &t0);
#endif
        time += mTimestep*nSteps;
        mComponentSignalptrs[s]->simulate(time);
#ifdef _WIN32
        HighResClock::time_point t1 = HighResClock::now();
        HighResClock::duration dt = t1-t0;
        mComponentSignalptrs[s]->setMeasuredTime(dt.count()/1000000.0);
#else
        timespec t1;
        clock_gettime(CLOCK_REALTIME, &t1);
        mComponentSignalptrs[s]->setMeasuredTime(double(t1.tv_nsec-t0.tv_nsec)/1000000.0);
#endif
    }

    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        time=mTime; // Reset time
#ifdef _WIN32
        HighResClock::time_point t0 = HighResClock::now();
#else
        timespec t0;
        clock_gettime(CLOCK_REALTIME, &t0);
#endif
        time += mTimestep*nSteps;
        mComponentCptrs[c]->simulate(time);
#ifdef _WIN32
        HighResClock::time_point t1 = HighResClock::now();
        HighResClock::duration dt = t1-t0;
        mComponentCptrs[c]->setMeasuredTime(dt.count()/1000000.0);
#else
        timespec t1;
        clock_gettime(CLOCK_REALTIME, &t1);
        mComponentCptrs[c]->setMeasuredTime(double(t1.tv_nsec-t0.tv_nsec)/1000000.0);
#endif
    }

    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        time=mTime; // Reset time
#ifdef _WIN32
        HighResClock::time_point t0 = HighResClock::now();
#else
        timespec t0;
        clock_gettime(CLOCK_REALTIME, &t0);
#endif
        time += mTimestep*nSteps;
        mComponentQptrs[q]->simulate(time);

#ifdef _WIN32
        HighResClock::time_point t1 = HighResClock::now();
        HighResClock::duration dt = t1-t0;
        mComponentQptrs[q]->setMeasuredTime(dt.count()/1000000.0);
#else
        timespec t1;
        clock_gettime(CLOCK_REALTIME, &t1);
        mComponentQptrs[q]->setMeasuredTime(double(t1.tv_nsec-t0.tv_nsec)/1000000.0);
#endif
    }

    return true;
#else
    this->addErrorMessage("Measuring simulation time requires C++11 support.");
    return false;
#endif
}


//! @brief Returns the total sum of the measured time of the components in the system
double ComponentSystem::getTotalMeasuredTime()
{
    double time = 0;
#if (__cplusplus >= 201103L)
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
#else
    this->addErrorMessage("Measuring simulation time requires C++11 support.");
#endif
    return time;
}


//! @brief Helper function that sorts C- and Q- component vectors by simulation time for each component.
//! @todo This function uses bubblesort. Maybe change to something faster.
void ComponentSystem::sortComponentVectorsByMeasuredTime()
{
#if (__cplusplus >= 201103L)
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
#else
    this->addErrorMessage("Cannot sort! Measuring simulation time requires C++11 support.");
#endif
}

#if !defined(HOPSANCORE_USEMULTITHREADING)
//This overrides the multi-threaded simulation call with a single-threaded simulation if multi-threading is not available.
//! @brief Simulate function that overrides multi-threaded simulation call with a single-threaded call
//! In case multi-threaded support is not available
void ComponentSystem::simulateMultiThreaded(const double /*startT*/, const double stopT, const size_t /*nThreads*/, const bool /*noChanges*/, ParallelAlgorithmT /*algorithm*/)
{
    this->addErrorMessage("Multi-threaded simulation not available (compiled without C++11 support). Simulating single-threaded.");
    this->simulate(stopT);
}


void ComponentSystem::distributeCcomponents(vector< vector<Component*> > &/*rSplitCVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeCcomponents(), but multi-threading is not avaialble.");
}


void ComponentSystem::distributeQcomponents(vector< vector<Component*> > &/*rSplitQVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeQcomponents(), but multi-threading is not avaialble.");
}


void ComponentSystem::distributeSignalcomponents(vector< vector<Component*> > &/*rSplitSignalVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeSignalcomponents(), but multi-threading is not avaialble.");
}


void ComponentSystem::distributeNodePointers(vector< vector<Node*> > &/*rSplitNodeVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeNodePointers(), but multi-threading is not avaialble.");
}

#endif


////! @brief Simulate function for single-threaded simulations.
////! @param startT Start time of simulation
////! @param stopT Stop time of simulation
//void ComponentSystem::simulate(const double startT, const double stopT)
//{
//    mTime = startT;

//    //Simulate
//    double stopTsafe = stopT - mTimestep/2.0; //minus half a timestep is here to ensure that no numerical issues occurs

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

//! @brief Simulate function for single-threaded simulations.
//! @param[in] stopT Simulate from current time until stop time
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

    //Move disabled components back to their original vectors
    for(size_t i=0; i<mDisabledCptrs.size(); ++i)
    {
        mComponentCptrs.push_back(mDisabledCptrs.at(i));
    }
    mDisabledCptrs.clear();
    for(size_t i=0; i<mDisabledQptrs.size(); ++i)
    {
        mComponentQptrs.push_back(mDisabledQptrs.at(i));
    }
    mDisabledQptrs.clear();
    for(size_t i=0; i<mDisabledSptrs.size(); ++i)
    {
        mComponentSignalptrs.push_back(mDisabledSptrs.at(i));
    }
    mDisabledSptrs.clear();
}

////! @brief This function will set the number of log data slots for preallocation and logDt based on a skip factor to the sample time
////! @param [in] factor The timestep skip factor, minimum 1.0, but if < 0 then disableLog
//void ComponentSystem::setLogSettingsSkipFactor(double factor, double start, double stop,  double sampletime)
//{
//    if (factor > 0)
//    {
//        enableLog();
//        //! @todo maybe only use integer factors
//        //make sure factor is not less then 1.0
//        factor = max(1.0, factor);
//        mLogTimeDt = sampletime * factor;
//        //mLastLogTime = start-mLogTimeDt;
//        mnLogSlots = (size_t)((stop-start)/mLogTimeDt+0.5); //Round to nearest
//        //! @todo FIXA /Peter
//    }
//    else
//    {
//        disableLog();
//    }
//}


////! @brief This function will set the number of log data slots for preallocation and logDt
////! @param [in] log_dt The desired log timestep, if log_dt < 0 then disableLog
//void ComponentSystem::setLogSettingsSampleTime(double log_dt, double start, double stop,  double sampletime)
//{
//    if (log_dt > 0)
//    {
//        enableLog();
//        // Make sure that we dont have log_dt lower than sampletime ( we cant log more then we calc)
//        log_dt = max(sampletime,log_dt);
//        mLogTimeDt = log_dt;
//        //mLastLogTime = start-mLogTimeDt;
//        mnLogSlots = size_t((stop-start)/log_dt+0.5); //Round to nearest
//        //! @todo FIXA /Peter
//    }
//    else
//    {
//        disableLog();
//    }
//}

//! @brief Enable node data logging
void ComponentSystem::enableLog()
{
    mEnableLogData = true;
}


//! @brief Disable node data logging
void ComponentSystem::disableLog()
{
    mEnableLogData = false;

    // If log disabled, then free memory if something has been previously allocated
    mTimeStorage.clear();
    mLogTheseTimeSteps.clear();

    mLogTimeDt = -1.0;
    //mLastLogTime = 0.0; //Initial value should not matter, will be overwritten when selecting log amount
    mnLogSlots = 0;
    mLogCtr = 0;
}

vector<double> *ComponentSystem::getLogTimeVector()
{
    return &mTimeStorage;
}

void ConditionalComponentSystem::configure()
{
    addInputVariable("on", "On/off condition, 1=on, 0=off", "", 0, &mpCondition);

    mAsleep = false;
}

void ConditionalComponentSystem::simulate(const double stopT)
{
    if((*mpCondition)>0)
    {
        if(mAsleep)
        {
            for(SubComponentMapT::iterator it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
            {
                it->second->mTime = this->mTime;
            }
            mAsleep = false;
        }

        ComponentSystem::simulate(stopT);
    }
    else
    {
        // Round to nearest, we may not get exactly the stop time that we want
        size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

        //Simulate
        for (size_t i=0; i<numSimulationSteps; ++i)
        {
            mTime += mTimestep; //mTime is updated here before the simulation,
            //mTime is the current time during the simulateOneTimestep

            ++mTotalTakenSimulationSteps;

            logTimeAndNodes(mTotalTakenSimulationSteps);
        }

        mAsleep = true;
    }
}

void ConditionalComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges, ParallelAlgorithmT algorithm)
{
    ComponentSystem::simulateMultiThreaded(startT,  stopT, nDesiredThreads, noChanges, algorithm);
}

} // namespace hopsan
