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

class NodeHydraulic :public Node
{
public:
    NodeHydraulic();
    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
};

class NodeMech :public Node
{
public:
    NodeMech();
    enum {VELOCITY, FORCE, DATALENGTH};
};

#endif // NODES_H_INCLUDED
