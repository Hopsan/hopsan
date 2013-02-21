/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

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

    //! @brief This enum type specifies all porttypes
    //It is VERY important the MultiPort enums commes LAST, MULTIPORT is never instantiated but enum MUST be present
    enum PortTypesEnumT {UNDEFINEDPORT, POWERPORT, READPORT, WRITEPORT, SYSTEMPORT, MULTIPORT, POWERMULTIPORT, READMULTIPORT};

    class DLLIMPORTEXPORT Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;
        friend class AliasHandler;
        friend class MultiPort;

    public:
        //! @brief This enum specifies the RequiredConnection enums
        enum ReqConnEnumT {REQUIRED, NOTREQUIRED};

        //Constructors - Destructors
        Port(const std::string nodeType, const std::string portName, Component *pPortOwner, Port *pParentPort=0);
        virtual ~Port();

        virtual inline double readNode(const size_t idx, const size_t portIdx=0) const;
        virtual inline void writeNode(const size_t &idx, const double &value, const size_t portIdx=0) const;

        virtual double readNodeSafe(const size_t idx, const size_t portIdx=0);
        virtual void writeNodeSafe(const size_t &idx, const double &value, const size_t portIdx=0);

        virtual double *getNodeDataPtr(const size_t idx, const size_t portIdx=0) const;
        virtual double *getSafeNodeDataPtr(const size_t idx, const double defaultValue, const size_t portIdx=0);
        virtual std::vector<double> *getDataVectorPtr(const size_t portIdx=0);

        virtual const std::vector<NodeDataDescription>* getNodeDataDescriptions(const size_t portIdx=0);
        virtual const NodeDataDescription* getNodeDataDescription(const size_t dataid, const size_t portIdx=0);
        virtual int getNodeDataIdFromName(const std::string name, const size_t portIdx=0);

        virtual void saveLogData(std::string filename, const size_t portIdx=0);
        virtual bool haveLogData(const size_t portIdx=0);
        virtual std::vector<double> *getLogTimeVectorPtr(const size_t portIdx=0);
        virtual std::vector<std::vector<double> > *getLogDataVectorPtr(const size_t portIdx=0);


        //virtual void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<double> &rValues, std::vector<std::string> &rUnits, const size_t portIdx=0);
        //virtual void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rValuesTxt, std::vector<std::string> &rUnits, const size_t portIdx=0);

        virtual double getStartValue(const size_t idx, const size_t portIdx=0);
        virtual void setStartValue(const size_t idx, const double value, const size_t portIdx=0);
        virtual void disableStartValue(const size_t idx);

        virtual bool isConnected();
        virtual bool isConnectedTo(Port *pOtherPort);
        bool isConnectionRequired();

        virtual size_t getNumPorts();

        bool isMultiPort() const;
        Port *getParentPort() const;
        const std::string getNodeType() const;
        PortTypesEnumT getPortType() const;
        virtual PortTypesEnumT getExternalPortType();
        virtual PortTypesEnumT getInternalPortType();

        const std::string getPortName() const;
        const std::string getComponentName() const;

        virtual void loadStartValues();
        virtual void loadStartValuesFromSimulation();

        Component* getComponent() const;

        std::string getVariableAlias(const int id) const;
        int getVariableIdByAlias(const std::string alias) const;
        virtual std::vector<Port*> &getConnectedPorts(const int portIdx=-1);


    protected:
        PortTypesEnumT mPortType;
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

        void createStartNode(NodeTypeT nodeType);

        void setVariableAlias(const std::string alias, const int id);

    private:
        std::string mPortName;
        Node* mpNode;
        Node* mpNCDummyNode; //NotConnected dummy node
        std::map<std::string, int> mVariableAliasMap;

        bool mConnectionRequired;

    };


    class SystemPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        //Constructors
        SystemPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort=0);
        PortTypesEnumT getExternalPortType();
        PortTypesEnumT getInternalPortType();
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
        double readNodeSafe(const size_t idx, const size_t portIdx);
        void writeNodeSafe(const size_t &idx, const double &value, const size_t portIdx);
        inline double readNode(const size_t idx, const size_t portIdx) const;
        inline void writeNode(const size_t &idx, const double &value, const size_t portIdx) const;

        double *getNodeDataPtr(const size_t idx, const size_t portIdx);
        double *getSafeNodeDataPtr(const size_t idx, const double defaultValue, const size_t portIdx);
        std::vector<double> *getDataVectorPtr(const size_t portIdx=0);

        const std::vector<NodeDataDescription>* getNodeDataDescriptions(const size_t portIdx=0);
        const NodeDataDescription* getNodeDataDescription(const size_t dataid, const size_t portIdx=0);
        int getNodeDataIdFromName(const std::string name, const size_t portIdx=0);

        bool haveLogData(const size_t portIdx=0);
        void saveLogData(std::string filename, const size_t portIdx=0);
        std::vector<double> *getLogTimeVectorPtr(const size_t portIdx=0);
        std::vector<std::vector<double> > *getLogDataVectorPtr(const size_t portIdx=0);


//        void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<double> &rValues, std::vector<std::string> &rUnits, const size_t portIdx=0);
//        void getStartValueDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rValuesTxt, std::vector<std::string> &rUnits, const size_t portIdx=0);
//        bool setStartValueDataByNames(std::vector<std::string> names, std::vector<double> values, const size_t portIdx=0);
//        bool setStartValueDataByNames(std::vector<std::string> names, std::vector<std::string> sysParNames, const size_t portIdx=0);

        double getStartValue(const size_t idx, const size_t portIdx=0);
//        void setStartValue(const size_t &idx, const double &value, const size_t portIdx=0);

        void loadStartValues();
        void loadStartValuesFromSimulation();

        bool isConnected();
        size_t getNumPorts();

        std::vector<Port*> &getConnectedPorts(const int portIdx=-1);

    protected:
        std::vector<Port*> mSubPortsVector;

        void removeSubPort(Port* ptr);
        Node *getNodePtr(const size_t portIdx=0);

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
        void writeNodeSafe(const size_t idx, const double value);
        inline void writeNode(const size_t idx, const double value) const;
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

        double readNodeSafe(const size_t idx);
        inline double readNode(const size_t idx) const;
    };

    Port* createPort(const PortTypesEnumT portType, const NodeTypeT nodeType, const std::string name, Component *pPortOwner, Port *pParentPort=0);
    std::string DLLIMPORTEXPORT portTypeToString(const PortTypesEnumT type);
}

#endif // PORT_H_INCLUDED
