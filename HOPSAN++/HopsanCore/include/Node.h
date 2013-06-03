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
class NodeDataDescription
{
public:
    HString name;
    HString shortname;
    HString unit;
    HString description;
    NodeDataVariableTypeEnumT varType;
    unsigned int id;
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
    const HString &getNiceName() const;
    const HString &getNodeType() const;

    size_t getNumDataVariables() const;
    virtual int getDataIdFromName(const HString &rName) const;
    //! @brief get data from node
    //! @param [in] dataId Identifier for the type of node data to get, (no bounds check is performed)
    //! @return The data value
    inline double getDataValue(const size_t dataId) const
    {
        return mDataValues[dataId];
    }
    //! @brief set data in node
    //! @param [in] dataId Identifier for the typ of node data to set, (no bounds check is performed)
    //! @param [in] data The data value
    inline void setDataValue(const size_t dataId, const double data)
    {
        mDataValues[dataId] = data;
    }

    const std::vector<NodeDataDescription>* getDataDescriptions() const;
    const NodeDataDescription* getDataDescription(const size_t id) const;
    virtual void setSignalDataUnitAndDescription(const HString &rUnit, const HString &rDescription);

    void logData(const size_t logSlot);

    int getNumberOfPortsByType(const int type) const;
    size_t getNumConnectedPorts() const;
    bool isConnectedToPort(const Port *pPort) const;

    Component *getWritePortComponentPtr() const;
    ComponentSystem *getOwnerSystem() const;

protected:
    // Protected member functions
    void setNiceName(const HString nicename);
    void setDataCharacteristics(const size_t id, const HString name, const HString shortname,
                                const HString unit, const NodeDataVariableTypeEnumT vartype=DefaultType);

    void copyNodeDataValuesTo(Node *pOtherNode) const;
    virtual void copySignalDataUnitAndDescriptionTo(Node *pOtherNode) const;
    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const;

    void preAllocateLogSpace(const size_t nLogSlots);

    double *getDataPtr(const size_t data_type);

    // Protected member variables
    HString mNiceName;
    std::vector<NodeDataDescription> mDataDescriptions;
    std::vector<double> mDataValues;

private:
    // Private member fuctions
    void addConnectedPort(Port *pPort);
    void removeConnectedPort(const Port *pPort);
    void enableLog();
    void disableLog();

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
