#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <vector>
#include <string>
#include "CoreUtilities/ClassFactory.h"
#include "win32dll.h"

using namespace std;

typedef string NodeTypeT;

class DLLIMPORTEXPORT Node
{
///TODO: Nodes should know their ports so a check can be performed by the node at connection time, the check should be virtual and implement different checks at different nodes.
public:
    Node();
    //string &getName();
    NodeTypeT &getNodeType();

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);
    double &getDataRef(const size_t data_type);

    void preAllocateLogSpace(const size_t nSlots);
    void logData(const double time);
    void saveLogData(string filename);

protected:

    NodeTypeT mNodeType;
    vector<double> mDataVector;

private:
    string mName;
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;
    bool mLogSpaceAllocated;
    size_t mLogCtr;

};

typedef ClassFactory<NodeTypeT, Node> NodeFactory;
//static NodeFactory gNodeFactory;

#endif // NODE_H_INCLUDED
