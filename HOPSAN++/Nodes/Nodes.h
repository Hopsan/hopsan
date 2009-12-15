#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include <vector>
#include <string>
using namespace std;

class Node
{
public:
    Node();
    //string &getName();
    string &getNodeType();

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);

    void logData(const double time);
    void saveLogData(string filename);

protected:
    vector<double> mDataVector;
    string mNodeType;
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;

private:
    string mName;


};

class NodeFluid :public Node
{
public:
    NodeFluid();
    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP};
};

class NodeHydraulic :public NodeFluid
{
public:
    NodeHydraulic();
    enum {MASSFLOW, PRESSURE, TEMPERATURE, HEATFLOW, DATALENGTH}; //Which is used here, enum from NodeFluid or this one???
};

class NodeMech :public Node
{
public:
    NodeMech();
    enum {VELOCITY, FORCE};
};

#endif // NODES_H_INCLUDED
