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
//! @file   Node.cpp
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Node base classes
//!
//$Id$

//! @class hopsan::Node
//! @brief The Node base class
//! @ingroup Nodes

#include <fstream>
#include <cassert>
#include <iostream>
#include <sstream>
#include "Node.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "Port.h"
#include "ComponentSystem.h"
#include "Quantities.h"

namespace {
bool anyPortWantsLogging(std::vector<hopsan::Port*>& ports)
{
    for (size_t p=0; p<ports.size(); ++p)
    {
        if (ports[p]->isLoggingEnabled())
        {
            return true;
        }
    }
    return false;
}
}

using namespace std;
using namespace hopsan;

//! Node base class constructor
//! @param [in] datalength The length of the data vector
Node::Node(const size_t datalength)
{
    // Make sure clear (should not really be needed)
    mDataValues.clear();
    mDataStorage.clear();
    mConnectedPorts.clear();

    // Init pointer
    mpOwnerSystem = 0;

    // Set initial node type
    mNodeType = "UndefinedNodeType";

    // Resize
    mDataDescriptions.resize(datalength);
    mDataValues.resize(datalength,0.0);

    // Default disabled logging
    setDoLogIfEnabled(false);
}

Node::~Node()
{
    // Nothing special, but needed to prevent warning about undefined behaviour
}

void Node::setSignalNumDimensions(const size_t numDims)
{
    HOPSAN_UNUSED(numDims)
            // Do nothing by default
}


//!
//! @brief returns the node type
//!
const HString &Node::getNodeType() const
{
    return mNodeType;
}

//! @brief Returns the total number of variables in a node
size_t Node::getNumDataVariables() const
{
    return mDataValues.size();
}


double *Node::getDataPtr(const size_t data_type)
{
    return &mDataValues[data_type];
}


void Node::setNiceName(const HString &rNicename)
{
    mNiceName = rNicename;
}


const HString &Node::getNiceName() const
{
    return mNiceName;
}


//! @brief Set data name and unit for a specified data variable
//! @param [in] id This is the ENUM data id
//! @param [in] rName The variable name
//! @param [in] rShortname The short variable name
//! @param [in] rQuantityOrUnit The quantity or unit of the variable
//! @param [in] vartype The type of the variable
void Node::setDataCharacteristics(const size_t id, const HString &rName, const HString &rShortname, const HString &rQuantityOrUnit, const NodeDataVariableTypeEnumT vartype)
{
    mDataDescriptions[id].id = id;
    mDataDescriptions[id].name = rName;
    mDataDescriptions[id].shortname = rShortname;
    mDataDescriptions[id].varType = vartype;
    mDataDescriptions[id].userModifiableQuantity = false;

    HString bu = gpInternalCoreQuantityRegister->lookupBaseUnit(rQuantityOrUnit);
    // If bu empty then, rUnit was not a quantity
    if (bu.empty())
    {
        mDataDescriptions[id].unit = rQuantityOrUnit;
    }
    // Else rUnit was actually a valid Quantity
    else
    {
        mDataDescriptions[id].quantity = rQuantityOrUnit;
        mDataDescriptions[id].unit = bu;
    }
}


//! Get a specific data name and unit
//! @param [in] id This is the ENUM data id
//! @returns A pointer to the desired description or 0 if out of bounds
const NodeDataDescription* Node::getDataDescription(const size_t id) const
{
    if (id < mDataDescriptions.size())
    {
        return &(mDataDescriptions[id]);
    }
    return 0;
}

//! @brief This function can be used to set unit string and displayName for signal nodes ONLY
void Node::setSignalQuantity(const HString &rQuantity, const HString &rUnit, const size_t dataId)
{
    HOPSAN_UNUSED(rQuantity);
    HOPSAN_UNUSED(rUnit);
    HOPSAN_UNUSED(dataId);
    // Do nothing by default
}

void Node::setSignalQuantityModifyable(bool tf, const size_t dataId)
{
    HOPSAN_UNUSED(tf);
    HOPSAN_UNUSED(dataId);
    // Do nothing by default
}

HString Node::getSignalQuantity(const size_t dataId) const
{
    HOPSAN_UNUSED(dataId);
    return "";
}

bool Node::getSignalQuantityModifyable(const size_t dataId) const
{
    HOPSAN_UNUSED(dataId);
    return false;
}


//! @brief This function gives you the data Id for a named data variable
//! @param [in] rName The data name
//! @return The Id, -1 if requested data name is not found
int Node::getDataIdFromName(const HString &rName) const
{
    for (size_t i=0; i<mDataDescriptions.size(); ++i)
    {
        if (rName == mDataDescriptions[i].name)
        {
            return int(i);
        }
    }
    return -1; //Did not find this name return -1 to signal failure
}


//! @brief Get the vector of data descriptions for the node
//! @returns A pointer to the descriptions vector
const std::vector<NodeDataDescription>* Node::getDataDescriptions() const
{
    return &mDataDescriptions;
}


//! @brief Copy variable values from this to pNode
//! @param pOtherNode The destination node to copy into
void Node::copyNodeDataValuesTo(Node *pOtherNode) const
{
    // Copy variable values from this to pNode
    if(pOtherNode->getNodeType()==this->getNodeType())
    {
        for(size_t i=0; i<pOtherNode->getNumDataVariables(); ++i)
        {
            //! @todo look over if all vector positions should be set or not.
            pOtherNode->mDataValues[i] = mDataValues[i];
        }
        setTLMNodeDataValuesTo(pOtherNode); //Handles Wave, imp variables and similar
    }
    else
    {
        if (mpOwnerSystem)
        {
            mpOwnerSystem->addFatalMessage("Node::copyNodeDataValuesTo(): Nodes do not have the same type.");
        }
    }
}

void Node::copySignalQuantityAndUnitTo(Node* /*pOtherNode*/) const
{
    // This is only possible in signal nodes
}

void Node::setTLMNodeDataValuesTo(Node* /*pOtherNode*/) const
{
    // This method should be implemented in child Nodes
}


//! @brief Pre allocate memory for the needed amount of log data
void Node::preAllocateLogSpace(const size_t nLogSlots)
{
    // Don't try to allocate if we are not going to log
    if (mDoLog)
    {
        mDataStorage.resize(nLogSlots, vector<double>(mDataValues.size()));
    }
}


//! @brief Copy current data vector into log storage at given logslot
//! @warning No bounds check is done
void Node::logData(const size_t logSlot)
{
    if (mDoLog)
    {
        mDataStorage[logSlot] = mDataValues;
    }
}


//! @brief Returns a pointer to the component with the write port in the node.
//! If connection is ok, any node can only have one write port. If no write port exists, a null pointer is returned.
Component *Node::getWritePortComponentPtr() const
{
    for(size_t i=0; i<mConnectedPorts.size(); ++i)
    {
        if(mConnectedPorts.at(i)->getPortType() == WritePortType)
        {
            return mConnectedPorts.at(i)->getComponent();
        }
    }

    return 0;   //Return null pointer if no write port was found
}


//! @brief Adds a pointer to a port connected to this node
//! @param [in] pPort The port pointer
void Node::addConnectedPort(Port *pPort)
{
    //Prevent duplicate port registration that can happen when "merging" nodes
    //The other code (connect) will be easier to write if we handle this in here
    bool found = false;
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            found = true;
            break;
            //cout << "Warning: you are trying to add a Port that does already exist in this node  (does nothing)" << endl;
        }
    }

    if (!found)
    {
        mConnectedPorts.push_back(pPort);
    }
}


//! @brief Removes a port pointer from this node, NOT delete, only remove
//! @param [in] pPort The port pointer to be removed
void Node::removeConnectedPort(const Port *pPort)
{
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            mConnectedPorts.erase(it);
            break;
        }
    }
}


//! Check if a specified port is connected to this node
//! @param [in] pPort The port pointer to find
//! @return Is specified port connected (true or false)
bool Node::isConnectedToPort(const Port *pPort) const
{
    vector<Port*>::const_iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            return true;
        }
    }
    return false;
}

Port *Node::getSortOrderSourcePort() const
{
    for(size_t i=0; i<mConnectedPorts.size(); ++i)
    {
        if(mConnectedPorts[i]->getSortHint() == Source)
        {
            return mConnectedPorts[i];
        }
    }
    return 0;   //Return null pointer if no source port was found
}


//! @brief Tag this node for logging
//! @param[in] doLog Flag that tags the node for logging or not
void Node::setDoLogIfEnabled(bool doLog)
{
    if(doLog && anyPortWantsLogging(mConnectedPorts))
    {
        mDoLog = true;
    }
    else
    {
        mDoLog = false;
        mDataStorage.clear();
    }
}

//! @brief Returns the number of attached ports of a specific type
int Node::getNumberOfPortsByType(const int type) const
{
    int nPorts = 0;
    std::vector<Port*>::const_iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if ((*it)->getPortType() == type)
        {
            nPorts++;
        }
    }
    return nPorts;
}

size_t Node::getNumConnectedPorts() const
{
    return mConnectedPorts.size();
}

//! @brief Returns a pointer to the ComponentSystem that own this Node
ComponentSystem *Node::getOwnerSystem() const
{
    return mpOwnerSystem;
}

HString hopsan::nodeDataVariableTypeAsString(const NodeDataVariableTypeEnumT type)
{
    switch(type)
    {
    case DefaultType:
        return "Default";
    case IntensityType:
        return "Intensity";
    case FlowType:
        return "Flow";
    case TLMType:
        return "TLM";
    case HiddenType:
        return "Hidden";
    default:
        return "Incorrect Node Variable Type";
    }
}
