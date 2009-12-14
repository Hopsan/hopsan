#include "Nodes.h"

//Node constructor
Node::Node(string name)
{
    mName = name;
    mDataVector.clear();
}

string &Node::getName()
{
    return mName;
}

void Node::setData(const size_t data_type, double data)
{
    mDataVector[data_type] = data;
}

double Node::getData(const size_t data_type)
{
    return mDataVector[data_type];
}

//Hydraulic Node constructor
HydraulicNode::HydraulicNode(string name) : Node(name)
{
    //mNodeType = ???
    mDataVector.resize(2);
}

//Mechanic Node constructor
MechNode::MechNode(string name) : Node(name)
{
    //mNodeType = ???
    mDataVector.resize(2);
}
