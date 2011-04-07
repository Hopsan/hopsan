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
        //It is VERY important the MultiPort enums commes LAST
        enum PORTTYPE {UNDEFINEDPORT, POWERPORT, READPORT, WRITEPORT, SYSTEMPORT, MULTIPORT, POWERMULTIPORT, READMULTIPORT};
        enum CONREQ {REQUIRED, NOTREQUIRED};

        //Constructors - Destructors
        Port(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);
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
        virtual bool isConnectedTo(Port *pOtherPort);
        bool isConnectionRequired();

        virtual size_t getNumPorts();

        const std::string &getNodeType();
        PORTTYPE getPortType();
        std::string getPortTypeString();
        const std::string &getPortName();
        const std::string &getComponentName();

        virtual void loadStartValues();
        virtual void loadStartValuesFromSimulation();

        Component* getComponent();


    protected:
        PORTTYPE mPortType;
        NodeTypeT mNodeType;

        Node* mpStartNode;
        Component* mpComponent;

        Port* mpParentPort;

        std::vector<Port*> mConnectedPorts;

        void setNode(Node* pNode, const size_t portIdx=0);
        virtual Node *getNodePtr(const size_t portIdx=0);

        virtual Port* addSubPort();
        virtual void removeSubPort(Port* ptr);

        void addConnectedPort(Port* pPort, const size_t portIdx=0);
        void eraseConnectedPort(Port* pPort, const size_t portIdx=0);
        virtual std::vector<Port*> &getConnectedPorts(const int portIdx=-1);

    private:
        std::string mPortName;
        Node* mpNode;

        bool mConnectionRequired;
        bool mIsConnected;


    };


    class SystemPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructors
        SystemPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);
    };


    class MultiPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructors
        MultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);
        ~MultiPort();

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

        void loadStartValues();
        void loadStartValuesFromSimulation();

        bool isConnected();
        size_t getNumPorts();

    protected:
        std::vector<Port*> mSubPortsVector;

        void removeSubPort(Port* ptr);
        Node *getNodePtr(const size_t portIdx=0);

        std::vector<Port*> &getConnectedPorts(const int portIdx=-1);
        std::vector<Port*> mAllConnectedPorts;
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
        ReadPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);
        void writeNode(const size_t idx, const double value);
    };

    class PowerMultiPort :public MultiPort
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructor
        PowerMultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);

    protected:
        Port* addSubPort();

    };

    class ReadMultiPort :public MultiPort
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructor
        ReadMultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);

    protected:
        Port* addSubPort();

    };


    class WritePort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructor
        WritePort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);

        double readNode(const size_t idx);
    };

    Port* CreatePort(Port::PORTTYPE type, NodeTypeT nodetype, std::string name, Component *portOwner, Port *pParentPort=0);
}

#endif // PORT_H_INCLUDED
