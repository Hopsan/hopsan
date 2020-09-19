/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include "HopsanCoreMacros.h"
#include "HopsanTypes.h"

namespace hopsan {

    // Forward declarations
    class Component;
    class ComponentSystem;
    class ConnectionAssistant;
    class MultiPort;

    //! @brief This enum type specifies all porttypes
    // It is VERY important that the MultiPort enums comes LAST, MULTIPORT is never instantiated but enum MUST be present
    enum PortTypesEnumT {UndefinedPortType, PowerPortType, BiDirectionalSignalPortType, ReadPortType, WritePortType, SystemPortType, MultiportType, PowerMultiportType, ReadMultiportType};
    enum SortHintEnumT {NoSortHint, Source, Destination, IndependentDestination};

    class HOPSANCORE_DLLAPI Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;
        friend class AliasHandler;
        friend class MultiPort;

    public:
        //! @brief This enum specifies the RequiredConnection enums
        enum RequireConnectionEnumT {Required, NotRequired};

        // Constructors - Destructors
        Port(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        virtual ~Port();

        //! @brief Reads a value from the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to read
        //! @return The data value
        inline double readNode(const size_t idx) const
        {
            return mpNode->mDataValues[idx];
        }

        //! @brief Reads a value from the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to read
        //! @param [in] subPortIdx (Ignored on non multi ports)
        //! @return The data value
        virtual inline double readNode(const size_t idx, const size_t subPortIdx) const
        {
            HOPSAN_UNUSED(subPortIdx)
            return mpNode->mDataValues[idx];
        }

        //! @brief Writes a value to the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to write
        //! @param [in] value The value to write
        inline void writeNode(const size_t idx, const double value)
        {
            mpNode->mDataValues[idx] = value;
        }

        //! @brief Writes a value to the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to write
        //! @param [in] value The value to write
        //! @param [in] subPortIdx (Ignored on non multi ports)
        virtual inline void writeNode(const size_t idx, const double value, const size_t subPortIdx)
        {
            HOPSAN_UNUSED(subPortIdx)
            mpNode->mDataValues[idx] = value;
        }

        ///@{
        //! @brief Returns a reference to the Node data in the port
        //! @returns A reference to the node data vector
        inline std::vector<double> &getNodeDataVector()
        {
            return mpNode->mDataValues;
        }

        inline const std::vector<double> &getNodeDataVector() const
        {
            return mpNode->mDataValues;
        }
        ///@}

        ///@{
        //! @brief Returns a reference to the Node data in the port
        //! @param[in] subPortIdx The index of a multiport subport to access
        //! @returns A reference to the node data vector
        virtual inline std::vector<double> &getNodeDataVector(const size_t subPortIdx)
        {
            HOPSAN_UNUSED(subPortIdx);
            return getNodeDataVector();
        }

        virtual inline const std::vector<double> &getNodeDataVector(const size_t subPortIdx) const
        {
            HOPSAN_UNUSED(subPortIdx);
            return getNodeDataVector();
        }
        ///@}

        virtual double readNodeSafe(const size_t idx, const size_t subPortIdx=0) const;
        virtual void writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx=0);

        virtual Node *getNodePtr(const size_t subPortIdx=0);
        virtual const Node *getNodePtr(const size_t subPortIdx=0) const;
        virtual double *getNodeDataPtr(const size_t idx, const size_t subPortIdx=0) const;
        virtual std::vector<double> *getDataVectorPtr(const size_t subPortIdx=0);

        virtual size_t getNumDataVariables() const;
        virtual const std::vector<NodeDataDescription>* getNodeDataDescriptions(const size_t subPortIdx=0) const;
        virtual const NodeDataDescription* getNodeDataDescription(const size_t dataid, const size_t subPortIdx=0) const;
        virtual int getNodeDataIdFromName(const HString &rName, const size_t subPortIdx=0);

        virtual void setSignalNodeQuantityOrUnit(const HString &rQuantityOrUnit);
        virtual void setSignalNodeQuantityModifyable(bool tf);
        virtual HString getSignalNodeQuantity() const;
        virtual bool getSignalNodeQuantityModifyable() const;

        const HString &getVariableAlias(const size_t id) const;
        int getVariableIdByAlias(const HString &rAlias) const;

        virtual bool haveLogData(const size_t subPortIdx=0);
        virtual std::vector<double> *getLogTimeVectorPtr(const size_t subPortIdx=0);
        virtual std::vector<std::vector<double> > *getLogDataVectorPtr(size_t subPortIdx=0);
        virtual const std::vector<std::vector<double> > *getLogDataVectorPtr(size_t subPortIdx=0) const;
        virtual void setEnableLogging(const bool enableLog);
        bool isLoggingEnabled() const;

        virtual bool isConnected() const;
        virtual bool isConnectedTo(Port *pOtherPort);
        bool isConnectionRequired();
        virtual std::vector<Port*> getConnectedPorts(const int subPortIdx=-1) const;
        size_t getNumConnectedPorts(const int subPortIdx=-1);
        virtual size_t getNumPorts();

        bool isInterfacePort() const;
        virtual bool isMultiPort() const;
        const HString &getNodeType() const;
        virtual PortTypesEnumT getPortType() const;
        virtual PortTypesEnumT getExternalPortType();
        virtual PortTypesEnumT getInternalPortType();

        virtual SortHintEnumT getSortHint() const;
        virtual void setSortHint(SortHintEnumT hint);
        virtual SortHintEnumT getInternalSortHint();

        Port *getParentPort() const;
        Component* getComponent() const;

        const HString &getName() const;
        const HString &getComponentName() const;
        const HString &getDescription() const;
        void setDescription(const HString &rDescription);

        virtual double getStartValue(const size_t idx, const size_t subPortIdx=0);
        virtual void loadStartValues();
        virtual void loadStartValuesFromSimulation();

    protected:
        HString mNodeType;
        SortHintEnumT mSortHint;
        Component* mpComponent;
        Port* mpParentPort;
        bool mEnableLogging;

        std::vector<Port*> mConnectedPorts;

        virtual void setDefaultStartValue(const size_t idx, const double value, const size_t subPortIdx=0);
        virtual void disableStartValue(const size_t idx);

        virtual Node *getStartNodePtr();
        virtual void setNode(Node* pNode);

        virtual Port* addSubPort();
        virtual void removeSubPort(Port* pPort);

        void registerStartValueParameters();
        void unRegisterStartValueParameters();

        void addConnectedPort(Port* pPort, const size_t subPortIdx=0);
        void eraseConnectedPort(Port* pPort, const size_t subPortIdx=0);

        void createStartNode(const HString &rNodeType);
        void eraseStartNode();

        void setVariableAlias(const HString &rAlias, const size_t id);

    private:
        HString mPortName;
        HString mDescription;
        Node *mpNode;
        Node *mpStartNode;
        std::map<HString, size_t> mVariableAliasMap;
        bool mConnectionRequired;

        const HString mEmptyString;
    };


    class SystemPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        SystemPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);

        PortTypesEnumT getPortType() const {return SystemPortType;}
        PortTypesEnumT getExternalPortType();
        PortTypesEnumT getInternalPortType();
        virtual SortHintEnumT getInternalSortHint();
    };


    class MultiPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        MultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        ~MultiPort();

        // Overloaded virtual functions
        virtual PortTypesEnumT getPortType() const = 0;
        bool isMultiPort() const;
        double readNodeSafe(const size_t idx, const size_t subPortIdx) const;
        void writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx);

        //! @brief Reads a value from the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to read
        //! @param [in] subPortIdx The subPort to read from (range is NOT checked)
        //! @return The data value
        inline double readNode(const size_t idx, const size_t subPortIdx) const
        {
            return mSubPortsVector[subPortIdx]->readNode(idx);
        }

        //! @brief Writes a value to the connected node
        //! @ingroup ComponentSimulationFunctions
        //! @param [in] idx The data id of the data to write
        //! @param [in] value The value to write
        //! @param [in] subPortIdx The subPort to write to (range is NOT checked)
        inline void writeNode(const size_t idx, const double value, const size_t subPortIdx)
        {
            return mSubPortsVector[subPortIdx]->writeNode(idx,value);
        }

        ///@{
        //! @brief Returns a reference to the Node data in the port
        //! @param[in] subPortIdx The index of a multiport subport to access
        //! @returns A reference to the node data vector
        inline std::vector<double> &getNodeDataVector(const size_t subPortIdx)
        {
            return mSubPortsVector[subPortIdx]->getNodeDataVector();
        }

        inline const std::vector<double> &getNodeDataVector(const size_t subPortIdx) const
        {
            return mSubPortsVector[subPortIdx]->getNodeDataVector();
        }
        ///@}

        const Node *getNodePtr(const size_t subPortIdx=0) const;
        double *getNodeDataPtr(const size_t idx, const size_t subPortIdx) const;
        std::vector<double> *getDataVectorPtr(const size_t subPortIdx=0);

        const std::vector<NodeDataDescription>* getNodeDataDescriptions(const size_t subPortIdx=0) const;
        const NodeDataDescription* getNodeDataDescription(const size_t dataid, const size_t subPortIdx=0) const;
        int getNodeDataIdFromName(const HString &rName, const size_t subPortIdx=0);

        bool haveLogData(const size_t subPortIdx=0);
        std::vector<double> *getLogTimeVectorPtr(const size_t subPortIdx=0);
        std::vector<std::vector<double> > *getLogDataVectorPtr(const size_t subPortIdx=0);
        const std::vector<std::vector<double> > *getLogDataVectorPtr(size_t subPortIdx=0) const;
        virtual void setEnableLogging(const bool enableLog);

        double getStartValue(const size_t idx, const size_t subPortIdx=0);

        void loadStartValues();
        void loadStartValuesFromSimulation();

        virtual bool isConnectedTo(Port *pOtherPort);
        bool isConnected() const;
        size_t getNumPorts();

        std::vector<Port*> getConnectedPorts(const int subPortIdx=-1) const;

    protected:
        void setNode(Node* pNode);
        Node *getNodePtr(const size_t subPortIdx=0);
        void removeSubPort(Port* ptr);

        Port* addSubPort(const hopsan::PortTypesEnumT type);

        std::vector<Port*> mSubPortsVector;
    };


    class PowerPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        PowerPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const {return PowerPortType;}
    };

    class BiDirectionalSignalPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        BiDirectionalSignalPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const {return BiDirectionalSignalPortType;}
    };


    class ReadPort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        ReadPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const {return ReadPortType;}

        void writeNodeSafe(const size_t idx, const double value, const size_t subPortIdx=0);
//        void writeNode(const size_t idx, const double value, const size_t subPortIdx);

//        virtual SortHintEnumT getSortHint() const {return Destination;}
        virtual void setSortHint(SortHintEnumT hint);

        virtual void loadStartValues();
        //virtual bool hasConnectedExternalSystemWritePort();
        virtual bool isConnectedToWriteOrPowerPort();
        virtual void forceLoadStartValue();
    };

    class PowerMultiPort :public MultiPort
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        PowerMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const {return PowerMultiportType;}

    protected:
        Port* addSubPort();
    };

    class ReadMultiPort :public MultiPort
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        ReadMultiPort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const {return ReadMultiportType;}

//        virtual SortHintEnumT getSortHint() const {return Destination;}
//        virtual void setSortHint(SortHintEnumT hint){HOPSAN_UNUSED(hint);}
        virtual void setSortHint(SortHintEnumT hint);

    protected:
        Port* addSubPort();
    };


    class WritePort :public Port
    {
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;

    public:
        WritePort(const HString &rNodeType, const HString &rPortName, Component *pParentComponent, Port *pParentPort=0);
        PortTypesEnumT getPortType() const {return WritePortType;}

        virtual SortHintEnumT getSortHint() const {return Source;}
        virtual void setSortHint(SortHintEnumT hint){HOPSAN_UNUSED(hint);}
    };

    Port* createPort(const PortTypesEnumT portType, const HString &rNodeType, const HString &rName, Component *pParentComponent, Port *pParentPort=0);
    HString HOPSANCORE_DLLAPI portTypeToString(const PortTypesEnumT type);
}

#endif // PORT_H_INCLUDED
