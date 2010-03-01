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
    mPortType = "EmptyPort";
    mpComponent = 0;
    clearNode();

}

Port::Port(string portname, string node_type)
{
    mPortType = "EmptyPort";
    mPortName = portname;
    mNodeType = node_type;
    mpComponent = 0;
    clearNode();
}

//Destructor
Port::~Port()
{
    //Nothing for now
}

void Port::clearNode()
{
    //! @todo maybe should be virtual so that we may also clear node type in system ports
    mpNode = 0;
    mIsConnected = false;
}

const string &Port::getNodeType()
{
    return mNodeType;
}

Node &Port::getNode()
{
    ///TODO: error handle if 0
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

const string &Port::getPortType()
{
    return mPortType;
}

const string &Port::getPortName()
{
    return mPortName;
}

SystemPort::SystemPort() : Port()
{
    mPortType = "SystemPort";
}

//Constructor
PowerPort::PowerPort() : Port()
{
    mPortType = "PowerPort";
}

PowerPort::PowerPort(string portname, string node_type) : Port(portname, node_type)
{
    mPortType = "PowerPort";
}

//Constructor
ReadPort::ReadPort() : Port()
{
    mPortType = "ReadPort";
}

ReadPort::ReadPort(string portname, string node_type) : Port(portname, node_type)
{
    mPortType = "ReadPort";
}

void ReadPort::writeNode(const size_t idx, const double value)
{
    cout << "Could not write to port, this is a ReadPort" << endl;
    assert(false);
}

//Constructor
WritePort::WritePort() : Port()
{
    mPortType = "WritePort";
}

WritePort::WritePort(string portname, string node_type) : Port(portname, node_type)
{
    mPortType = "WritePort";
}

double WritePort::readNode(const size_t idx)
{
    cout << "Could not read from port, this is a WritePort" << endl;
    assert(false);
}

//!
//! @brief Very simple port factory, no need to complicate things with the more advanced one as we will only have four port types.
//!
Port* CreatePort(const string &rPortType)
{
    ///TODO: maybe swap PortType to enums instead of strings (not really important)
    if (rPortType.c_str() == string("PowerPort"))
    {
        return new PowerPort();
    }
    else if (rPortType.c_str() == string("ReadPort"))
    {
        return new ReadPort();
    }
    else if (rPortType.c_str() == string("WritePort"))
    {
        return new WritePort();
    }
    else if (rPortType.c_str() == string("SystemPort"))
    {
        return new SystemPort();
    }
    else
    {
        ///TODO: maybe defualt should be impossible
        return new Port();
    }
}
