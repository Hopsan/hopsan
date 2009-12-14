#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include <vector>
#include <string>
using namespace std;

//enum {HYDRAULICNODE, MECHANICNODE}



//class NodeData
//{
//    public:
//    NodeData();
//
//    private:
//    vector<int> mDataMap;
//    vector<double> mDataVector;
//
//};

class Node
{
public:
    Node(string name);
    string &getName();

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);

protected:
    vector<double> mDataVector;

private:
    size_t mNodeType;
    string mName;

};

class HydraulicNode :public Node // Måste ha samma uppsättning attribut och metoder som Node för att vara "polymorphic"
{
public:
    HydraulicNode(string name);
    enum enum1 {MASSFLOW, PRESSURE, TEMPERATURE};
};

class MechNode :public Node // Måste ha samma uppsättning attribut och metoder som Node för att vara "polymorphic"
{
public:
    MechNode(string name);
    enum {VELOCITY, FORCE};
};

#endif // NODES_H_INCLUDED
