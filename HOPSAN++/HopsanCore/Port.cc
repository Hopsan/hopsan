//!
//! @file   Port.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains Port base class as well as Sub classes
//!
//$Id$

#include "Port.h"
#include "Node.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include "Component.h"

using namespace std;
using namespace hopsan;


//! Port base class constructor
Port::Port(string node_type, string portname, Component *portOwner)
{
    mPortType = UNDEFINEDPORT;
    mPortName = portname;
    mNodeType = node_type;
    mpComponent = portOwner;
    mConnectionRequired = true;
    clearConnection();
    mpStartNode = 0;
}


//Destructor
Port::~Port()
{
    //Nothing for now
}


//! Helper function for quickly clearing all connection info
void Port::clearConnection()
{
    //! @todo maybe should be virtual so that we may also clear node type in system ports
    mpNode = 0;
    mConnectedPorts.clear();
    mIsConnected = false;
}


//! Returns the type of node that can be connected to this port
const string &Port::getNodeType()
{
    return mNodeType;
}


//! Returns referense to connected node (Dont use this)
Node &Port::getNode()
{
    //! @todo error handle if 0
    return *mpNode;
}


//! This one shal be removed
Node* Port::getNodePublic()
{
    return mpNode;
}


//! Returns a pointer to the connected node
Node* Port::getNodePtr()
{
    return mpNode;
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
double Port::readNode(const size_t idx)
{
    //! @todo ummm??, if this is a readport node and it is not connected then noone will ever read
    //! @todo maybe use dummy nodes with 0 in for not connected ports
    if((this->getPortType() == Port::READPORT) && (!this->isConnected()))      //Signal nodes don't have to be connected
    {
        return 0;
    }
    return mpNode->getData(idx);
}


//! Writes a value to the connected node
//! @param [in] idx The data id of the data to write
//! @param [in] value The value of the data to read
void Port::writeNode(const size_t idx, const double value)
{
    //! @todo ummm??, if this is a writeport and it is not connected then noone will ever write
    if((this->getPortType() == Port::WRITEPORT) && (!this->isConnected()))     //Signal nodes don't have to be connected
    {
        return;
    }
    return mpNode->setData(idx, value);
}


//! Set the node that the port is connected to
//! @param [in] pNode A pointer to the Node
void Port::setNode(Node* pNode)
{
    mpNode = pNode;
    mIsConnected = true; //!< @todo do we really need this bool, we can compare pointer != 0 instead
}


//! Adds a pointer to an other connected port to a port
//! @param [in] pPort A pointer to the other port
void Port::addConnectedPort(Port* pPort)
{
    mConnectedPorts.push_back(pPort);
}


//! Removes a pointer to an other connected port from a port
//! @param [in] pPort The pointer to the other port to be removed
void Port::eraseConnectedPort(Port* pPort)
{
    vector<Port*>::iterator it;
    bool found = false;
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
        cout << "Warning: You tried to erase port ptr that did not exist in the connected ports list" << endl;
    }
}


//! Get a vector of pointers to all other ports connected connected to this one
//! @returns A refernce to the internal vector of connected port pointers
//! @todo maybe should return const vector so that contents my not be changed
vector<Port*> &Port::getConnectedPorts()
{
    return mConnectedPorts;
}


//! Calls the save log data function of the connected node (if any)
void Port::saveLogData(string filename)
{
    if (mpNode != 0)
    {
        mpNode->saveLogData(filename);
    }
    else
    {
        cout << mpComponent->getName() << "-port:" << mPortName << " can not log data, the Port has no Node connected" << endl;
        assert(false);
    }
}


//! Get all data names and units from the connected node
//! @param [in,out] rNames This vector will contain the names
//! @param [in,out] rUnits This vector will contain the units
void Port::getNodeDataNamesAndUnits(vector<string> &rNames, vector<string> &rUnits)
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
void Port::getNodeDataNameAndUnit(const size_t dataid, string &rName, string &rUnit)
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
int Port::getNodeDataIdFromName(const string name)
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


vector<double> *Port::getTimeVectorPtr()
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


vector<vector<double> > *Port::getDataVectorPtr()
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


void Port::getStartValueDataNamesValuesAndUnits(vector<string> &rNames, std::vector<double> &rValues, vector<string> &rUnits)
{
    if(mpStartNode)
        mpStartNode->getDataNamesValuesAndUnits(rNames, rValues, rUnits);
}


void Port::setStartValueDataByNames(vector<string> names, std::vector<double> values)
{
    if(mpStartNode)
        mpStartNode->setDataValuesByNames(names, values);
}

//! Check if the port is curently connected
bool Port::isConnected()
{
    return mIsConnected;
}


//! Check if the port MUST be connected
bool Port::isConnectionRequired()
{
    return mConnectionRequired;
}


//! Get the port type
Port::PORTTYPE Port::getPortType()
{
    return mPortType;
}


//! @brief Get the port type as a string
//! @todo this can probably be made some other better way, mayb let port type lie ooutside port class
string Port::getPortTypeString()
{
    switch (mPortType)
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
    default :
        return "UNDEFINEDPORT";
    }
}


//! Get the port name
const string &Port::getPortName()
{
    return mPortName;
}


//! Get the name of the commponent that the port is attached to
const string &Port::getComponentName()
{
    return mpComponent->getName();
}


////! SystemPort constructor
//SystemPort::SystemPort() : Port()
//{
//    mPortType = SYSTEMPORT;
//}


//! SystemPort constructor
SystemPort::SystemPort(std::string node_type, std::string portname, Component *portOwner) : Port(node_type, portname, portOwner)
{
    mPortType = SYSTEMPORT;
}


////! PowerPort constructor
//PowerPort::PowerPort() : Port()
//{
//    mPortType = POWERPORT;
//}


//! PowerPort constructor
PowerPort::PowerPort(std::string node_type, std::string portname, Component *portOwner) : Port(node_type, portname, portOwner)
{
    mPortType = POWERPORT;
    if(mpComponent->isComponentC())
        mpStartNode = gCoreNodeFactory.createInstance(mNodeType);
}


////Constructor
//ReadPort::ReadPort() : Port()
//{
//    mPortType = READPORT;
//}


ReadPort::ReadPort(std::string node_type, std::string portname, Component *portOwner) : Port(node_type, portname, portOwner)
{
    mPortType = READPORT;
}


void ReadPort::writeNode(const size_t idx, const double value)
{
    cout << "Could not write to port, this is a ReadPort" << endl;
    assert(false);
}


////Constructor
//WritePort::WritePort() : Port()
//{
//    mPortType = WRITEPORT;
//}


WritePort::WritePort(std::string node_type, std::string portname, Component *portOwner) : Port(node_type, portname, portOwner)
{
    mPortType = WRITEPORT;
    mpStartNode = gCoreNodeFactory.createInstance(mNodeType);
}


double WritePort::readNode(const size_t idx)
{
    cout << "Could not read from port, this is a WritePort" << endl;
    assert(false);
    return 0;
}


//!
//! @brief Very simple port factory, no need to complicate things with the more advanced one as we will only have a few fixed port types.
//!
Port* hopsan::CreatePort(Port::PORTTYPE type, NodeTypeT nodetype, string name, Component *portOwner)
{
    switch (type)
    {
    case Port::POWERPORT :
        return new PowerPort(nodetype, name, portOwner);
        break;
    case Port::WRITEPORT :
        return new WritePort(nodetype, name, portOwner);
        break;
    case Port::READPORT :
        return new ReadPort(nodetype, name, portOwner);
        break;
    case Port::SYSTEMPORT :
        return new SystemPort(nodetype, name, portOwner);
        break;
    default :
       //! @todo maybe defualt should be impossible
       return new Port(nodetype, name, portOwner);
    }
}
