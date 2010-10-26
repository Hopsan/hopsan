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

namespace hopsan {

    //Forward declarations
    class Component;
    class ComponentSystem;

    class DLLIMPORTEXPORT Port
    {
        friend class Component;
        friend class ComponentSystem;

    public:
        enum PORTTYPE {POWERPORT, READPORT, WRITEPORT, SYSTEMPORT, UNDEFINEDPORT};
        enum CONREQ {REQUIRED, NOTREQUIRED};

        //Constructors - Destructors
        Port(std::string node_type, std::string portname, Component *portOwner);
        virtual ~Port();

        virtual double readNode(const size_t idx);
        virtual void writeNode(const size_t &idx, const double &value);

        double *getNodeDataPtr(const size_t idx);

        void saveLogData(std::string filename);
        void getNodeDataNamesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rUnits);
        void getNodeDataNameAndUnit(const size_t dataid, std::string &rName, std::string &rUnit);
        int getNodeDataIdFromName(const std::string name);
        std::vector<double> *getTimeVectorPtr();
        std::vector<std::vector<double> > *getDataVectorPtr();

        void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<double> &rValues, std::vector<std::string> &rUnits);
        void setStartValueDataByNames(std::vector<std::string> names, std::vector<double> values);

        bool isConnected();
        bool isConnectionRequired();

        const std::string &getNodeType();
        PORTTYPE getPortType();
        std::string getPortTypeString();
        const std::string &getPortName();
        const std::string &getComponentName();

        Node* getNodePublic();

        void loadStartValues();
        void loadStartValuesFromSimulation();

    protected:

        PORTTYPE mPortType;
        Component* mpComponent;
        NodeTypeT mNodeType;

        void setNode(Node* pNode);
        Node &getNode();
        Node *getNodePtr();

        Node* mpStartNode;

    private:
        std::string mPortName;
        Node* mpNode;
        std::vector<Port*> mConnectedPorts;
        bool mConnectionRequired;
        bool mIsConnected;

        void addConnectedPort(Port* pPort);
        void eraseConnectedPort(Port* pPort);
        std::vector<Port*> &getConnectedPorts();
        void clearConnection();
    };


    class SystemPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;

    public:
        //Constructors
        //SystemPort();
        SystemPort(std::string node_type, std::string portname, Component *portOwner);
    };


    class PowerPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;

    public:
        //Constructors
        //PowerPort();
        PowerPort(std::string node_type, std::string portname, Component *portOwner);
    };


    class ReadPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;

    public:
        //Constructors
        //ReadPort();
        ReadPort(std::string node_type, std::string portname, Component *portOwner);

        void writeNode(const size_t idx, const double value);
    };


    class WritePort :public Port
    {
        friend class Component;
        friend class ComponentSystem;

    public:
        //Constructors
        //WritePort();
        WritePort(std::string node_type, std::string portname, Component *portOwner);

        double readNode(const size_t idx);
    };

    Port* CreatePort(Port::PORTTYPE type, NodeTypeT nodetype, std::string name, Component *portOwner);
}

#endif // PORT_H_INCLUDED
