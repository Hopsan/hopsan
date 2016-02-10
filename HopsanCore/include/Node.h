/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

DLLIMPORTEXPORT HString nodeDataVariableTypeAsString(const NodeDataVariableTypeEnumT type);

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

class DLLIMPORTEXPORT Node
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

    void setForceDisableLog(bool value);
    bool getForceDisableLog() const;

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

    void setLoggingEnabled(bool enable=true);

    // Private member variables
    HString mNodeType;
    std::vector<Port*> mConnectedPorts;
    ComponentSystem *mpOwnerSystem;

    // Log specific variables
    std::vector<std::vector<double> > mDataStorage;
    bool mDoLog;
    bool mForceDisableLog;
};

typedef ClassFactory<HString, Node> NodeFactory;
}

#endif // NODE_H_INCLUDED
