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
#include "Component.h"


//! Port base class constructor
Port::Port()
{
    //mPortType = "EmptyPort";
    mPortType = UNDEFINEDPORT;
    mpComponent = 0;
    clearConnection();

}

//! Port base class constructor
Port::Port(string portname, string node_type)
{
    //mPortType = "EmptyPort";
    mPortType = UNDEFINEDPORT;
    mPortName = portname;
    mNodeType = node_type;
    mpComponent = 0;
    clearConnection();
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

//! Reads a value from the connected node
//! @param [in] idx The data id of the data to read
//! @return The data value
double Port::readNode(const size_t idx)
{
    return mpNode->getData(idx);
}

//! Writes a value to the connected node
//! @param [in] idx The data id of the data to write
//! @param [in] value The value of the data to read
void Port::writeNode(const size_t idx, const double value)
{
    return mpNode->setData(idx, value);
}

//! Set the node that the port is connected to
//! @param [in] pNode A pointer to the Node
void Port::setNode(Node* pNode)
{
    mpNode = pNode;
    mIsConnected = true;
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
    mpNode->getDataNamesAndUnits(rNames, rUnits);
}

//! @brief Get node data name and unit for specific node data
//! @param [in] dataid The node data id
//! @param [in,out] rName This vector will contain the names
//! @param [in,out] rUnit This vector will contain the units
void Port::getNodeDataNameAndUnit(const size_t dataid, string &rName, string &rUnit)
{
    rName = mpNode->getDataName(dataid);
    rUnit = mpNode->getDataUnit(dataid);
}


vector<double> *Port::getTimeVectorPtr()
{
    return &(getNode().mTimeStorage);
}


vector<vector<double> > *Port::getDataVectorPtr()
{
    return &(getNode().mDataStorage);
}

//! Check if the port is curently connected
bool Port::isConnected()
{
    return mIsConnected;
}

//const string &Port::getPortType()
//{
//    return mPortType;
//}

//! Get the port type
Port::PORTTYPE Port::getPortType()
{
    return mPortType;
}

//! Get the port name
const string &Port::getPortName()
{
    return mPortName;
}

//! SystemPort constructor
SystemPort::SystemPort() : Port()
{
    //mPortType = "SystemPort";
    mPortType = SYSTEMPORT;
}

//! PowerPort constructor
PowerPort::PowerPort() : Port()
{
    //mPortType = "PowerPort";
    mPortType = POWERPORT;
}

//! PowerPort constructor
PowerPort::PowerPort(string portname, string node_type) : Port(portname, node_type)
{
    //mPortType = "PowerPort";
    mPortType = POWERPORT;
}

//Constructor
ReadPort::ReadPort() : Port()
{
    //mPortType = "ReadPort";
    mPortType = READPORT;
}

ReadPort::ReadPort(string portname, string node_type) : Port(portname, node_type)
{
    //mPortType = "ReadPort";
    mPortType = READPORT;
}

void ReadPort::writeNode(const size_t idx, const double value)
{
    cout << "Could not write to port, this is a ReadPort" << endl;
    assert(false);
}

//Constructor
WritePort::WritePort() : Port()
{
    //mPortType = "WritePort";
    mPortType = WRITEPORT;
}

WritePort::WritePort(string portname, string node_type) : Port(portname, node_type)
{
    //mPortType = "WritePort";
    mPortType = WRITEPORT;
}

double WritePort::readNode(const size_t idx)
{
    cout << "Could not read from port, this is a WritePort" << endl;
    assert(false);
}

////!
////! @brief Very simple port factory, no need to complicate things with the more advanced one as we will only have four port types.
////!
//Port* CreatePort(const string &rPortType)
//{
//    //! @todo maybe swap PortType to enums instead of strings (not really important)
//    if (rPortType.c_str() == string("PowerPort"))
//    {
//        return new PowerPort();
//    }
//    else if (rPortType.c_str() == string("ReadPort"))
//    {
//        return new ReadPort();
//    }
//    else if (rPortType.c_str() == string("WritePort"))
//    {
//        return new WritePort();
//    }
//    else if (rPortType.c_str() == string("SystemPort"))
//    {
//        return new SystemPort();
//    }
//    else
//    {
//        //! @todo maybe defualt should be impossible
//        return new Port();
//    }
//}

//!
//! @brief Very simple port factory, no need to complicate things with the more advanced one as we will only have a few fixed port types.
//!
Port* CreatePort(Port::PORTTYPE type)
{
    switch (type)
    {
    case Port::POWERPORT :
        return new PowerPort();
        break;
    case Port::WRITEPORT :
        return new WritePort();
        break;
    case Port::READPORT :
        return new ReadPort();
        break;
    case Port::SYSTEMPORT :
        return new SystemPort();
        break;
    default :
       //! @todo maybe defualt should be impossible
       return new Port();
    }
}
