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
//! @file   Port.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains Port base class as well as Sub classes
//!
//$Id$

#include "Port.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "HopsanEssentials.h"
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"
#include "Quantities.h"

using namespace std;
using namespace hopsan;

//! @brief Port base class constructor
Port::Port(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort)
{
    mPortName = rPortName;
    mNodeType = rNodeType;
    mpComponent = pParentComponent;
    mpParentPort = pParentPort; //Only used by subports in multiports
    mConnectionRequired = true;
    mConnectedPorts.clear();
    mSortHint = NoSortHint;
    mpNode = 0;
    mpStartNode = 0;
    mEnableLogging = true;

    // Create the initial node
    setNode(getComponent()->getHopsanEssentials()->createNode(mNodeType.c_str()));
    if (getComponent()->getSystemParent())
    {
        getComponent()->getSystemParent()->addSubNode(mpNode);
    }
}


//! @brief Destructor
Port::~Port()
{
    if (mpStartNode)
    {
        getComponent()->getHopsanEssentials()->removeNode(mpStartNode);
    }

    // Remove node if it was not handled by disconnect (like in non-connected ports)
    if (mpNode)
    {
        if (mpNode->getOwnerSystem())
        {
            mpNode->getOwnerSystem()->removeSubNode(mpNode);
        }
        getComponent()->getHopsanEssentials()->removeNode(mpNode);
    }
}



//! @brief Returns the type of node that can be connected to this port
const HString &Port::getNodeType() const
{
    return mNodeType;
}


//! @brief Returns the parent component
Component* Port::getComponent() const
{
    if(mpParentPort)
        return mpParentPort->getComponent();
    else
        return mpComponent;
}


//! @brief Returns a pointer to the connected node or 0 if no node exist
//! @param[in] subPortIdx Ignored on non multi ports
Node* Port::getNodePtr(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx);
    return mpNode;
}


//! @brief Adds a subport to a multiport
Port* Port::addSubPort()
{
    mpComponent->addFatalMessage("Port::addSubPort(): This should only be implemented and called from multiports.");
    return 0;
}

//! @brief Removes a subport from multiport
void Port::removeSubPort(Port* pPort)
{
    HOPSAN_UNUSED(pPort)
    mpComponent->addFatalMessage("Port::removeSubPort(): This should only be implemented and called from multiports.");
}

//! @brief This function registers the startvalue parameters from the start node
void Port::registerStartValueParameters()
{
    // Prevent registering if subports in multiport, or if startnode is missing
    if (mpStartNode && !mpParentPort)
    {
        for(size_t i=0; i<mpStartNode->getNumDataVariables(); ++i)
        {
            const NodeDataDescription* pDesc = mpStartNode->getDataDescription(i);
            const HString desc = "startvalue: Port "+getName();
            const HString name = getName()+"#"+pDesc->name;
            getComponent()->addConstant(name, desc, pDesc->quantity, pDesc->unit, *(mpStartNode->getDataPtr(pDesc->id)), *(mpStartNode->getDataPtr(pDesc->id)));
        }
    }
}


//! @brief Unregisters all startvalue parameters from the start node
void Port::unRegisterStartValueParameters()
{
    if (mpStartNode && !mpParentPort)
    {
        for(size_t i=0; i<mpStartNode->getNumDataVariables(); ++i)
        {
            const NodeDataDescription* pDesc = mpStartNode->getDataDescription(i);
            const HString name = getName()+"#"+pDesc->name;
            getComponent()->unRegisterParameter(name);
        }
    }
}




//! @brief Load start values by copying the start values from the port to the node
void Port::loadStartValues()
{
    if(mpStartNode)
    {
        mpStartNode->copyNodeDataValuesTo(mpNode);
        mpStartNode->copySignalQuantityAndUnitTo(mpNode);
    }
}


//! @brief Load start values to the start value container from the node (last values from simulation)
void Port::loadStartValuesFromSimulation()
{
    if(isConnected() && mpStartNode)
    {
        mpNode->copyNodeDataValuesTo(mpStartNode);
    }
}


//! @brief Reads a value from the connected node
//! @param [in] idx The data id of the data to read
//! @param [in] subPortIdx The subPort index of a port in a multiport, (ignored if not a multiport)
//! @details Safe but slow version, will not crash if idx out of bounds
//! @return The data value
//! @ingroup ComponentSimulationFunctions
double Port::readNodeSafe(const size_t idx, const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)

    if (idx < mpNode->getNumDataVariables())
    {
        return mpNode->mDataValues[idx];
    }
    getComponent()->addErrorMessage("data idx out of range in Port::readNodeSafe()");
    return -1;
}


//! @brief Writes a value to the connected node
//! @param [in] idx The data id of the data to write  (Such as NodeHydraulic::Pressure)
//! @param [in] value The value of the data to read
//! @param [in] subPortIdx Ignored for non multi ports
//! @details Safe but slow version, will not crash if idx out of bounds
//! @ingroup ComponentSimulationFunctions
void Port::writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if (idx < mpNode->getNumDataVariables())
    {
        mpNode->mDataValues[idx] = value;
    }
    else
    {
        getComponent()->addErrorMessage("data idx out of range in Port::writeNodeSafe()");
    }
}

//! @brief Returns the node pointer in the port
//! @param [in] subPortIdx Ignored for non multi ports
//! @returns The node pointer in this port
const Node *Port::getNodePtr(const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    return mpNode;
}

//! @brief Get a ptr to the data variable in the node
//! @param [in] idx The id of the data variable to return ptr to
//! @param [in] subPortIdx Ignored for non multi ports
//! @returns Pointer to data variable or 0 if idx was not found
double *Port::getNodeDataPtr(const size_t idx, const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    if (idx < mpNode->getNumDataVariables())
    {
        return mpNode->getDataPtr(idx);
    }
    else
    {
        return 0;
    }
}


//! @brief Set the node that the port is connected to
//! @param [in] pNode A pointer to the Node
void Port::setNode(Node* pNode)
{
    if (!pNode)
    {
        getComponent()->addFatalMessage("In Port::setNode(), You cant set a NULL node ptr");
    }
    else
    {
        if (mpNode)
        {
            mpNode->removeConnectedPort(this);
        }
        mpNode = pNode;
        mpNode->addConnectedPort(this);
    }
}


//! @brief Adds a pointer to an other connected port to a port
//! @param [in] pPort A pointer to the other port
//! @param [in] subPortIdx Ignored for non multi ports
void Port::addConnectedPort(Port* pPort, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    mConnectedPorts.push_back(pPort);
}


//! @brief Removes a pointer to an other connected port from a port
//! @param [in] pPort The pointer to the other port to be removed
//! @param [in] subPortIdx Ignored for non multi ports
void Port::eraseConnectedPort(Port* pPort, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        //printf("*it: %p pPort: %p", (void*)(*it), (void*)pPort);
        //cout << endl;
        //cout << "*it: " << *it << " pPort: " << pPort << endl;
        if (*it == pPort)
        {
            mConnectedPorts.erase(it);
            return;
        }
    }
    cout << "Error: You tried to erase port ptr that did not exist in the connected ports list" << endl;
}


//! @brief Get a vector of pointers to all other ports connected connected to this one
//! @param [in] subPortIdx Ignored for non multi ports
//! @returns A vector with the connected port pointers
//! @todo maybe should return const vector so that contents my not be changed
std::vector<Port *> Port::getConnectedPorts(const int subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    return mConnectedPorts;
}

//! @brief Returns the number of ports connected to this port
//! @param[in] subPortIdx The index of the subPort to check. Ignored on non multi ports
//! @returns The number of connected ports
size_t Port::getNumConnectedPorts(const int subPortIdx)
{
    return getConnectedPorts(subPortIdx).size();
}

//! @brief Creates a start node in the port
//! @param[in] rNodeType The type of node to create (Such as NodeHydraulic)
void Port::createStartNode(const HString &rNodeType)
{
    // If startnode already exists we need to remove it before creating a new one (to avoid memory leak)
    if (mpStartNode)
    {
        eraseStartNode();
    }
    // Now create a new startnode of given type
    mpStartNode = getComponent()->getHopsanEssentials()->createNode(rNodeType);
    //!< @todo Maybe I don't even need to create startnodes for subports in multiports, in that case, move this line into if below

    // Prevent registering startvalues for subports in multiports, It will be very difficult to ensure that those would actually work as expected
    if (mpParentPort == 0)
    {
        registerStartValueParameters();
    }
}

//! @brief Removes the start node in the port and unregisters all start value parameters
void Port::eraseStartNode()
{
    unRegisterStartValueParameters();
    getComponent()->getHopsanEssentials()->removeNode(mpStartNode);
    mpStartNode = 0;
}

//! @note This one should be called by system, do not call this manually (that will create a mess)
void Port::setVariableAlias(const HString &rAlias, const size_t id)
{
    //! @todo check id
    // First remove it if already set
    std::map<HString, size_t>::iterator it = mVariableAliasMap.begin();
    while (it!=mVariableAliasMap.end())
    {
        if (it->second == id)
        {
            mVariableAliasMap.erase(it);
            // Restart search if something was removed as iterator breaks
            it = mVariableAliasMap.begin();
        }
        else
        {
             ++it;
        }
    }

    // Replace with new name, if not empty
    if (!rAlias.empty())
    {
        mVariableAliasMap.insert(std::pair<HString, size_t>(rAlias, id));
    }
}

//! @brief Get the alias name for a specific node variable id
//! @param [in] id The node data id of the requested variable (Ex: NodeHydraulic::Pressure)
//! @return The alias name or empty string if no alias name exist for requested variable
const HString &Port::getVariableAlias(const size_t id) const
{
    std::map<HString, size_t>::const_iterator it;
    for(it=mVariableAliasMap.begin();it!=mVariableAliasMap.end();++it)
    {
        if (it->second == id)
        {
            return it->first;
        }
    }
    return mEmptyString;
}

//! @brief Get the variable id for a specific alias name
//! @param [in] rAlias The alias name to search for
//! @returns The variable id (integer value) (Ex:: NodeHydarulic::Pressure) or -1 if not found
int Port::getVariableIdByAlias(const HString &rAlias) const
{
    std::map<HString, size_t>::const_iterator it = mVariableAliasMap.find(rAlias);
    {
        if (it!=mVariableAliasMap.end())
        {
            return int(it->second);
        }
    }
    return -1;
}

//! @brief Get a pointer to the start node
//! @returns StartNode ptr or 0 if no startnode
Node *Port::getStartNodePtr()
{
    return mpStartNode;
}

//! @brief Check if log data  exist in the ports node
//! @param[in] subPortIdx Ignored on non multi ports
//! @returns True or False
bool Port::haveLogData(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if (mpNode)
    {
        // Here we assume that timevector DOES exist. If simulation code is correct it should exist
        return !mpNode->mDataStorage.empty();
    }
    return false;
}

void Port::setEnableLogging(const bool enableLog)
{
    mEnableLogging = enableLog;
}

bool Port::isLoggingEnabled() const
{
    return mEnableLogging;
}

//! @brief Get all node data descriptions
//! @param [in] subPortIdx Ignored on non multi ports
//! @returns A const pointer to the internal node vector with node data descriptions
const std::vector<NodeDataDescription>* Port::getNodeDataDescriptions(const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    // We prefer to use the startnode
    if (mpStartNode)
    {
        return mpStartNode->getDataDescriptions();
    }
    else
    {
        // This node could contain descriptions copied by someone else
        return mpNode->getDataDescriptions();
    }
}


//! @brief Get a specific node data description
//! @param [in] dataid The node data id (Such as NodeHydraulic::Pressure)
//! @param [in] subPortIdx Ignored on non multi ports
//! @returns A const pointer to the node data description, or 0 if no node exist
const NodeDataDescription* Port::getNodeDataDescription(const size_t dataid, const size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    // We prefer to use the startnode
    if (mpStartNode)
    {
        return mpStartNode->getDataDescription(dataid);
    }
    else
    {
        // This node could contain descriptions copied by someone else
        return mpNode->getDataDescription(dataid);
    }
}


//! @brief Ask the node for the dataId for a particular data name such as (Pressure)
//! @details This is a wrapper function for the actual Node function,
//! @param [in] rName The name of the variable (Such as Pressure)
//! @param [in] subPortIdx Ignored on non multi ports
//! @returns The node data id (positive integer) if the variable name is found else returns -1 to indicate failure
int Port::getNodeDataIdFromName(const HString &rName, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    //! @todo since mpNode should always be set maybe we could remove (almost) all the checks (but not for multiports their mpNOde will be 0)
    if (mpNode != 0)
    {
        return mpNode->getDataIdFromName(rName);
    }
    else
    {
        return -1;
    }
}

//! @brief A help function that makes it possible to overwrite the unit or quantity of scalar signal node variables
void Port::setSignalNodeQuantityOrUnit(const HString &rQuantityOrUnit)
{
    //! @todo multiport version needed

    HString bu = gpInternalCoreQuantityRegister->lookupBaseUnit(rQuantityOrUnit);
    // If this was not a quantity
    if (bu.empty())
    {
        mpNode->setSignalQuantity("", rQuantityOrUnit);
        if (mpStartNode)
        {
            mpStartNode->setSignalQuantity("", rQuantityOrUnit);
        }
    }
    // If this was a quantity
    else
    {
        mpNode->setSignalQuantity(rQuantityOrUnit, bu);
        if (mpStartNode)
        {
            mpStartNode->setSignalQuantity(rQuantityOrUnit, bu);
        }
    }
}

void Port::setSignalNodeQuantityModifyable(bool tf)
{
    //! @todo multiport version needed

    mpNode->setSignalQuantityModifyable(tf);
    if (mpStartNode)
    {
        mpStartNode->setSignalQuantityModifyable(tf);
    }
}

HString Port::getSignalNodeQuantity() const
{
    // Return setting in startnode as that is teh setting for our own node, the mpNode may contain someone elses setting
    if (mpStartNode)
    {
        return mpStartNode->getSignalQuantity();
    }
    return "";
}

bool Port::getSignalNodeQuantityModifyable() const
{
    // Return setting in startnode as that is teh setting for our own node, the mpNode may contain someone elses setting
    if (mpStartNode)
    {
        return mpStartNode->getSignalQuantityModifyable();
    }
    return false;
}

//! @param [in] subPortIdx Ignored on non multi ports
vector<double> *Port::getLogTimeVectorPtr(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if (mpNode != 0)
    {
        ComponentSystem *pOwnerSys = mpNode->getOwnerSystem();
        if (pOwnerSys)
        {
            return pOwnerSys->getLogTimeVector();
        }
    }
    return 0; //Nothing found return 0
}

//! @param [in] subPortIdx Ignored on non multi ports
vector<vector<double> > *Port::getLogDataVectorPtr(size_t subPortIdx)
{
    // TODO use const cast maybe
    HOPSAN_UNUSED(subPortIdx)
    if (mpNode != 0) {
        return &(mpNode->mDataStorage);
    }
    else {
        return 0;
    }
}

const std::vector<std::vector<double> > *Port::getLogDataVectorPtr(size_t subPortIdx) const
{
    HOPSAN_UNUSED(subPortIdx)
    if (mpNode != 0) {
        return &(mpNode->mDataStorage);
    }
    else {
        return 0;
    }
}

bool Port::isInterfacePort() const
{
    return getComponent()->isComponentSystem();
}

//! @param [in] subPortIdx Ignored on non multi ports
vector<double> *Port::getDataVectorPtr(const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if(mpNode != 0)
    {
        return &(mpNode->mDataValues);
    }
    else
    {
        return 0;
    }
}

//! @brief Returns the number of data variables in the node
//! @returns The number of data variables in the node
size_t Port::getNumDataVariables() const
{
    return mpNode->getNumDataVariables();
}


//! @brief Get the actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param[in] subPortIdx Ignored on non multi ports
//! @returns the start value
double Port::getStartValue(const size_t idx, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if(mpStartNode && getComponent()->getSystemParent()->keepsValuesAsStartValues())
    {
        return mpNode->getDataValue(idx);
    }
    else if(mpStartNode)
    {
        return mpStartNode->getDataValue(idx);
    }
    getComponent()->addErrorMessage("Port::getStartValue(): Port does not have a start value.");
    return -1;
}


//! @brief Set the an actual start value of the port
//! @param [in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param [in] value is the start value that should be written
//! @param[in] subPortIdx Ignored on non multi ports
void Port::setDefaultStartValue(const size_t idx, const double value, const size_t subPortIdx)
{
    HOPSAN_UNUSED(subPortIdx)
    if(mpStartNode)
    {
        if (idx < mpStartNode->getNumDataVariables())
        {
            mpStartNode->setDataValue(idx, value);
        }
        else
        {
            getComponent()->addWarningMessage("Port::setDefaultStartValue index out of range");
        }
    }
    else
    {
        getComponent()->addWarningMessage("Tried to set StartValue for port: "+getName()+" This was ignored because this port does not have a StartNode.");
    }
}


//! @brief Disables start value for specified data type
//! @param idx Data index of start value to be disabled
void Port::disableStartValue(const size_t idx)
{
    if (mpStartNode)
    {
        // The start value has already been registered as a parameter in the component, so we must unregister it.
        // This is probably not the most beautiful solution.
        HString name = getName()+"#"+mpStartNode->getDataDescription(idx)->name;
        mpComponent->addDebugMessage("Disabling_StartValue: "+name);
        mpComponent->unRegisterParameter(name);

        // Note, the startNode and its value will remain, it will also be copied every time.
        // Components should automatically write the correct initial value to nodes in initialize
        // If a startvalue has been disabled you can not change it, (it actually means that it is hidden)
    }
}


//! @brief Check if the port is currently connected
//! @brief Returns True or False
bool Port::isConnected() const
{
    return (mConnectedPorts.size() > 0);
}

//! @brief Check if this port is connected to other port
//! @todo how do we handle multiports
bool Port::isConnectedTo(Port *pOtherPort)
{
    //! @todo this is a hack for now, since isConnectedTo is overloaded we want the actual multiport to check, Make smarter in the future
    if (pOtherPort->isMultiPort())
    {
        return pOtherPort->isConnectedTo(this);
    }
    else
    {
        std::vector<Port*>::iterator pit;
        for(pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
        {
            //printf("isConnectedTo: *pit: %p pOtherPort: %p", (void*)(*pit), (void*)pOtherPort);
            //cout << endl;
            if ( *pit == pOtherPort )
            {
                return true;
            }
        }
        return false;
    }
}


//! @brief Check if the port MUST be connected
bool Port::isConnectionRequired()
{
    return mConnectionRequired;
}

size_t Port::getNumPorts()
{
    return 1;
}

//! @brief Convenience function to check if port is multiport
bool Port::isMultiPort() const
{
    return false;
}

Port *Port::getParentPort() const
{
    return mpParentPort;
}

//! @brief Get the port type
PortTypesEnumT Port::getPortType() const
{
    return UndefinedPortType;
}

//! @brief Get the External port type (virtual, should be overloaded in systemports only)
PortTypesEnumT Port::getExternalPortType()
{
    return getPortType();
}

//! @brief Get the Internal port type (virtual, should be overloaded in systemports only)
PortTypesEnumT Port::getInternalPortType()
{
    return getPortType();
}

SortHintEnumT Port::getSortHint() const
{
    return mSortHint;
}

void Port::setSortHint(SortHintEnumT hint)
{
    mSortHint = hint;
}

SortHintEnumT Port::getInternalSortHint()
{
    return getSortHint();
}


//! @brief Get the port name
const HString &Port::getName() const
{
    return mPortName;
}


//! @brief Get the name of the component that the port is attached to
const HString &Port::getComponentName() const
{
    return getComponent()->getName();
}

//! @brief Get port description
//! @returns Port description
const HString &Port::getDescription() const
{
    return mDescription;
}

//! @brief Set port description
//! @param [in] rDescription The new description
void Port::setDescription(const HString &rDescription)
{
    mDescription = rDescription;
}


SystemPort::SystemPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    // Do nothing special
}


//! @brief Get the External port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getExternalPortType()
{
    Port* pPortFound=0;
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        // External port component parents will belong to this system ports component parents parent system
        // If we have an external power port, then we want to return that, else we return the port type that we find
        // Usually the first one we find is correct, except when we mix powerports and sensor ports (read ports)
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent()->getSystemParent() )
        {
            pPortFound = *pit;
            //! @todo what about multpowerporttype (not sure if that is supported yet (systemport <-> multiport that is)
            if (pPortFound->getPortType() == PowerPortType)
            {
                return pPortFound->getPortType();
            }
        }
    }
    // If we did not find a power port but found some other port type, then return this other type
    if (pPortFound)
    {
        return pPortFound->getPortType();
    }
    // If no external ports found return our actual type (systemport)
    else
    {
        return getPortType();
    }
}

//! @brief Get the Internal port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getInternalPortType()
{
    Port* pPortFound=0;
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        // Internal port component parents will belong to this systemports parent component (the system)
        // If we have an internal power port, then we want to return that, else we return the port type that we find
        // Usually the first one we find is correct, except when we mix powerports and sensor ports (read ports)
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent() )
        {
            pPortFound = *pit;
            //! @todo what about multpowerporttype (not sure if that is supported yet (systemport <-> multiport that is)
            if (pPortFound->getPortType() == PowerPortType)
            {
                return pPortFound->getPortType();
            }
        }
    }
    // If we did not find a power port but found some other port type, then return this other type
    if (pPortFound)
    {
        return pPortFound->getPortType();
    }
    // If no internal ports found return our actual type (systemport)
    else
    {
        return getPortType();
    }
}

SortHintEnumT SystemPort::getInternalSortHint()
{
    size_t nSources=0, nDest=0, nUndef=0;
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        Port* pPort = *pit;
        // Only check internal component ports
        if (pPort->getComponent()->getSystemParent() == this->getComponent())
        {
            if (pPort->getSortHint() != NoSortHint)
            {
                if (pPort->getSortHint() == Source)
                {
                    nSources++;
                }
                else if (pPort->getSortHint() == Destination)
                {
                    nDest++;
                }
                else
                {
                    nUndef++;
                }
            }
        }
    }
    if (nSources>0)
    {
        return Source;
    }
    else if (nDest>0)
    {
        return Destination;
    }
    else
    {
        return NoSortHint;
    }
}

//! @brief PowerPort constructor
PowerPort::PowerPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}

BiDirectionalSignalPort::BiDirectionalSignalPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    //createStartNode(rNodeType);
}


ReadPort::ReadPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    setSortHint(Destination);
    createStartNode(rNodeType);
}


void ReadPort::writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx)
{
    HOPSAN_UNUSED(idx)
    HOPSAN_UNUSED(value)
    HOPSAN_UNUSED(subPortIdx)
            mpComponent->addErrorMessage("ReadPort::writeNodeSafe(): Could not write to port, this is a ReadPort.");
}

void ReadPort::setSortHint(SortHintEnumT hint)
{
    if (hint == Destination || hint == IndependentDestination)
    {
        mSortHint = hint;
    }
}


//void ReadPort::writeNode(const size_t idx, const double value, const size_t subPortIdx)
//{
//    HOPSAN_UNUSED(idx)
//    HOPSAN_UNUSED(value)
//    HOPSAN_UNUSED(subPortIdx)
//    mpComponent->addErrorMessage("ReadPort::writeNode(): Could not write to port, this is a ReadPort.");
//}

void ReadPort::loadStartValues()
{
    // Prevent loading startvalues if this port is connected, then the write node will set the start value
    if (!isConnected())
    {
        Port::loadStartValues();
    }
}


bool ReadPort::isConnectedToWriteOrPowerPort()
{
    vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        Port *pPort = (*pit);
        if ((pPort->getPortType() == WritePortType) || (pPort->getPortType() == PowerPortType) || (pPort->getPortType() == PowerMultiportType))
        {
            return true;
        }
    }
    return false;
}

void ReadPort::forceLoadStartValue()
{
    Port::loadStartValues();
}

WritePort::WritePort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    createStartNode(mNodeType);
}


MultiPort::MultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    Port(rNodeType, rPortName, pParentComponent, pParentPort)
{
    // Do not log multiports,
    // It is not possible to determin what sub port to log anyway
    mEnableLogging = false;
}

MultiPort::~MultiPort()
{
    // Delete all subports that may remain, if everything is working this should be zero
    if (mSubPortsVector.size() != 0)
    {
        getComponent()->addFatalMessage("~MultiPort(): mSubPortsVector.size() != 0 in multiport destructor (will fix later)");
    }
}

bool MultiPort::isMultiPort() const
{
    return true;
}


//! @brief Reads a value from the connected node
//! @param [in] idx The data id of the data to read
//! @param [in] subPortIdx The subPort index in the multi port
//! @details Safe but slow version, will not crash if idx out of bounds
//! @return The data value or -1 if any of the idxes are out of range
//! @ingroup ComponentSimulationFunctions
double MultiPort::readNodeSafe(const size_t idx, const size_t subPortIdx) const
{
    if (subPortIdx < mSubPortsVector.size())
    {
        return mSubPortsVector[subPortIdx]->readNodeSafe(idx);
    }
    getComponent()->addErrorMessage("portIdx out of range in MultiPort::readNodeSafe()");
    return -1;
}

//! @brief Writes a value to the connected node
//! @param [in] idx The data id of the data to write (Such as NodeHydraulic::Pressure)
//! @param [in] value The value of the data to read
//! @param [in] subPortIdx The subport to write to, (range check is performed)
//! @details Safe but slow version, will not crash if idx out of bounds
//! @ingroup ComponentSimulationFunctions
void MultiPort::writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx)
{
    if (subPortIdx < mSubPortsVector.size())
    {
        mSubPortsVector[subPortIdx]->writeNodeSafe(idx,value);
    }
    else
    {
        getComponent()->addErrorMessage("portIdx out of range in MultiPort::writeNodeSafe()");
    }
}

//! @brief Returns the node pointer from one of the subports in the port (const version)
//! @param [in] subPortIdx The sub port to retrieve from, (range check is NOT performed!)
//! @returns The node pointer in the sub port
const Node *MultiPort::getNodePtr(const size_t subPortIdx) const
{
    return mSubPortsVector[subPortIdx]->getNodePtr();
}

//! @todo why do we even want a unsafe getNodeDataPtr it should be the safe version
double *MultiPort::getNodeDataPtr(const size_t idx, const size_t subPortIdx) const
{
    //If we try to access node data for subport that does not exist then return multiport shared dummy safe ptr
    if (subPortIdx >= mSubPortsVector.size())
    {
        return Port::getNodeDataPtr(idx, subPortIdx);
    }
    else
    {
        return mSubPortsVector[subPortIdx]->getNodeDataPtr(idx);
    }
}

//! @brief Get all node data descriptions from a connected sub port node
//! @param [in] subPortIdx The subport idx to fetch from (range is checked)
//! @returns A const pointer to the internal node vector with node data descriptions or 0 if subPortIdx is out of range or if no node exist in port
const std::vector<NodeDataDescription>* MultiPort::getNodeDataDescriptions(const size_t subPortIdx) const
{
    if (subPortIdx < mSubPortsVector.size())
    {
        return mSubPortsVector[subPortIdx]->getNodeDataDescriptions();
    }
    return 0;
}

//! @brief Get a specific node data description from a connected sub port node
//! @param [in] dataid The node data id (Such as NodeHydraulic::Pressure)
//! @param [in] subPortIdx The subport idx to fetch from (range is NOT checked)
//! @returns A const pointer to the node data description, or 0 if no node exist
const NodeDataDescription* MultiPort::getNodeDataDescription(const size_t dataid, const size_t subPortIdx) const
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getNodeDataDescription(dataid);
    }
    else if (mpStartNode)
    {
        return mpStartNode->getDataDescription(dataid);
    }
    return 0;
}

//! @brief Ask the node for the dataId for a particular data name such as (Pressure)
//! @details This is a wrapper function for the actual Node function,
//! @param [in] rName The name of the variable (Such as Pressure)
//! @param [in] subPortIdx The subPort to ask, (range is NOT checked)
//! @returns The node data id (positive integer) if the variable name is found else returns -1 to indicate failure
int MultiPort::getNodeDataIdFromName(const HString &rName, const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getNodeDataIdFromName(rName);
    }
    return -1;
}

bool MultiPort::haveLogData(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->haveLogData();
    }
    return false;
}

std::vector<double> *MultiPort::getLogTimeVectorPtr(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getLogTimeVectorPtr();
    }
    return 0;
}

std::vector<std::vector<double> > *MultiPort::getLogDataVectorPtr(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getLogDataVectorPtr();
    }
    return 0;
}

const std::vector<std::vector<double> > *MultiPort::getLogDataVectorPtr(size_t subPortIdx) const
{
    if (isConnected()) {
        return mSubPortsVector[subPortIdx]->getLogDataVectorPtr();
    }
    return 0;
}

void MultiPort::setEnableLogging(const bool enableLog)
{
    HOPSAN_UNUSED(enableLog);
    // Do nothing since multiports can not be logged
}


std::vector<double> *MultiPort::getDataVectorPtr(const size_t subPortIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[subPortIdx]->getDataVectorPtr();
    }
    return 0;
}

//! @brief Get the an actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param[in] subPortIdx The sub port to get start value from
//! @todo shouldn't this be called get default startvalue, to avoid confusion with initial value (I am not sure)
//! @returns the start value
double MultiPort::getStartValue(const size_t idx, const size_t subPortIdx)
{
    if(mpStartNode && mpComponent->getSystemParent()->keepsValuesAsStartValues())
    {
        return mSubPortsVector[subPortIdx]->mpNode->getDataValue(idx);
    }
    else if(mpStartNode)
    {
        return mpStartNode->getDataValue(idx);
    }
    mpComponent->addErrorMessage("MultiPort::getStartValue(): Port does not have a start value.");
    return -1.0;
}

void MultiPort::loadStartValues()
{
    if(mpStartNode)
    {
        for(size_t p=0; p<mSubPortsVector.size(); ++p)
        {
            mpStartNode->copyNodeDataValuesTo(mSubPortsVector[p]->mpNode);
            mpStartNode->copySignalQuantityAndUnitTo(mSubPortsVector[p]->mpNode);
        }
    }
}

void MultiPort::loadStartValuesFromSimulation()
{
    //! @todo what about this one then how should we handle this
}

bool MultiPort::isConnectedTo(Port *pOtherPort)
{
    if (this->isMultiPort() && pOtherPort->isMultiPort())
    {
        getComponent()->addFatalMessage("In isConnectedTo() both ports are multiports, this should not happen!");
        return false;
    }

    for (size_t i=0; i<mSubPortsVector.size(); ++i)
    {
        if (mSubPortsVector[i]->isConnectedTo(pOtherPort))
        {
            return true;
        }
    }

    return false;
}

//! @brief Check if the port is currently connected
bool MultiPort::isConnected() const
{
    //! @todo actually we should check all subports if they are connected (but a subport should not exist if not connected)
    return (mSubPortsVector.size() > 0);
}

size_t MultiPort::getNumPorts()
{
    return mSubPortsVector.size();
}

//! @brief Removes a specific subport
void MultiPort::removeSubPort(Port* ptr)
{
    std::vector<Port*>::iterator spit;
    for (spit=mSubPortsVector.begin(); spit!=mSubPortsVector.end(); ++spit)
    {
        if ( *spit == ptr )
        {
            mSubPortsVector.erase(spit);
            break;
        }
    }
}

//! @brief Adds a subport of a particular type to a multiport
Port* MultiPort::addSubPort(const hopsan::PortTypesEnumT type)
{
    hopsan::Port* pNewSubPort = createPort(type, mNodeType, "noname_subport", 0, this);
    pNewSubPort->setEnableLogging(mEnableLogging);
    mSubPortsVector.push_back( pNewSubPort );
    return pNewSubPort;
}

//! @brief Returns the node pointer from one of the subports in the port
//! @param [in] subPortIdx The sub port to retrieve from, (range check is performed)
//! @returns The node pointer in the sub port, or 0 if index out of range
Node *MultiPort::getNodePtr(const size_t subPortIdx)
{
    if(mSubPortsVector.size() <= subPortIdx)
    {
        mpComponent->addWarningMessage("MultiPort::getNodePtr(): mSubPortsVector.size() <= portIdx");
        return 0;
    }
    return mSubPortsVector[subPortIdx]->getNodePtr();
}

//! @brief Get all the connected ports
//! @param[in] subPortIdx The sub port to get connected ports from, Use -1 to indicate that all subports should be considered
//! @returns A vector with port pointers to connected ports
std::vector<Port *> MultiPort::getConnectedPorts(const int subPortIdx) const
{
    if (subPortIdx<0)
    {
        //Ok lets return ALL connected ports
        std::vector<Port*> allConnectedPorts;
        for (size_t i=0; i<mSubPortsVector.size(); ++i)
        {
            for (size_t j=0; j<mSubPortsVector[i]->getConnectedPorts().size(); ++j)
            {
                allConnectedPorts.push_back(mSubPortsVector[i]->getConnectedPorts()[j]);
            }
        }

        return allConnectedPorts;
    }
    else
    {
        return mSubPortsVector[subPortIdx]->getConnectedPorts();
    }
}

void MultiPort::setNode(Node* pNode)
{
    HOPSAN_UNUSED(pNode)
    // Do nothing for multiports, only subports are interfaced with
}

PowerMultiPort::PowerMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    MultiPort(rNodeType, rPortName, pParentComponent, pParentPort)
{
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}


//! @brief Adds a subport to a powermultiport
Port* PowerMultiPort::addSubPort()
{
    return MultiPort::addSubPort(PowerPortType);
}

ReadMultiPort::ReadMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort) :
    MultiPort(rNodeType, rPortName, pParentComponent, pParentPort)
{
    setSortHint(Destination);
}

void ReadMultiPort::setSortHint(SortHintEnumT hint)
{
    if (hint == Destination || hint == IndependentDestination)
    {
        mSortHint = hint;
    }
}


//! @brief Adds a subport to a readmultiport
Port* ReadMultiPort::addSubPort()
{
    return MultiPort::addSubPort(ReadPortType);
}

//! @brief A very simple port factory, no need to complicate things with the more advanced one as we will only have a few fixed port types.
//! @param [in] portType The type of port to create
//! @param [in] nodeType The type of node that the port should contain
//! @param [in] name The name of the port
//! @param [in] pPortOwner A pointer to the owner component
//! @param [in] pParentPort A pointer to the parent port in case of creation of a subport to a multiport
//! @return A pointer to the created port
Port* hopsan::createPort(const PortTypesEnumT portType, const HString &rNodeType, const HString &rName, Component *pParentComponent, Port *pParentPort)
{
    switch (portType)
    {
    case PowerPortType :
        return new PowerPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case WritePortType :
        return new WritePort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case BiDirectionalSignalPortType :
        return new BiDirectionalSignalPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case ReadPortType :
        return new ReadPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case SystemPortType :
        return new SystemPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case PowerMultiportType :
        return new PowerMultiPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    case ReadMultiportType :
        return new ReadMultiPort(rNodeType, rName, pParentComponent, pParentPort);
        break;
    default :
       return 0;
    }
}

//! @brief Converts a PortTypeEnum to string
//! @param [in] type The port type enum
//! @return The port type in string format
HString hopsan::portTypeToString(const PortTypesEnumT type)
{
    switch (type)
    {
    case PowerPortType :
        return "PowerPortType";
        break;
    case BiDirectionalSignalPortType :
        return "BiDirectionalSignalPortType";
        break;
    case ReadPortType :
        return "ReadPortType";
        break;
    case WritePortType :
        return "WritePortType";
        break;
    case SystemPortType :
        return "SystemPortType";
        break;
    case MultiportType:
        return "MultiportType";
        break;
    case PowerMultiportType:
        return "PowerMultiportType";
        break;
    case ReadMultiportType:
        return "ReadMultiportType";
        break;
    default :
        return "UndefinedPortType";
    }
}
