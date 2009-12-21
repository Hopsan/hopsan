#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include <vector>
#include <string>
#include <map>
using namespace std;

class Node
{
public:
    Node();
    //string &getName();
    string &getNodeType();

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);
    double &getDataRef(const size_t data_type);

    void preAllocateLogSpace(const size_t nSlots);
    void logData(const double time);
    void saveLogData(string filename);

protected:

    string mNodeType;
    vector<double> mDataVector;

private:
    string mName;
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;
    bool mLogSpaceAllocated;
    size_t mLogCtr;

};


class NodeHydraulic :public Node
{
public:
    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    NodeHydraulic() : Node()
    {
        mNodeType = "NodeHydraulic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};

class NodeMech :public Node
{
public:
    enum {VELOCITY, FORCE, DATALENGTH};
    NodeMech() : Node()
    {
        mNodeType = "NodeMech";
        mDataVector.resize(DATALENGTH,0.0);
    }
};

#endif // NODES_H_INCLUDED
