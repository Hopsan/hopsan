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

//typedef string PortTypeT;

//Forward declarations
class Component;
class ComponentSystem;

class DLLIMPORTEXPORT Port
{
    friend class Component;
    friend class ComponentSystem;

public:
    enum PORTTYPE {POWERPORT, READPORT, WRITEPORT, SYSTEMPORT, UNDEFINEDPORT};

    //Constructors - Destructors
    Port();
    Port(string portname, string node_type);
    virtual ~Port();

    virtual double readNode(const size_t idx);
    virtual void writeNode(const size_t idx, const double value);

    void saveLogData(string filename);
    void getNodeDataNamesAndUnits(vector<string> &rNames, vector<string> &rUnits);
    void getNodeDataNameAndUnit(const size_t dataid, string &rName, string &rUnit);
    vector<double> *getTimeVectorPtr();
    vector<vector<double> > *getDataVectorPtr();

    bool isConnected();

    const string &getNodeType();
    //const string &getPortType();
    PORTTYPE getPortType();
    const string &getPortName();

    Node* getNodePublic();

protected:

    //PortTypeT mPortType;
    PORTTYPE mPortType;

    void setNode(Node* pNode);
    Node &getNode();
    Node *getNodePtr();

private:
    string mPortName;
    NodeTypeT mNodeType;
    Node* mpNode;
    Component* mpComponent;
    vector<Port*> mConnectedPorts;
    bool mIsConnected;

    void addConnectedPort(Port* pPort);
    void eraseConnectedPort(Port* pPort);
    vector<Port*> &getConnectedPorts();
    void clearConnection();
};


class SystemPort :public Port
{
    friend class Component;
    friend class ComponentSystem;
    //friend Port* CreatePort(const string &rPortType);

public:
    //Constructors
    SystemPort();
};


class PowerPort :public Port
{
    friend class Component;
    friend class ComponentSystem;
    //friend Port* CreatePort(const string &rPortType);

public:
    //Constructors
    PowerPort();
    PowerPort(string portname, string node_type);
};


class ReadPort :public Port
{
    friend class Component;
    friend class ComponentSystem;
    //friend Port* CreatePort(const string &rPortType);

public:
    //Constructors
    ReadPort();
    ReadPort(string portname, string node_type);

    void writeNode(const size_t idx, const double value);
};


class WritePort :public Port
{
    friend class Component;
    friend class ComponentSystem;
    //friend Port* CreatePort(const string &rPortType);

public:
    //Constructors
    WritePort();
    WritePort(string portname, string node_type);

    double readNode(const size_t idx);
};

//Port* CreatePort(const string &rPortType);
Port* CreatePort(Port::PORTTYPE type);

#endif // PORT_H_INCLUDED
