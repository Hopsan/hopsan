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

    friend class Component;
    friend class ComponentSystem;

///TODO: Nodes should know their ports so a check can be performed by the node at connection time, the check should be virtual and implement different checks at different nodes.
public:
    Node();
    NodeTypeT &getNodeType();

    void setData(const size_t data_type, double data); ///TODO: Move to protected
    double getData(const size_t data_type); ///TODO: Move to protected
    double &getDataRef(const size_t data_type); ///TODO: Move to protected

    void preAllocateLogSpace(const size_t nSlots);
    void logData(const double time);
    void saveLogData(string filename);

protected:
    NodeTypeT mNodeType;
    vector<double> mDataVector;
    vector<Port*> mPortPtrs;

private:
    string mName;
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;
    bool mLogSpaceAllocated;
    size_t mLogCtr;
    void setPort(Port *pPort);
    bool connectedToPort(Port *pPort);

};

typedef ClassFactory<NodeTypeT, Node> NodeFactory;
extern NodeFactory gCoreNodeFactory;
DLLIMPORTEXPORT NodeFactory* getCoreNodeFactoryPtr();

#endif // NODE_H_INCLUDED
