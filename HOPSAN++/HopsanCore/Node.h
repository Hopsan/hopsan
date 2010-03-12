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
#include "CoreUtilities/ClassFactory.h"
#include "win32dll.h"

using namespace std;

typedef string NodeTypeT;

class Port; //forward declaration

class DLLIMPORTEXPORT Node
{
    friend class Port;
    friend class Component;
    friend class ComponentSystem;

public:
    //The user should never bother about Nodes

protected:
    //Protected member functions
    Node();
    NodeTypeT &getNodeType();

    void preAllocateLogSpace(const size_t nSlots);
    void logData(const double time);
    void saveLogData(string filename);

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);
    double &getDataRef(const size_t data_type);

    //Protected member variables
    NodeTypeT mNodeType;
    vector<double> mDataVector;
    vector<Port*> mPortPtrs;

private:
    //Private member fuctions
    void setPort(Port *pPort);
    void removePort(Port *pPort);
    bool isConnectedToPort(Port *pPort);

    //Private member variables
    string mName;
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;
    bool mLogSpaceAllocated;
    size_t mLogCtr;
};

typedef ClassFactory<NodeTypeT, Node> NodeFactory;
extern NodeFactory gCoreNodeFactory;
DLLIMPORTEXPORT NodeFactory* getCoreNodeFactoryPtr();

#endif // NODE_H_INCLUDED
