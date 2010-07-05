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

//Forward declarations
class Component;
class ComponentSystem;

class DLLIMPORTEXPORT Port
{
    friend class Component;
    friend class ComponentSystem;

public:
    enum PORTTYPE {POWERPORT, READPORT, WRITEPORT, SYSTEMPORT, UNDEFINEDPORT};
    enum CONREQ {REQUIRED, OPTIONAL};

    //Constructors - Destructors
    Port();
    Port(string portname, string node_type);
    virtual ~Port();

    virtual double readNode(const size_t idx);
    virtual void writeNode(const size_t idx, const double value);

    void saveLogData(string filename);
    void getNodeDataNamesAndUnits(vector<string> &rNames, vector<string> &rUnits);
    void getNodeDataNameAndUnit(const size_t dataid, string &rName, string &rUnit);
    int getNodeDataIdFromName(const string name);
    vector<double> *getTimeVectorPtr();
    vector<vector<double> > *getDataVectorPtr();

    bool isConnected();
    bool isConnectionRequired();

    const string &getNodeType();
    PORTTYPE getPortType();
    string getPortTypeString();
    const string &getPortName();
    const string &getComponentName();

    Node* getNodePublic();

protected:

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
    bool mConnectionRequired;
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

public:
    //Constructors
    SystemPort();
};


class PowerPort :public Port
{
    friend class Component;
    friend class ComponentSystem;

public:
    //Constructors
    PowerPort();
    PowerPort(string portname, string node_type);
};


class ReadPort :public Port
{
    friend class Component;
    friend class ComponentSystem;

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

public:
    //Constructors
    WritePort();
    WritePort(string portname, string node_type);

    double readNode(const size_t idx);
};

Port* CreatePort(Port::PORTTYPE type);

#endif // PORT_H_INCLUDED
