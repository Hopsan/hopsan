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


//Constructor
Port::Port()
{
    //mPortType = "EmptyPort";
    mPortType = UNDEFINEDPORT;
    mpComponent = 0;
    clearConnection();

}

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

void Port::clearConnection()
{
    //! @todo maybe should be virtual so that we may also clear node type in system ports
    mpNode = 0;
    mConnectedPorts.clear();
    mIsConnected = false;
}

const string &Port::getNodeType()
{
    return mNodeType;
}

Node &Port::getNode()
{
    //! @todo error handle if 0
    return *mpNode;
}

Node* Port::getNodePublic()
{
    return mpNode;
}

Node* Port::getNodePtr()
{
    return mpNode;
}

double Port::readNode(const size_t idx)
{
    return mpNode->getData(idx);
}

void Port::writeNode(const size_t idx, const double value)
{
    return mpNode->setData(idx, value);
}

void Port::setNode(Node* pNode)
{
    mpNode = pNode;
    mIsConnected = true;
}

void Port::addConnectedPort(Port* pPort)
{
    mConnectedPorts.push_back(pPort);
}

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

vector<Port*> &Port::getConnectedPorts()
{
    return mConnectedPorts;
}

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


vector<double> *Port::getTimeVectorPtr()
{
    return &(getNode().mTimeStorage);
}


vector<vector<double> > *Port::getDataVectorPtr()
{
    return &(getNode().mDataStorage);
}


bool Port::isConnected()
{
    return mIsConnected;
}

//const string &Port::getPortType()
//{
//    return mPortType;
//}

Port::PORTTYPE Port::getPortType()
{
    return mPortType;
}

const string &Port::getPortName()
{
    return mPortName;
}

SystemPort::SystemPort() : Port()
{
    //mPortType = "SystemPort";
    mPortType = SYSTEMPORT;
}

//Constructor
PowerPort::PowerPort() : Port()
{
    //mPortType = "PowerPort";
    mPortType = POWERPORT;
}

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
//! @brief Very simple port factory, no need to complicate things with the more advanced one as we will only have four port types.
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
