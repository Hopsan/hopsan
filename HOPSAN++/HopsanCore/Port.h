//!
//! @file   Port.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains Port base class as well as Sub classes
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

class DLLIMPORTEXPORT Port
{
    friend class Component;
    friend class ComponentSystem;

public:
    string &getNodeType();
    Node &getNode(); ///TODO: Move to protected
    Node *getNodePtr(); ///TODO: Move to protected
    virtual double readNode(const size_t idx);
    virtual void writeNode(const size_t idx, const double value);
    bool isConnected();

    string &getPortType();
    string &getPortName();

protected:
    //Constructors
    Port();
    Port(string portname, string node_type);

    string mPortType;
    void setNode(Node* pNode);

private:
    string mPortName, mNodeType;
    Node* mpNode;
    Component* mpComponent;
    bool mIsConnected;
};

class DLLIMPORTEXPORT PowerPort :public Port
{
    friend class Component;
    friend class ComponentSystem;

protected:
    //Constructors
    PowerPort();
    PowerPort(string portname, string node_type);
};


class DLLIMPORTEXPORT ReadPort :public Port
{
    friend class Component;
    friend class ComponentSystem;

public:
    void writeNode(const size_t idx, const double value);

protected:
    //Constructors
    ReadPort();
    ReadPort(string portname, string node_type);
};


class DLLIMPORTEXPORT WritePort :public Port
{
    friend class Component;
    friend class ComponentSystem;

public:
    double readNode(const size_t idx);

protected:
    //Constructors
    WritePort();
    WritePort(string portname, string node_type);
};

#endif // PORT_H_INCLUDED
