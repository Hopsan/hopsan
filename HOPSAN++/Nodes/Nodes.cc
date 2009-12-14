#include "Nodes.h"

//Node constructor
Node::Node()
{
    //mName = name;
    mNodeType = "Node";
    mDataVector.clear();
}

//string &Node::getName()
//{
//    return mName;
//}

string &Node::getNodeType()
{
    return mNodeType;
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
NodeFluid::NodeFluid() : Node()
{
    mNodeType = "NodeFluid";
    mDataVector.resize(3);
}

//Hydraulic Node constructor
NodeHydraulic::NodeHydraulic() : NodeFluid()
{
    mNodeType = "NodeHydraulic";
    mDataVector.resize(4);
}

//Mechanic Node constructor
NodeMech::NodeMech() : Node()
{
    mNodeType = "NodeMech";
    mDataVector.resize(2);
}
