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
    mPortType = "PowerPort"; ///TODO: Workaround to get old comp to work, should be "Port"
    mpNode  = 0;
    mpComponent = 0;
    mIsConnected = false;
}

Port::Port(string portname, string node_type)
{
    mPortType = "PowerPort"; ///TODO: Workaround to get old comp to work, should be "Port"
    mPortName = portname;
    mNodeType = node_type;
    mpNode  = 0;
    mpComponent = 0;
    mIsConnected = false;
}

string &Port::getNodeType()
{
    return mNodeType;
}

Node &Port::getNode()
{
    ///TODO: error handle if 0
    return *mpNode;
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

bool Port::isConnected()
{
    return mIsConnected;
}

string &Port::getPortType()
{
    return mPortType;
}

string &Port::getPortName()
{
    return mPortName;
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
