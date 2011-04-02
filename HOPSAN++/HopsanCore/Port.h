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
    class ConnectionAssistant;
    class MultiPort;

    class DLLIMPORTEXPORT Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;
        friend class MultiPort;

    public:
        enum PORTTYPE {MULTIPORT, POWERPORT, READPORT, WRITEPORT, SYSTEMPORT, UNDEFINEDPORT};
        enum CONREQ {REQUIRED, NOTREQUIRED};

        //Constructors - Destructors
        Port(std::string node_type, std::string portname, Component *portOwner);
        virtual ~Port();

        virtual double readNode(const size_t idx, const size_t portIdx=0);
        virtual void writeNode(const size_t &idx, const double &value, const size_t portIdx=0);

        virtual double *getNodeDataPtr(const size_t idx, const size_t portIdx=0);

        virtual void saveLogData(std::string filename, const size_t portIdx=0);
        virtual void getNodeDataNamesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rUnits, const size_t portIdx=0);
        virtual void getNodeDataNameAndUnit(const size_t dataid, std::string &rName, std::string &rUnit, const size_t portIdx=0);
        virtual int getNodeDataIdFromName(const std::string name, const size_t portIdx=0);
        virtual std::vector<double> *getTimeVectorPtr(const size_t portIdx=0);
        virtual std::vector<std::vector<double> > *getDataVectorPtr(const size_t portIdx=0);

        virtual void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<double> &rValues, std::vector<std::string> &rUnits, const size_t portIdx=0);
        virtual void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rValuesTxt, std::vector<std::string> &rUnits, const size_t portIdx=0);
        virtual bool setStartValueDataByNames(std::vector<std::string> names, std::vector<double> values, const size_t portIdx=0);
        virtual bool setStartValueDataByNames(std::vector<std::string> names, std::vector<std::string> sysParNames, const size_t portIdx=0);

        virtual double getStartValue(const size_t idx, const size_t portIdx=0);
        virtual void setStartValue(const size_t &idx, const double &value, const size_t portIdx=0);

        virtual bool isConnected();
        bool isConnectionRequired();

        virtual size_t getNumPorts();

        const std::string &getNodeType();
        PORTTYPE getPortType();
        std::string getPortTypeString();
        const std::string &getPortName();
        const std::string &getComponentName();

        void loadStartValues();
        void loadStartValuesFromSimulation();

        Component* getComponent();

        Port* mpParentPort; ///////////////////////TEMPORARILY PUBLIC (To be able to set it in addSubPort())
    protected:

        PORTTYPE mPortType;
        NodeTypeT mNodeType;

        Node* mpStartNode;
        Component* mpComponent;

        void setNode(Node* pNode, const size_t portIdx=0);
        Node *getNodePtr(const size_t portIdx=0);

        virtual Port* addSubPort();
        virtual void removeSubPort(Port* ptr);


    private:
        std::string mPortName;
        Node* mpNode;
        std::vector<Port*> mConnectedPorts;
        bool mConnectionRequired;
        bool mIsConnected;

        void addConnectedPort(Port* pPort, const size_t portIdx=0);
        void eraseConnectedPort(Port* pPort, const size_t portIdx=0);
        std::vector<Port*> &getConnectedPorts(const size_t portIdx=0);
    };


    class SystemPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructors
        SystemPort(std::string node_type, std::string portname, Component *portOwner);
    };


    class MultiPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructors
        MultiPort(std::string node_type, std::string portname, Component *portOwner);

        //Overloaded virtual functions
        double readNode(const size_t idx, const size_t portIdx);
        void writeNode(const size_t &idx, const double &value, const size_t portIdx);

        double *getNodeDataPtr(const size_t idx, const size_t portIdx);

        void saveLogData(std::string filename, const size_t portIdx=0);
        void getNodeDataNamesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rUnits, const size_t portIdx=0);
        void getNodeDataNameAndUnit(const size_t dataid, std::string &rName, std::string &rUnit, const size_t portIdx=0);
        int getNodeDataIdFromName(const std::string name, const size_t portIdx=0);
        std::vector<double> *getTimeVectorPtr(const size_t portIdx=0);
        std::vector<std::vector<double> > *getDataVectorPtr(const size_t portIdx=0);

//        void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<double> &rValues, std::vector<std::string> &rUnits, const size_t portIdx=0);
//        void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rValuesTxt, std::vector<std::string> &rUnits, const size_t portIdx=0);
//        bool setStartValueDataByNames(std::vector<std::string> names, std::vector<double> values, const size_t portIdx=0);
//        bool setStartValueDataByNames(std::vector<std::string> names, std::vector<std::string> sysParNames, const size_t portIdx=0);

//        double getStartValue(const size_t idx, const size_t portIdx=0);
//        void setStartValue(const size_t &idx, const double &value, const size_t portIdx=0);

        bool isConnected();
        size_t getNumPorts();

    protected:
        Port* addSubPort();
        void removeSubPort(Port* ptr);
        Node *getNodePtr(const size_t portIdx=0);

    private:
        std::vector<Port*> mSubPortsVector;
    };


    class PowerPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructor
        PowerPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);
    };


    class ReadPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructor
        ReadPort(std::string node_type, std::string portname, Component *portOwner);

        void writeNode(const size_t idx, const double value);
    };


    class WritePort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructor
        WritePort(std::string node_type, std::string portname, Component *portOwner);

        double readNode(const size_t idx);
    };

    Port* CreatePort(Port::PORTTYPE type, NodeTypeT nodetype, std::string name, Component *portOwner);
}

#endif // PORT_H_INCLUDED
