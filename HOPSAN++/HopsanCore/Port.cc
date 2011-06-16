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
#include "HopsanEssentials.h"
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"

using namespace std;
using namespace hopsan;

//! Port base class constructor
Port::Port(string node_type, string portname, Component *portOwner, Port *pParentPort)
{
    mPortType = UNDEFINEDPORT;
    mPortName = portname;
    mNodeType = node_type;
    mpComponent = portOwner;
    mpParentPort = pParentPort; //Only used by subports in multiports
    mConnectionRequired = true;
    mConnectedPorts.clear();
    mpNode = 0;
    mpStartNode = 0;
}


//Destructor
Port::~Port()
{
    if (mpStartNode != 0)
    {
        //! Remove the mapping to eventual system parameters to avoid cowboy-writing in memory after deleted port.
        //! dataNames and dataUnits are here just to decide the number of elements in the start node.
        std::vector<std::string> dataNames, dataUnits;
        mpStartNode->getDataNamesAndUnits(dataNames, dataUnits);
        for(int i = 0; i < dataNames.size(); ++i)
        {
            getComponent()->getSystemParent()->getSystemParameters().unMapParameter(mpStartNode->getDataPtr(i));
        }
    }
}



//! Returns the type of node that can be connected to this port
const string Port::getNodeType()
{
    return mNodeType;
}


//! Returns the parent component
Component* Port::getComponent()
{
    if(mpParentPort)
        return mpParentPort->getComponent();
    else
        return mpComponent;
}


//! Returns a pointer to the connected node
Node* Port::getNodePtr(const size_t /*portIdx*/)
{
    return mpNode;
}

//! Adds a subport to a multiport
Port* Port::addSubPort()
{
    assert("This should only be implemented and called from multiports" == 0);
    return 0;
}

//! REmoves a subport from multiport
void Port::removeSubPort(Port* /*ptr*/)
{
    assert("This should only be implemented and called from multiports" == 0);
}


//! @brief Load start values by copying the start values from the port to the node
void Port::loadStartValues()
{
    if((isConnected()) && mpStartNode)
    {
        this->mpStartNode->copyNodeVariables(mpNode);
    }
}


//! @brief Load start values to the start value container from the node (last values from simulation)
void Port::loadStartValuesFromSimulation()
{
    if((isConnected()) && mpStartNode)
    {
        this->mpNode->copyNodeVariables(mpStartNode);
    }
}


//! Reads a value from the connected node
//! @param [in] idx The data id of the data to read
//! @return The data value
double Port::readNode(const size_t idx, const size_t /*portIdx*/)
{
    //! @todo ummm??, if this is a readport node and it is not connected then noone will ever read
    //! @todo maybe use dummy nodes with 0 in for not connected ports
    /*if((this->getPortType() == Port::READPORT) && (!this->isConnected()))      //Signal nodes don't have to be connected
    {
        return 0;
    }*/
    //return mpNode->getData(idx);
    //cout << "readNode: " << mpNode << endl;
    return mpNode->mDataVector[idx];//Test for speed up
}


//! Writes a value to the connected node
//! @param [in] idx The data id of the data to write
//! @param [in] value The value of the data to read
void Port::writeNode(const size_t &idx, const double &value, const size_t /*portIdx*/)
{
    //! @todo ummm??, if this is a writeport and it is not connected then noone will ever write. Should the check may be done?
    /*if((this->getPortType() == Port::WRITEPORT) && (!this->isConnected()))     //Signal nodes don't have to be connected
    {
        return;
    }*/
    mpNode->mDataVector[idx] = value;//Test for speed up
    //mpNode->setData(idx, value);
}


double *Port::getNodeDataPtr(const size_t idx, const size_t /*portIdx*/)
{
    return mpNode->getDataPtr(idx);
}



//! Set the node that the port is connected to
//! @param [in] pNode A pointer to the Node
void Port::setNode(Node* pNode, const size_t /*portIdx*/)
{
    mpNode = pNode;
    //mIsConnected = true; //!< @todo do we really need this bool, we can compare pointer != 0 instead
}


//! Adds a pointer to an other connected port to a port
//! @param [in] pPort A pointer to the other port
void Port::addConnectedPort(Port* pPort, const size_t /*portIdx*/)
{
    mConnectedPorts.push_back(pPort);
}


//! Removes a pointer to an other connected port from a port
//! @param [in] pPort The pointer to the other port to be removed
void Port::eraseConnectedPort(Port* pPort, const size_t /*portIdx*/)
{
    vector<Port*>::iterator it;
    bool found = false;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            mConnectedPorts.erase(it);
            found = true;
            //If this was the last port removed the clear rhe node ptr
            if (mConnectedPorts.size() == 0)
            {
                mpNode = 0;
            }
            break;
        }
    }
    if (!found)
    {
        cout << "Error: You tried to erase port ptr that did not exist in the connected ports list" << endl;
    }
}


//! Get a vector of pointers to all other ports connected connected to this one
//! @returns A refernce to the internal vector of connected port pointers
//! @todo maybe should return const vector so that contents my not be changed
vector<Port*> &Port::getConnectedPorts(const int /*portIdx*/)
{
    return mConnectedPorts;
}


//! Calls the save log data function of the connected node (if any)
void Port::saveLogData(string filename, const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        mpNode->saveLogData(filename);
    }
    else
    {
        cout << getComponentName() << "-port:" << mPortName << " can not log data, the Port has no Node connected" << endl;
        assert(false);
    }
}


//! Get all data names and units from the connected node
//! @param [in,out] rNames This vector will contain the names
//! @param [in,out] rUnits This vector will contain the units
void Port::getNodeDataNamesAndUnits(vector<string> &rNames, vector<string> &rUnits, const size_t /*portIdx*/)
{
    if(this->isConnected())
    {
        mpNode->getDataNamesAndUnits(rNames, rUnits);
    }
    else
    {
        return;
    }
}


//! @brief Get node data name and unit for specific node data
//! @param [in] dataid The node data id
//! @param [in,out] rName This string will contain the name
//! @param [in,out] rUnit This string will contain the unit
void Port::getNodeDataNameAndUnit(const size_t dataid, string &rName, string &rUnit, const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        rName = mpNode->getDataName(dataid);
        rUnit = mpNode->getDataUnit(dataid);
    }
    else
    {
        rName = "";
        rUnit = "";
    }
}


//! @brief Wraper for the Node function
int Port::getNodeDataIdFromName(const string name, const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        return mpNode->getDataIdFromName(name);
    }
    else
    {
        return -1;
    }
}


vector<double> *Port::getTimeVectorPtr(const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        return  &(mpNode->mTimeStorage);
    }
    else
    {
        return 0;
    }
}


vector<vector<double> > *Port::getDataVectorPtr(const size_t /*portIdx*/)
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


//! @brief Read the start values to a start value node in the port
//! @param[out] rNames is the Vector of names of the star values
//! @param[out] rValues is the Vector of values of the star values, if it is mapped to a System parameter the value of this will be here
//! @param[out] rUnits is the Vector of units of the star values
void Port::getStartValueDataNamesValuesAndUnits(vector<string> &rNames, std::vector<double> &rValues, vector<string> &rUnits, const size_t /*portIdx*/)
{
    if(mpStartNode)
    {
        mpStartNode->getDataNamesValuesAndUnits(rNames, rValues, rUnits);
    }
}


//! @brief Read the start values to a start value node in the port
//! @param[out] rNames is the Vector of names of the star values
//! @param[out] rValues is the Vector of values of the star values, if it is mapped to a System parameter the name of this will be here
//! @param[out] rUnits is the Vector of units of the star values
void Port::getStartValueDataNamesValuesAndUnits(vector<string> &rNames, std::vector<std::string> &rValuesTxt, std::vector<std::string> &rUnits, const size_t /*portIdx*/)
{
    if(mpStartNode)
    {
        std::vector<double> values;
        getStartValueDataNamesValuesAndUnits(rNames, values, rUnits);
        rValuesTxt.resize(values.size());
        for(int i = 0; i < rNames.size(); ++i)
        {
            //Get a pointer to the actual node data
            double *nodeDataPtr = mpStartNode->getDataPtr(mpStartNode->getDataIdFromName(rNames[i]));
            //Check if the nodeDataPtr is in the System parameters
            std::string valueTxt = getComponent()->getSystemParent()->getSystemParameters().findOccurrence(nodeDataPtr);
            if(!(valueTxt.empty()))
            {
                //The nodeDataPrt is connected to a System parameter, read out this name
                rValuesTxt[i] = valueTxt;
            }
            else
            {
                //The nodeDataPrt is not connected to a System parameter, read out the node data value to the string
                std::ostringstream oss;
                oss << values[i];
                rValuesTxt[i] = oss.str();
            }
        }
    }
}


//! @brief Sets start values to a start value node in the port
//! @param[in] names is a Vector of names to be set
//! @param[in] values is a Vector of start values to be set
bool Port::setStartValueDataByNames(vector<string> names, std::vector<double> values, const size_t /*portIdx*/)
{
    bool success = false;
    if(mpStartNode)
    {
        //Remove references from the System parameters if any
        for(int i = 0; i < names.size(); ++i)
        {
            //Get a pointer to the actual node data
            double *nodeDataPtr = mpStartNode->getDataPtr(mpStartNode->getDataIdFromName(names[i]));
            getComponent()->getSystemParent()->getSystemParameters().unMapParameter(nodeDataPtr);
        }
        //Write the value to the start value node
        success = mpStartNode->setDataValuesByNames(names, values);
    }
    return success;
}


//! @brief Sets start values to a start value node in the port
//!
//! Observe that this method is ONLY used to map System parameters to the start values!
//!
//! @param[in] names is a Vector of names to be set
//! @param[in] sysParNames is a Vector of names of System parameters that should be associated to the start value
bool Port::setStartValueDataByNames(vector<std::string> names, std::vector<std::string> sysParNames, const size_t /*portIdx*/)
{
    cout << "In setStartValueDataByNames()" << endl;
    bool success = false;
    if(mpStartNode)
    {
        success = true;
        std::vector<double> values;
        values.resize(sysParNames.size());
        for(int i = 0; i < sysParNames.size(); ++i)
        {
            getComponent()->getSystemParent()->getSystemParameters().getValue(sysParNames[i], values[i]);
            //Get a pointer to the actual node data
            double *nodeDataPtr = mpStartNode->getDataPtr(mpStartNode->getDataIdFromName(names[i]));
            //Map the node data to the System parameter
            success = getComponent()->getSystemParent()->getSystemParameters().mapParameter(sysParNames[i], nodeDataPtr);
        }
        success *= mpStartNode->setDataValuesByNames(names, values);
    }
    return success;
}


//! @brief Get the an actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @returns the start value
double Port::getStartValue(const size_t idx, const size_t /*portIdx*/)
{
    if(mpStartNode)
        return mpStartNode->getData(idx);
    assert(false);
    return 0.0;
}


//! @brief Set the an actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @param[in] value is the start value that should be written
void Port::setStartValue(const size_t &idx, const double &value, const size_t /*portIdx*/)
{
    if(mpStartNode)
    {
        mpStartNode->setData(idx, value);

        //Overwrite the default start value with new value
        //! @todo Make sure this works for multiports
        std::map<std::string, double> tempMap = getComponent()->mDefaultParameters;
        if(tempMap.find(this->getPortName() + mpStartNode->getDataName(idx)) != tempMap.end())
        {
            getComponent()->mDefaultParameters.find(this->getPortName() + mpStartNode->getDataName(idx))->second = value;
        }
        else
        {
            //Do something else?!
        }
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Tried to add StartValue for to Component: " +\
                                              getComponentName() + "::" + getPortName() +\
                                              " This was ignored because this port does not have any StartValue to set.");
    }
}


void Port::disableStartValue(const size_t &idx)
{
    mpStartNode->mDataNames.at(idx) = "";
}


//! Check if the port is curently connected
bool Port::isConnected()
{
    return (mConnectedPorts.size() > 0);
}

//! Check if this port is connected to other port
//! @todo how do we handle multiports
bool Port::isConnectedTo(Port *pOtherPort)
{
    std::vector<Port*>::iterator pit;
    for(pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        if ( *pit == pOtherPort )
        {
            return true;
        }
    }
    return false;
}


//! Check if the port MUST be connected
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
    return (mPortType > MULTIPORT);
}

//! Get the port type
PORTTYPE Port::getPortType()
{
    return mPortType;
}

//! Get the External port type (virtual, should be overloaded in systemports only)
PORTTYPE Port::getExternalPortType()
{
    return getPortType();
}

//! Get the Internal port type (virtual, should be overloaded in systemports only)
PORTTYPE Port::getInternalPortType()
{
    return getPortType();
}


//! Get the port name
const string &Port::getPortName()
{
    return mPortName;
}


//! Get the name of the commponent that the port is attached to
const string &Port::getComponentName()
{
    return getComponent()->getName();
}


//! SystemPort constructor
SystemPort::SystemPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = SYSTEMPORT;
}

//! Get the External port type (virtual, should be overloaded in systemports only)
PORTTYPE SystemPort::getExternalPortType()
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

//! Get the Internal port type (virtual, should be overloaded in systemports only)
PORTTYPE SystemPort::getInternalPortType()
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


//! PowerPort constructor
PowerPort::PowerPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = POWERPORT;
    if(getComponent()->isComponentC())
    {
        mpStartNode = HopsanEssentials::getInstance()->createNode(mNodeType);

        // Skipp this if parent port is set, that is if we are a subport in a multiport
        if (mpParentPort == 0)
        {
            //Copy all start values to default parameters map in component
            std::vector<std::string> names, data;
            mpStartNode->getDataNamesAndUnits(names, data);
            for(int i=0; i<names.size(); ++i)
            {
                getComponent()->mDefaultParameters.insert(std::pair<std::string, double>(portname + mpStartNode->getDataName(i), mpStartNode->getData(i)));
                std::cout << "Writing " << portname << mpStartNode->getDataName(i) << " with " << mpStartNode->getData(i) << endl;
            }
        }
    }
}


ReadPort::ReadPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = READPORT;
}


void ReadPort::writeNode(const size_t /*idx*/, const double /*value*/)
{
    assert("Could not write to port, this is a ReadPort" == 0);
}


WritePort::WritePort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = WRITEPORT;
    mpStartNode = HopsanEssentials::getInstance()->createNode(mNodeType);

    //Copy start value to default parameters map in component
    std::vector<std::string> names, data;
    mpStartNode->getDataNamesAndUnits(names, data);
    portOwner->mDefaultParameters.insert(std::pair<std::string, double>(portname + names.at(0), mpStartNode->getData(0)));
}


double WritePort::readNode(const size_t /*idx*/)
{
    assert("Could not read from port, this is a WritePort" == 0);
    return 0;
}

MultiPort::MultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = MULTIPORT;
}

MultiPort::~MultiPort()
{
    //Deleate all subports thay may remain, if everything is working this shoudl be zero
    assert(mSubPortsVector.size() == 0); //should be removed by other code, use this assert to check if that is working
}

double MultiPort::readNode(const size_t idx, const size_t portIdx)
{
    return mSubPortsVector[portIdx]->readNode(idx);
}

void MultiPort::writeNode(const size_t &idx, const double &value, const size_t portIdx)
{
    return mSubPortsVector[portIdx]->writeNode(idx,value);
}

double *MultiPort::getNodeDataPtr(const size_t idx, const size_t portIdx)
{
    return mSubPortsVector[portIdx]->getNodeDataPtr(idx);
}

void MultiPort::saveLogData(std::string filename, const size_t portIdx)
{
    return mSubPortsVector[portIdx]->saveLogData(filename);
}

void MultiPort::getNodeDataNamesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rUnits, const size_t portIdx)
{
    mSubPortsVector[portIdx]->getNodeDataNamesAndUnits(rNames, rUnits);
}

void MultiPort::getNodeDataNameAndUnit(const size_t dataid, std::string &rName, std::string &rUnit, const size_t portIdx)
{
    mSubPortsVector[portIdx]->getNodeDataNameAndUnit(dataid, rName, rUnit);
}

int MultiPort::getNodeDataIdFromName(const std::string name, const size_t portIdx)
{
    return mSubPortsVector[portIdx]->getNodeDataIdFromName(name);
}

std::vector<double> *MultiPort::getTimeVectorPtr(const size_t portIdx)
{
    return mSubPortsVector[portIdx]->getTimeVectorPtr();
}

std::vector<std::vector<double> > *MultiPort::getDataVectorPtr(const size_t portIdx)
{
    return mSubPortsVector[portIdx]->getDataVectorPtr();
}

void MultiPort::loadStartValues()
{
    //! @todo what should we do here actaully, from where should we copy the starvalues and where to, maybe we should tell the component programmer to fix this
}

void MultiPort::loadStartValuesFromSimulation()
{
    //! @todo what about this one then how should we handle this
}

//! Check if the port is curently connected
bool MultiPort::isConnected()
{
    //! @todo actaully we should check all subports if they are connected
    return (mSubPortsVector.size() > 0);
}

size_t MultiPort::getNumPorts()
{
    return mSubPortsVector.size();
}

//! Removes a specific subport
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

//! Retreives Node Ptr from given subnode
Node *MultiPort::getNodePtr(const size_t portIdx)
{
    assert(mSubPortsVector.size() > portIdx);
    return mSubPortsVector[portIdx]->getNodePtr();
}

//! we use -1 as portindex to indicate that we want all subports
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

PowerMultiPort::PowerMultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : MultiPort(node_type, portname, portOwner, pParentPort)
{
    mPortType = POWERMULTIPORT;
    if(getComponent()->isComponentC())
    {
        mpStartNode = HopsanEssentials::getInstance()->createNode(mNodeType);
    }
}

//! Adds a subport to a powermultiport
Port* PowerMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(POWERPORT, mNodeType, "noname subport", 0, this) );
    return mSubPortsVector.back();
}

ReadMultiPort::ReadMultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : MultiPort(node_type, portname, portOwner, pParentPort)
{
    mPortType = READMULTIPORT;
}

//! Adds a subport to a readmultiport
Port* ReadMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(READPORT, mNodeType, "noname subport", 0, this) );
    return mSubPortsVector.back();
}

//!
//! @brief Very simple port factory, no need to complicate things with the more advanced one as we will only have a few fixed port types.
//!
Port* hopsan::createPort(PORTTYPE porttype, NodeTypeT nodetype, string name, Component *portOwner, Port *pParentPort)
{
    switch (porttype)
    {
    case POWERPORT :
        return new PowerPort(nodetype, name, portOwner, pParentPort);
        break;
    case WRITEPORT :
        return new WritePort(nodetype, name, portOwner, pParentPort);
        break;
    case READPORT :
        return new ReadPort(nodetype, name, portOwner, pParentPort);
        break;
    case SYSTEMPORT :
        return new SystemPort(nodetype, name, portOwner, pParentPort);
        break;
    case POWERMULTIPORT :
        return new PowerMultiPort(nodetype, name, portOwner, pParentPort);
        break;
    case READMULTIPORT :
        return new ReadMultiPort(nodetype, name, portOwner, pParentPort);
        break;
    default :
       assert(false); //Should not be able to create any other port type
       return 0;
    }
}

//! @brief Get the port type as a string
std::string hopsan::portTypeToString(const PORTTYPE type)
{
    switch (type)
    {
    case POWERPORT :
        return "POWERPORT";
        break;
    case READPORT :
        return "READPORT";
        break;
    case WRITEPORT :
        return "WRITEPORT";
        break;
    case SYSTEMPORT :
        return "SYSTEMPORT";
        break;
    case MULTIPORT:
        return "MULTIPORT";
        break;
    case POWERMULTIPORT:
        return "POWERMULTIPORT";
        break;
    case READMULTIPORT:
        return "READMULTIPORT";
        break;
    default :
        return "UNDEFINEDPORT";
    }
}
