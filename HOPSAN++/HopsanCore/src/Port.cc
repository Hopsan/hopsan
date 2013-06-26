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
//! @file   Port.cc
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

using namespace std;
using namespace hopsan;

//! @brief Port base class constructor
Port::Port(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort)
{
    mPortType = UndefinedPortType;
    mPortName = rPortName;
    mNodeType = rNodeType;
    mpComponent = pParentComponent;
    mpParentPort = pParentPort; //Only used by subports in multiports
    mConnectionRequired = true;
    mConnectedPorts.clear();
    mpNode = 0;
    mpStartNode = 0;
    //mpTempAlias=0;

    // Create the initial node
    mpNode = getComponent()->getHopsanEssentials()->createNode(mNodeType.c_str());
    this->setNode(mpNode);
    if (getComponent()->getSystemParent())
    {
        getComponent()->getSystemParent()->addSubNode(mpNode);
    }
}


//Destructor
Port::~Port()
{
    if (mpStartNode != 0)
    {
        //! Remove the mapping to eventual system parameters to avoid cowboy-writing in memory after deleted port.
//        for(size_t i = 0; i < mpStartNode->getNumDataVariables(); ++i)
//        {
//FIXA, the parameters will probably be deleted when parent component is deleted -> no problems            getComponent()->getSystemParent()->getSystemParameters().unMapParameter(mpStartNode->getDataPtr(i));
//        }
    }

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

    //free(mpTempAlias);
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
Node* Port::getNodePtr(const size_t /*portIdx*/)
{
    return mpNode;
}

//! @brief Adds a subport to a multiport
Port* Port::addSubPort()
{
    mpComponent->addFatalMessage("Port::addSubPort(): This should only be implemented and called from multiports.");
    return 0;
}

//! @brief Removes a subport from multiport
void Port::removeSubPort(Port* /*ptr*/)
{
    mpComponent->addFatalMessage("Port::removeSubPort(): This should only be implemented and called from multiports.");
}


//! @brief Load start values by copying the start values from the port to the node
void Port::loadStartValues()
{
    if(mpStartNode)
    {
        mpStartNode->copyNodeDataValuesTo(mpNode);
        mpStartNode->copySignalDataUnitAndDescriptionTo(mpNode);
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
//! @return The data value
double Port::readNodeSafe(const size_t idx, const size_t /*portIdx*/)
{
    if (idx < mpNode->getNumDataVariables())
    {
        return mpNode->mDataValues[idx];
    }
    getComponent()->addErrorMessage("data idx out of range in Port::readNodeSafe()");
    return -1;
}


//! @brief Writes a value to the connected node
//! @param [in] idx The data id of the data to write
//! @param [in] value The value of the data to read
void Port::writeNodeSafe(const size_t idx, const double value, const size_t /*portIdx*/)
{
    if (idx < mpNode->getNumDataVariables())
    {
        mpNode->mDataValues[idx] = value;
    }
    else
    {
        getComponent()->addErrorMessage("data idx out of range in Port::writeNodeSafe()");
    }
}

const Node *Port::getNodePtr(const size_t /*portIdx*/) const
{
    return mpNode;
}

//! @brief Get a ptr to the data variable in the node
//! @param [in] idx The id of the data variable to return ptr to
//! @returns Pointer to data vaariable or 0 if idx was not found
double *Port::getNodeDataPtr(const size_t idx, const size_t /*portIdx*/) const
{
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
//! @param [in] pNode A pointer to the Node, or 0 for NC dummy node
void Port::setNode(Node* pNode)
{
    if (!pNode)
    {
        getComponent()->addFatalMessage("In Port::setNode(), You cant set a NULL node ptr");
    }
    else
    {
        //! @todo what to do with log data (clear maybe) if dummy node
        mpNode->removeConnectedPort(this);
        mpNode = pNode;
        mpNode->addConnectedPort(this);
    }
}


//! @brief Adds a pointer to an other connected port to a port
//! @param [in] pPort A pointer to the other port
void Port::addConnectedPort(Port* pPort, const size_t /*portIdx*/)
{
    mConnectedPorts.push_back(pPort);
}


//! @brief Removes a pointer to an other connected port from a port
//! @param [in] pPort The pointer to the other port to be removed
void Port::eraseConnectedPort(Port* pPort, const size_t /*portIdx*/)
{
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
//! @returns A refernce to the internal vector of connected port pointers
//! @todo maybe should return const vector so that contents my not be changed
vector<Port*> &Port::getConnectedPorts(const int /*portIdx*/)
{
    return mConnectedPorts;
}

size_t Port::getNumConnectedPorts(const int portIdx)
{
    return getConnectedPorts(portIdx).size();
}


void Port::createStartNode(const HString &rNodeType)
{
    mpStartNode = getComponent()->getHopsanEssentials()->createNode(rNodeType.c_str());
    //!< @todo Maye I dont even need to create startnodes for subports in multiports, in that case, move this line into if bellow

    // Prevent registering startvalues for subports in multiports, It will be very difficult to ensure that those would actually work as expected
    if (mpParentPort == 0)
    {
        for(size_t i = 0; i < mpStartNode->getNumDataVariables(); ++i)
        {
            const NodeDataDescription* pDesc = mpStartNode->getDataDescription(i);
            const HString desc = "startvalue: Port "+getName();
            const HString name = getName()+"#"+pDesc->name;
            getComponent()->addConstant(name, desc, pDesc->unit, *(mpStartNode->getDataPtr(pDesc->id)));
        }
    }
}

//! @note This one should be called by system, do not call this manually (that will create a mess)
void Port::setVariableAlias(const HString &rAlias, const int id)
{
    //! @todo check id
    // First remove it if already set
    std::map<HString, int>::iterator it = mVariableAliasMap.begin();
    while (it!=mVariableAliasMap.end())
    {
        if (it->second == id)
        {
            mVariableAliasMap.erase(it);
            // Restart search if something was removed as itterator breaks
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
        mVariableAliasMap.insert(std::pair<HString, int>(rAlias, id));
    }
}

//! @todo return reference to string instead, if that is possible
const HString &Port::getVariableAlias(const int id)
{
    std::map<HString, int>::const_iterator it;
    for(it=mVariableAliasMap.begin();it!=mVariableAliasMap.end();++it)
    {
        if (it->second == id)
        {
            //copyString(&mpTempAlias, it->first);
            //return mpTempAlias;
            return it->first;
        }
    }
//    copyString(&mpTempAlias, "");
//    return mpTempAlias;
    return mEmptyString;
}

int Port::getVariableIdByAlias(const HString &rAlias) const
{
    std::map<HString, int>::const_iterator it = mVariableAliasMap.find(rAlias);
    {
        if (it!=mVariableAliasMap.end())
        {
            return it->second;
        }
    }
    return -1;
}

Node *Port::getStartNodePtr()
{
    return mpStartNode;
}


////! @brief Debug function to dump logged node data to a file
////! @param [in] filename The name of the file to write to
//void Port::saveLogData(string filename, const size_t /*portIdx*/)
//{
//    if (mpNode != 0)
//    {
//        HString header = getComponentName() + "::" + getName();

//        ofstream out_file;
//        out_file.open(filename.c_str());
//        if (out_file.good())
//        {
//            vector<double>* pTimeStorage = mpNode->getOwnerSystem()->getLogTimeVector();
//            if(pTimeStorage->size() != mpNode->mDataStorage.size())
//            {
//                mpComponent->addFatalMessage("Port::saveLogData(): pTimeStorage->size() != mpNode->mDataStorage.size()");
//            }

//            // First write HEADER info containing node info
//            out_file << header.c_str() << " " << mpNode->getNodeType().c_str() << endl;
//            out_file << "time";
//            for (size_t i=0; i<mpNode->getNumDataVariables(); ++i)
//            {
//                out_file << " " << mpNode->getDataDescription(i)->name;
//            }
//            out_file << endl;

//            //Write log data to file
//            for (size_t row=0; row<pTimeStorage->size(); ++row)
//            {
//                out_file << pTimeStorage->at(row);
//                for (size_t datacol=0; datacol<mpNode->getNumDataVariables(); ++datacol)
//                {
//                    out_file << " " << mpNode->mDataStorage[row][datacol];
//                }
//                out_file << endl;
//            }
//            out_file.close();
//            cout << "Done! Saving node data to file: " << filename << endl;
//        }
//        else
//        {
//            cout << "Warning! Could not open out file for writing: " << filename << endl;
//        }
//    }
//    else
//    {
//        cout << getComponentName().c_str() << "-port:" << mPortName.c_str() << " can not log data, the Port has no Node connected" << endl;
//    }
//}

bool Port::haveLogData(const size_t /*portIdx*/)
{
    if (mpNode)
    {
        // Here we assume that timevector DOES exist. If simulation code is correct it should exist
        return !mpNode->mDataStorage.empty();
    }
    return false;
}


//! @brief Get all data names and units from the connected node
//! @param [in,out] rNames This vector will contain the names
//! @param [in,out] rUnits This vector will contain the units
const std::vector<NodeDataDescription>* Port::getNodeDataDescriptions(const size_t /*portIdx*/)
{
    // We prefere to use the startnode
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


//! @brief Get node data name and unit for specific node data
//! @param [in] dataid The node data id
//! @returns A pointer to teh node data description, or 0 if no node exist
const NodeDataDescription* Port::getNodeDataDescription(const size_t dataid, const size_t /*portIdx*/)
{
    // We prefere to use the startnode
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


//! @brief Wraper for the Node function
int Port::getNodeDataIdFromName(const HString &rName, const size_t /*portIdx*/)
{
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

void Port::setSignalNodeUnitAndDescription(const HString &rUnit, const HString &rDescription)
{
    //! @todo multiport version needed
    mpNode->setSignalDataUnitAndDescription(rUnit, rDescription);
    if (mpStartNode)
    {
        mpStartNode->setSignalDataUnitAndDescription(rUnit, rDescription);
    }
}


vector<double> *Port::getLogTimeVectorPtr(const size_t /*portIdx*/)
{
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


vector<vector<double> > *Port::getLogDataVectorPtr(const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        return &(mpNode->mDataStorage);
    }
    else
    {
        return 0;
    }
}


vector<double> *Port::getDataVectorPtr(const size_t /*portIdx*/)
{
    if(mpNode != 0)
    {
        return &(mpNode->mDataValues);
    }
    else
    {
        return 0;
    }
}

size_t Port::getNumDataVariables() const
{
    return mpNode->getNumDataVariables();
}


//! @brief Get the actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @returns the start value
double Port::getStartValue(const size_t idx, const size_t /*portIdx*/)
{
    if(mpStartNode && getComponent()->getSystemParent()->doesKeepStartValues())
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
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param[in] value is the start value that should be written
void Port::setDefaultStartValue(const size_t idx, const double value, const size_t /*portIdx*/)
{
    if(mpStartNode)
    {
        mpStartNode->setDataValue(idx, value);
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
        // If a startvalue has been disabled you can not change it, (it actually means that it is hiddden)
    }
}


//! @brief Check if the port is curently connected
bool Port::isConnected()
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

//! @brief Convenience functin to check if port is multiport
bool Port::isMultiPort() const
{
    return (mPortType > MultiportType);
}

Port *Port::getParentPort() const
{
    return mpParentPort;
}

//! @brief Get the port type
PortTypesEnumT Port::getPortType() const
{
    return mPortType;
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


//! @brief Get the port name
const HString &Port::getName() const
{
    return mPortName;
}


//! @brief Get the name of the commponent that the port is attached to
const HString &Port::getComponentName() const
{
    return getComponent()->getName();
}

const HString &Port::getDescription() const
{
    return mDescription;
}

void Port::setDescription(const HString &rDescription)
{
    mDescription = rDescription;
}


//! @brief SystemPort constructor
SystemPort::SystemPort(const HString &rNodeType, const HString &rPortName, Component *portOwner, Port *pParentPort) : Port(rNodeType, rPortName, portOwner, pParentPort)
{
    mPortType = SystemPortType;
}

//! @brief Get the External port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getExternalPortType()
{
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        //External ports component parents will belong to the same system as our component parent
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent()->getSystemParent() )
        {
            //! @todo for now we return the first one we find, usually thi is corect except when you are mixing powerports and readports, powerports should be returned in that case but I dont know how to fix this except going through ALL ports every time
            return (*pit)->getPortType();
        }
    }

    //If no external ports found return our actual type (systemport)
    return getPortType();
}

//! @brief Get the Internal port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getInternalPortType()
{
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        //Internal ports component parents will belong to our component parent
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent() )
        {
            //! @todo for now we return the first one we find, usually thi is corect except when you are mixing powerports and readports, powerports should be returned in that case but I dont know how to fix this except going through ALL ports every time
            return (*pit)->getPortType();
        }
    }

    //If no internal ports found return our actual type (systemport)
    return getPortType();
}


//! @brief PowerPort constructor
PowerPort::PowerPort(const HString &rNodeType, const HString &rPortName, Component *portOwner, Port *pParentPort) : Port(rNodeType, rPortName, portOwner, pParentPort)
{
    mPortType = PowerPortType;
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}


ReadPort::ReadPort(const HString &rNodeType, const HString &rPortName, Component *portOwner, Port *pParentPort) : Port(rNodeType, rPortName, portOwner, pParentPort)
{
    mPortType = ReadPortType;
    createStartNode(rNodeType);
}


void ReadPort::writeNodeSafe(const size_t /*idx*/, const double /*value*/)
{
    mpComponent->addErrorMessage("ReadPort::writeNodeSafe(): Could not write to port, this is a ReadPort.");
}


void ReadPort::writeNode(const size_t /*idx*/, const double /*value*/) const
{
    mpComponent->addErrorMessage("ReadPort::writeNode(): Could not write to port, this is a ReadPort.");
}

void ReadPort::loadStartValues()
{
    // Prevent loading startvalues if this port is connected, then the write node will set the start value
    if (!isConnected())
    {
        Port::loadStartValues();
    }
}

bool ReadPort::hasConnectedExternalSystemWritePort()
{
    // Frist figure out who my system parent is
    Component *pSystemParent;
    if (this->getComponent()->isComponentSystem())
    {
        // If my parent component is a system, then I am a kind of systemport (I am a normal port as interface port on a system)
        pSystemParent = this->getComponent();
    }
    else
    {
        // Take my parent components systemparent
        pSystemParent = this->getComponent()->getSystemParent();
    }

    // Now check all connected ports, find a write port
    vector<Port*>::iterator portIt;
    for (portIt = mConnectedPorts.begin(); portIt != mConnectedPorts.end(); ++portIt)
    {
        Port *pPort = (*portIt);
        if (pPort->getPortType() == WritePortType)
        {
            // Check if this writport belongs to a component whos parent system is the same as my system grand parent
            if (pPort->getComponent()->getSystemParent() == pSystemParent->getSystemParent())
            {
                return true;
            }
        }
    }
    return false;
}

void ReadPort::forceLoadStartValue()
{
    Port::loadStartValues();
}

WritePort::WritePort(const HString &rNodeType, const HString &rPortName, Component *portOwner, Port *pParentPort) : Port(rNodeType, rPortName, portOwner, pParentPort)
{
    mPortType = WritePortType;
    createStartNode(mNodeType);
}


double WritePort::readNode(const size_t /*idx*/) const
{
    mpComponent->addWarningMessage("WritePort::readNode(): Could not read to port, this is a WritePort");
    return -1;
}

MultiPort::MultiPort(const HString &rNodeType, const HString &rPortName, Component *portOwner, Port *pParentPort) : Port(rNodeType, rPortName, portOwner, pParentPort)
{
    mPortType = MultiportType;
}

MultiPort::~MultiPort()
{
    //Delete all subports thay may remain, if everything is working this shoudl be zero
    //! @todo removed assert, BUT problem needs to be fixed /Peter
    if (mSubPortsVector.size() != 0)
    {
        getComponent()->addFatalMessage("~MultiPort(): mSubPortsVector.size() != 0 in multiport destructor (will fix later)");
    }
    //assert(mSubPortsVector.size() == 0); //should be removed by other code, use this assert to check if that is working
}


double MultiPort::readNodeSafe(const size_t idx, const size_t portIdx)
{
    if (portIdx < mSubPortsVector.size())
    {
        return mSubPortsVector[portIdx]->readNodeSafe(idx);
    }
    getComponent()->addErrorMessage("portIdx out of range in MultiPort::readNodeSafe()");
    return -1;
}

void MultiPort::writeNodeSafe(const size_t idx, const double value, const size_t portIdx)
{
    if (portIdx < mSubPortsVector.size())
    {
        mSubPortsVector[portIdx]->writeNodeSafe(idx,value);
    }
    else
    {
        getComponent()->addErrorMessage("portIdx out of range in MultiPort::readNodeSafe()");
    }
}


double MultiPort::readNode(const size_t idx, const size_t portIdx) const
{
    //! @todo handle portIdx ot of range
    return mSubPortsVector[portIdx]->readNode(idx);
}

void MultiPort::writeNode(const size_t idx, const double value, const size_t portIdx) const
{
    return mSubPortsVector[portIdx]->writeNode(idx,value);
}

const Node *MultiPort::getNodePtr(const size_t portIdx) const
{
    return mSubPortsVector[portIdx]->getNodePtr();
}

//! @todo why do we even want a unsafe getNodeDataPtr it should be the safe version
double *MultiPort::getNodeDataPtr(const size_t idx, const size_t portIdx) const
{
    //If we try to access node data for subport that does not exist then return multiport shared dummy safe ptr
    if (portIdx >= mSubPortsVector.size())
    {
        return Port::getNodeDataPtr(idx, portIdx);
    }
    else
    {
        return mSubPortsVector[portIdx]->getNodeDataPtr(idx);
    }
}


const std::vector<NodeDataDescription>* MultiPort::getNodeDataDescriptions(const size_t portIdx)
{
    if (portIdx < mSubPortsVector.size())
    {
        return mSubPortsVector[portIdx]->getNodeDataDescriptions();
    }
    return 0;
}

const NodeDataDescription* MultiPort::getNodeDataDescription(const size_t dataid, const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getNodeDataDescription(dataid);
    }
    else if (mpStartNode)
    {
        return mpStartNode->getDataDescription(dataid);
    }
    return 0;
}

int MultiPort::getNodeDataIdFromName(const HString &rName, const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getNodeDataIdFromName(rName);
    }
    return -1;
}

bool MultiPort::haveLogData(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->haveLogData();
    }
    return false;
}

std::vector<double> *MultiPort::getLogTimeVectorPtr(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getLogTimeVectorPtr();
    }
    return 0;
}

std::vector<std::vector<double> > *MultiPort::getLogDataVectorPtr(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getLogDataVectorPtr();
    }
    return 0;
}

std::vector<double> *MultiPort::getDataVectorPtr(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getDataVectorPtr();
    }
    return 0;
}

//! @brief Get the an actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @returns the start value
double MultiPort::getStartValue(const size_t idx, const size_t portIdx)
{
    if(mpStartNode && mpComponent->getSystemParent()->doesKeepStartValues())
    {
        return mSubPortsVector[portIdx]->mpNode->getDataValue(idx);
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
            mpStartNode->copySignalDataUnitAndDescriptionTo(mSubPortsVector[p]->mpNode);
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

//! @brief Check if the port is curently connected
bool MultiPort::isConnected()
{
    //! @todo actaully we should check all subports if they are connected (but a subport should not exist if not connected)
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
            delete ptr;
            break;
        }
    }
}

//! @brief Retreives Node Ptr from given subnode
Node *MultiPort::getNodePtr(const size_t portIdx)
{
    if(mSubPortsVector.size() <= portIdx)
    {
        mpComponent->addWarningMessage("MultiPort::getNodePtr(): mSubPortsSVector.size() <= portIdx");
        return 0;
    }
    return mSubPortsVector[portIdx]->getNodePtr();
}

//! @note we use -1 as portindex to indicate that we want all subports
std::vector<Port*> &MultiPort::getConnectedPorts(const int portIdx)
{
    if (portIdx<0)
    {
        //Ok lets return ALL connected ports
        //! @todo since this function returns a reference to the internal vector we need a new memberVector
        mAllConnectedPorts.clear();
        for (size_t i=0; i<mSubPortsVector.size(); ++i)
        {
            for (size_t j=0; j<mSubPortsVector[i]->getConnectedPorts().size(); ++j)
            {
                mAllConnectedPorts.push_back(mSubPortsVector[i]->getConnectedPorts()[j]);
            }
        }

        return mAllConnectedPorts;
    }
    else
    {
        return mSubPortsVector[portIdx]->getConnectedPorts();
    }
}

void MultiPort::setNode(Node* /*pNode*/)
{
    // Do nothing for multiports, only subports are interfaced with
}

PowerMultiPort::PowerMultiPort(const HString &rNodeType, const HString &rPortName, Component *portOwner, Port *pParentPort) : MultiPort(rNodeType, rPortName, portOwner, pParentPort)
{
    mPortType = PowerMultiportType;
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}

//! @brief Adds a subport to a powermultiport
Port* PowerMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(PowerPortType, mNodeType, "noname_subport", 0, this) );
    return mSubPortsVector.back();
}

ReadMultiPort::ReadMultiPort(const HString &rNodeType, const HString &rPortName, Component *portOwner, Port *pParentPort) : MultiPort(rNodeType, rPortName, portOwner, pParentPort)
{
    mPortType = ReadMultiportType;
}

//! @brief Adds a subport to a readmultiport
Port* ReadMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(ReadPortType, mNodeType, "noname_subport", 0, this) );
    return mSubPortsVector.back();
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
