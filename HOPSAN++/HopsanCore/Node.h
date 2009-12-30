#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <vector>
#include <string>
#include "ClassFactory.h"
#include "win32dll.h"

using namespace std;

typedef string NodeTypeT;

class Node
{
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

#endif // NODE_H_INCLUDED
