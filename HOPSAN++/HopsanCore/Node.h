//! @file   Node.h
//! @author <FluMeS>
//! @date   2009-12-20
//!
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
    Node();
    NodeTypeT &getNodeType();

    void preAllocateLogSpace(const size_t nSlots);
    void logData(const double time);
    void saveLogData(string filename);
    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);
    double &getDataRef(const size_t data_type);
    NodeTypeT mNodeType;
    vector<double> mDataVector;
    vector<Port*> mPortPtrs;
    vector<Port*> mTransparentPortPtrs;

private:
    string mName;
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;
    bool mLogSpaceAllocated;
    size_t mLogCtr;
    void setPort(Port *pPort);
    void setTransparentPort(Port *pPort);
    bool connectedToPort(Port *pPort);

};

typedef ClassFactory<NodeTypeT, Node> NodeFactory;
extern NodeFactory gCoreNodeFactory;
DLLIMPORTEXPORT NodeFactory* getCoreNodeFactoryPtr();

#endif // NODE_H_INCLUDED
