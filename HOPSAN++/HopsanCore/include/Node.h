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

typedef std::string NodeTypeT;

//Forward Declarations
class Port;
class Component;
class ComponentSystem;
class ConnectionAssistant;
class HopsanEssentials;

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

    void logData(const double time);

protected:
    //Protected member functions
    Node(const size_t datalength);
    void setDataCharacteristics(const size_t id, const std::string name, const std::string unit, const NodeDataVariableTypeT vartype=Default);

    void copyNodeDataValuesTo(Node *pNode);
    virtual void setSpecialStartValues(Node *pNode);

    void setLogSettingsNSamples(int nSamples, double start, double stop, double sampletime);
    void setLogSettingsSkipFactor(double factor, double start, double stop, double sampletime);
    void setLogSettingsSampleTime(double log_dt, double start, double stop, double sampletime);
    bool preAllocateLogSpace();
    void saveLogDataToFile(const std::string filename, const std::string header);

    double *getDataPtr(const size_t data_type);

    //bool setDataValuesByNames(std::vector<std::string> names, std::vector<double> values);
    int getNumberOfPortsByType(int type);

    ComponentSystem *getOwnerSystem();

    //Protected member variables
    std::vector<Port*> mPortPtrs;

    std::vector<NodeDataDescription> mDataDescriptions;
    std::vector<double> mDataValues;
    ComponentSystem *mpOwnerSystem;

private:
    //Private member fuctions
    void setPort(Port *pPort);
    void removePort(Port *pPort);
    bool isConnectedToPort(Port *pPort);
    void enableLog();
    void disableLog();

    //Private member variables
    NodeTypeT mNodeType;

    //Log specific variables
    std::vector<double> mTimeStorage;
    std::vector<std::vector<double> > mDataStorage;
    bool mLogSpaceAllocated;
    bool mDoLog;
    double mLogTimeDt;
    double mLastLogTime;
    size_t mLogSlots;
    size_t mLogCtr;
};

typedef ClassFactory<NodeTypeT, Node> NodeFactory;
}

#endif // NODE_H_INCLUDED
