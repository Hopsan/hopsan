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

typedef std::string NodeTypeT;
enum NodeDataVariableTypeT {Default, TLM, Hidden};

class NodeDataDescription
{
public:
    std::string name;
    std::string unit;
    NodeDataVariableTypeT varType;
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
    const NodeTypeT getNodeType() const;
    size_t getNumDataVariables() const;

    virtual int getDataIdFromName(const std::string name);
    double getDataValue(const size_t data_type) const;
    void setDataValue(const size_t data_type, const double data);
    Component *getWritePortComponentPtr();

    const std::vector<NodeDataDescription>* getDataDescriptions() const;
    const NodeDataDescription* getDataDescription(const size_t id) const;

    virtual void setSignalDataUnit(const std::string unit);
    virtual void setSignalDataName(const std::string name);

    //void logData(const double time);
    void logData(const size_t logSlot);

    ComponentSystem *getOwnerSystem();

protected:
    //Protected member functions
    Node(const size_t datalength);
    void setDataCharacteristics(const size_t id, const std::string name, const std::string unit, const NodeDataVariableTypeT vartype=Default);

    void copyNodeDataValuesTo(Node *pNode);
    virtual void setSpecialStartValues(Node *pNode);

    bool preAllocateLogSpace(const size_t nLogSlots);

    double *getDataPtr(const size_t data_type);

    //bool setDataValuesByNames(std::vector<std::string> names, std::vector<double> values);
    int getNumberOfPortsByType(int type);

    //Protected member variables
    std::vector<NodeDataDescription> mDataDescriptions;
    std::vector<double> mDataValues;

private:
    //Private member fuctions
    void addConnectedPort(Port *pPort);
    void removeConnectedPort(Port *pPort);
    bool isConnectedToPort(Port *pPort);
    void enableLog();
    void disableLog();

    //Private member variables
    NodeTypeT mNodeType;
    std::vector<Port*> mConnectedPorts;
    ComponentSystem *mpOwnerSystem;

    //Log specific variables
    std::vector<std::vector<double> > mDataStorage;
    bool mDoLog;
};

typedef ClassFactory<NodeTypeT, Node> NodeFactory;
}

#endif // NODE_H_INCLUDED
