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

//! @file   Node.h
//! @author FluMeS
//! @date   2009-12-20
//! @brief Contains Node base classes
//!
//$Id$

#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <vector>
#include "HopsanTypes.h"
#include "CoreUtilities/ClassFactory.hpp"
#include "win32dll.h"

namespace hopsan {

// Forward Declarations
class Port;
class Component;
class ComponentSystem;
class ConnectionAssistant;
class HopsanEssentials;

enum NodeDataVariableTypeEnumT {DefaultType, IntensityType, FlowType, TLMType, HiddenType};

HOPSANCORE_DLLAPI HString nodeDataVariableTypeAsString(const NodeDataVariableTypeEnumT type);

class NodeDataDescription
{
public:
    HString name;
    HString shortname;
    HString quantity;
    HString unit;
    bool userModifiableQuantity;
    NodeDataVariableTypeEnumT varType;
    size_t id;
};

class HOPSANCORE_DLLAPI Node
{
    friend class Port;
    friend class MultiPort;
    friend class PowerPort;
    friend class WritePort;
    friend class Component;
    friend class ComponentSystem;
    friend class ConnectionAssistant;
    friend class HopsanEssentials;

public:
    Node(const size_t datalength);
    virtual ~Node();

    virtual void setSignalNumDimensions(size_t numDims);

    const HString &getNiceName() const;
    const HString &getNodeType() const;

    size_t getNumDataVariables() const;
    int getDataIdFromName(const HString &rName) const;
    //! @brief get data from node
    //! @param [in] dataId Identifier for the type of node data to get, (no bounds check is performed)
    //! @return The data value
    inline double getDataValue(const size_t dataId) const
    {
        return mDataValues[dataId];
    }
    //! @brief set data in node
    //! @param [in] dataId Identifier for the type of node data to set, (no bounds check is performed)
    //! @param [in] data The data value
    inline void setDataValue(const size_t dataId, const double data)
    {
        mDataValues[dataId] = data;
    }

    const std::vector<NodeDataDescription>* getDataDescriptions() const;
    const NodeDataDescription* getDataDescription(const size_t id) const;

    virtual void setSignalQuantity(const HString &rQuantity, const HString &rUnit, const size_t dataId=0);
    virtual void setSignalQuantityModifyable(bool tf, const size_t dataId=0);
    virtual HString getSignalQuantity(const size_t dataId=0) const;
    virtual bool getSignalQuantityModifyable(const size_t dataId=0) const;

    void logData(const size_t logSlot);

    int getNumberOfPortsByType(const int type) const;
    size_t getNumConnectedPorts() const;
    bool isConnectedToPort(const Port *pPort) const;

    Port *getSortOrderSourcePort() const;
    Component *getWritePortComponentPtr() const;
    ComponentSystem *getOwnerSystem() const;

protected:
    // Protected member functions
    void setNiceName(const HString &rNicename);
    void setDataCharacteristics(const size_t id, const HString &rName, const HString &rShortname,
                                const HString &rQuantityOrUnit, const NodeDataVariableTypeEnumT vartype=DefaultType);

    void copyNodeDataValuesTo(Node *pOtherNode) const;
    virtual void copySignalQuantityAndUnitTo(Node *pOtherNode) const;
    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const;

    void preAllocateLogSpace(const size_t nLogSlots);

    double *getDataPtr(const size_t data_type);

    // Protected member variables
    HString mNiceName;
    std::vector<NodeDataDescription> mDataDescriptions;
    std::vector<double> mDataValues;

private:
    // Private member functions
    void addConnectedPort(Port *pPort);
    void removeConnectedPort(const Port *pPort);

    void setDoLogIfEnabled(bool doLog=true);

    // Private member variables
    HString mNodeType;
    std::vector<Port*> mConnectedPorts;
    ComponentSystem *mpOwnerSystem;

    // Log specific variables
    std::vector<std::vector<double> > mDataStorage;
    bool mDoLog;
};

typedef ClassFactory<HString, Node> NodeFactory;
}

#endif // NODE_H_INCLUDED
