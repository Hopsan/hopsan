#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include <vector>
#include <string>
using namespace std;

class Node
{
public:
    Node(string name);
    string &getName();

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);

    string &getNodeType();

protected:
    vector<double> mDataVector;
    string mNodeType;

private:
    string mName;


};

class NodeFluid :public Node
{
public:
    NodeFluid(string name);
    enum {MASSFLOW, PRESSURE, TEMPERATURE};
};

class NodeHydraulic :public NodeFluid
{
public:
    NodeHydraulic(string name);
    enum {MASSFLOW, PRESSURE, TEMPERATURE, HEATFLOW};
};

class NodeMech :public Node
{
public:
    NodeMech(string name);
    enum {VELOCITY, FORCE};
};

#endif // NODES_H_INCLUDED
