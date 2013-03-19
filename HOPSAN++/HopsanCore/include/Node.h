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
#include <string>
#include "CoreUtilities/ClassFactory.hpp"
#include "win32dll.h"

namespace hopsan {

//Forward Declarations
class Port;
class Component;
class ComponentSystem;
class ConnectionAssistant;
class HopsanEssentials;

enum NodeDataVariableTypeEnumT {Default, Intensity, Flow, TLM, Hidden};
class NodeDataDescription
{
public:
    std::string name;
    std::string shortname;
    std::string unit;
    std::string description;
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
    std::string getNiceName() const;
    const std::string getNodeType() const;

    size_t getNumDataVariables() const;
    virtual int getDataIdFromName(const std::string name) const;
    double getDataValue(const size_t data_type) const;
    void setDataValue(const size_t data_type, const double data);

    const std::vector<NodeDataDescription>* getDataDescriptions() const;
    const NodeDataDescription* getDataDescription(const size_t id) const;
    virtual void setSignalDataUnitAndDescription(const std::string &rUnit, const std::string &rName);

    void logData(const size_t logSlot);

    int getNumberOfPortsByType(const int type) const;
    size_t getNumConnectedPorts() const;
    bool isConnectedToPort(const Port *pPort) const;

    Component *getWritePortComponentPtr() const;
    ComponentSystem *getOwnerSystem() const;

protected:
    // Protected member functions
    void setNiceName(const std::string nicename);
    void setDataCharacteristics(const size_t id, const std::string name, const std::string shortname,
                                const std::string unit, const NodeDataVariableTypeEnumT vartype=Default);

    void copyNodeDataValuesTo(Node *pNode);
    virtual void setSpecialStartValues(Node *pNode);

    void preAllocateLogSpace(const size_t nLogSlots);

    double *getDataPtr(const size_t data_type);

    //Protected member variables
    std::string mNiceName;
    std::vector<NodeDataDescription> mDataDescriptions;
    std::vector<double> mDataValues;

private:
    //Private member fuctions
    void addConnectedPort(Port *pPort);
    void removeConnectedPort(Port *pPort);
    void enableLog();
    void disableLog();

    //Private member variables
    std::string mNodeType;
    std::vector<Port*> mConnectedPorts;
    ComponentSystem *mpOwnerSystem;

    //Log specific variables
    std::vector<std::vector<double> > mDataStorage;
    bool mDoLog;
};

typedef ClassFactory<std::string, Node> NodeFactory;
}

#endif // NODE_H_INCLUDED
