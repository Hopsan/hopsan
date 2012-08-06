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
//! @file   Node.cc
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

using namespace std;
using namespace hopsan;

//! Node base class constructor
//! @param [in] datalength The length of the data vector
Node::Node(const size_t datalength)
{
    //Make sure clear (should not really be needed)
    mDataValues.clear();
    mDataStorage.clear();
    //mTimeStorage.clear();
    mConnectedPorts.clear();

    //Init pointer
    mpOwnerSystem = 0;

    //Set initial node type
    mNodeType = "UndefinedNodeType";

    //Resize
    mDataDescriptions.resize(datalength);
    mDataValues.resize(datalength,0.0);

    //Set log specific variables
    disableLog();       //Default log node dissabled
    //mLogTimeDt = -1.0;
    //mLastLogTime = 0.0; //Initial valus should not matter, will be overwritten when selecting log amount
    //mLogSlots = 0;
    //mLogCtr = 0;
}


//!
//! @brief returns the node type
//!
const NodeTypeT Node::getNodeType() const
{
    return mNodeType;
}

//! @brief Returns the total number of variables in a node
size_t Node::getNumDataVariables() const
{
    return mDataValues.size();
}


//!
//! @brief set data in node
//! @param [in] data_type Identifier for the typ of node data to set
//! @param [in] data The data value
//!
void Node::setDataValue(const size_t data_type, const double data)
{
    mDataValues[data_type] = data;
}


//!
//! @brief get data from node
//! @param [in] data_type Identifier for the type of node data to get
//! @return The data value
//!
double Node::getDataValue(const size_t data_type) const
{
    return mDataValues[data_type];
}


double *Node::getDataPtr(const size_t data_type)
{
    return &mDataValues[data_type];
}


//! @brief Set data name and unit for a specified data variable
//! @param [in] id This is the ENUM data id
//! @param [in] name The variable name
//! @param [in] unit The variable unit
void Node::setDataCharacteristics(const size_t id, const string name, const string unit, const NodeDataVariableTypeT vartype)
{
    mDataDescriptions[id].id = id;
    mDataDescriptions[id].name = name;
    mDataDescriptions[id].unit = unit;
    mDataDescriptions[id].varType = vartype;
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


//! @brief This function gives you the data Id for a names data variable
//! @param [in] name The data name
//! @return The Id, -1 if requested data name is not found
int Node::getDataIdFromName(const string name)
{
    for (size_t i=0; i<mDataDescriptions.size(); ++i)
    {
        if (name == mDataDescriptions.at(i).name)
        {
            return i;
        }
    }
    return -1; //Did not find this name return -1 to signal failure
}


//bool Node::setDataValuesByNames(vector<string> names, std::vector<double> values)
//{
//    bool success = true;
//    for(size_t i=0; i<names.size(); ++i)
//    {
//        this->setData(this->getDataIdFromName(names[i]),values[i]);
//        //! @todo introduce setDataSafe and similar at many places in code
//    }
//    return success;
//}


//! @brief Get the vector of data descriptions for the node
//! @returns A pointer to the descriptions vector
const std::vector<NodeDataDescription>* Node::getDataDescriptions() const
{
    return &mDataDescriptions;
}


void Node::copyNodeDataValuesTo(Node *pNode)
{
    // this ska kopiera sina varabler till pNode
    assert(pNode->getNodeType()==this->getNodeType());
    if(pNode->getNodeType()==this->getNodeType())
    {
        for(size_t i=0; i<pNode->getNumDataVariables(); ++i)
        {
            //cout << "Name: " << mDataNames[i] << "  Value: " << mDataVector[i] << "  , " << pNode->mDataVector[i] << "  Unit: " << mDataUnits[i] << endl;
            //! @todo look over if all vector positions should be set or not.
            //if(mPlotBehaviour[i] == Node::PLOT)
            {
                //pNode->mDataNames[i] = mDataNames[i];
                pNode->mDataValues[i] = mDataValues[i];
                //pNode->mDataUnits[i] = mDataUnits[i];
            }
        }
        setSpecialStartValues(pNode); //Handles Wave, imp variables and similar
    }
}

void Node::setSpecialStartValues(Node* /*pNode*/)
{
    //This method schould be implemented in child Nodes
    //cout << "This nodetype seem not to have any hidden variables for the user." << endl;
}


//! @brief Pre allocate memory for the needed amount of log data
bool Node::preAllocateLogSpace(const size_t nLogSlots)
{
    // Dont try to allocate if we are not going to log
    if (mDoLog)
    {
        // Now try to allocate log memmory
        try
        {
            mDataStorage.resize(nLogSlots, vector<double>(mDataValues.size()));
            //cout << "requestedSize: " << mLogSlots << " " << data_size << " Capacities: " << mTimeStorage.capacity() << " " << mDataStorage.capacity() << " " << mDataStorage[1].capacity() << " Size: " << mTimeStorage.size() << " " << mDataStorage.size() << " " << mDataStorage[1].size() << endl;
            return true;
        }
        catch (exception &e)
        {
            //cout << "preAllocateLogSpace: Standard exception: " << e.what() << endl;
            gCoreMessageHandler.addErrorMessage("Failed to allocate log data memmory, try reducing the amount of log data", "FailedMemmoryAllocation");
            mDoLog = false;
            return false;
        }
    }
    return true; //Success allocating nothing
}


////! Copy current data vector into log storage, also adds current time
//void Node::logData(const double time)
//{
//    if (mDoLog)
//    {
//        //! @todo Danger comparing doubles
//        //! @todo is this correct, Subtract a tenth of logDt to avoid numerical problem with double >= double
//        //! @todo since all nodes in a system will all do this calculation maybe we could speed things up by do ing this check once, in the system and only call this function in all nodes when needed
//        if (time >= mLastLogTime+mLogTimeDt-mLogTimeDt/10.0)
//        {
//            //cout << "mLogCtr: " << mLogCtr << endl;
//            //! @todo this if check should not be needed if everything else is working
//            if (mLogCtr < mTimeStorage.size())
//            {
//                //! @todo maybe time vector should be in the system instead, since all nodes in the same system will have the same time vector
//                mTimeStorage[mLogCtr] = time;   //We log the "real"  simulation time for the sample
//                mDataStorage[mLogCtr] = mDataValues;
//            }else
//            {
//                stringstream ss;
//                ss << "mLogCtr >= mTimeStorage.size() " << mLogCtr;
//                //gCoreMessageHandler.addWarningMessage(ss.str());
//            }
//            ++mLogCtr;

//            mLastLogTime = mLastLogTime+mLogTimeDt; //Can not use "real" time directly as this may mean that not all log slots will be filled
//        }
//    }
//}

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
Component *Node::getWritePortComponentPtr()
{
    for(size_t i=0; i<mConnectedPorts.size(); ++i)
    {
        if(mConnectedPorts.at(i)->getPortType() == WRITEPORT)
        {
            return mConnectedPorts.at(i)->getComponent();
        }
    }

    return 0;   //Return null pointer if no write port was found
}

//! @brief This function can be used to set unit string for signal nodes ONLY
void Node::setSignalDataUnit(const string unit)
{
    //Do nothing by default
}

//! @brief This function can be used to set name string for signal nodes ONLY
void Node::setSignalDataName(const string name)
{
    //Do nothing by default
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


//! @brief Removes a port poniter from this node, NOT delete only remove
//! @param [in] pPort The port pointer to be removed
void Node::removeConnectedPort(Port *pPort)
{
    bool found = false;
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            mConnectedPorts.erase(it);
            found = true;
            break;
        }
    }

    if (!found)
    {
        cout << "Warning: you are trying to remove a Port that does not exist in this node  (does nothing)" << endl;
    }
}


//! Check if a specified port is connected to this node
//! @param [in] pPort The port pointer to find
//! @return Is specified port connected (true or false)
bool Node::isConnectedToPort(Port *pPort)
{
    vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            return true;
        }
    }
    return false;
}


//! Enable node data logging
void Node::enableLog()
{
    mDoLog = true;
    //cout << "enableLog" << endl;
}


//! Disable node data logging
void Node::disableLog()
{
    mDoLog = false;
    //cout << "disableLog" << endl;
    // If log dissabled then free memory if something has been previously allocated
    //mTimeStorage.clear();
    mDataStorage.clear();
}


int Node::getNumberOfPortsByType(int type)
{
    int nPorts = 0;
    std::vector<Port*>::iterator it;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if ((*it)->getPortType() == type)
        {
            nPorts++;
        }
    }
    return nPorts;
}

//! @brief Returns a pointer to the ComponentSystem that own this Node
ComponentSystem *Node::getOwnerSystem()
{
    return mpOwnerSystem;
}
