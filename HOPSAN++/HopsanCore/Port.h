//!
//! @file   Port.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains Port base classes as well as Sub classes
//!
//$Id$

#ifndef PORT_H_INCLUDED
#define PORT_H_INCLUDED

#include "Node.h"
#include "win32dll.h"
#include <string>

using namespace std;

class Component; //forward declaration
class ComponentSystem;  //forward declaration

class DLLIMPORTEXPORT Port ///TODO: Should be made virtual somehow OR Change Port into PowerPort
{
    friend class Component;
    friend class ComponentSystem;

public:
    Port();
    Port(string portname, string node_type);
    string &getNodeType(); ///TODO: Move to protected
    Node &getNode(); ///TODO: Move to protected
    Node *getNodePtr(); ///TODO: Move to protected
    virtual double ReadNode(const size_t idx);
    virtual void WriteNode(const size_t idx, const double value);
    bool isConnected();

    string &getPortType();
    string &getPortName();

protected:
    string mPortType;
    void setNode(Node* node_ptr);

private:
    string mPortName, mNodeType;
    Node* mpNode;
    Component* mpComponent;
    bool mIsConnected;
};

class DLLIMPORTEXPORT PowerPort :public Port
{
public:
    PowerPort();
    PowerPort(string portname, string node_type);
};


class DLLIMPORTEXPORT ReadPort :public Port
{
public:
    ReadPort();
    ReadPort(string portname, string node_type);

    void WriteNode(const size_t idx, const double value);
};


class DLLIMPORTEXPORT WritePort :public Port
{
public:
    WritePort();
    WritePort(string portname, string node_type);

    double ReadNode(const size_t idx);
};

#endif // PORT_H_INCLUDED
