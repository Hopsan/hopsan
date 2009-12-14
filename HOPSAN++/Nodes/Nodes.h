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

protected:
    vector<double> mDataVector;
    string mNodeType;

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
    enum {MASSFLOW, PRESSURE, TEMPERATURE, HEATFLOW};
};

class NodeMech :public Node
{
public:
    NodeMech();
    enum {VELOCITY, FORCE};
};

#endif // NODES_H_INCLUDED
