#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include "Node.h"

class NodeHydraulic :public Node
{
    static Node* CreatorFunction() {return new NodeHydraulic;}
    static string iDummyId;

public:
    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    NodeHydraulic() : Node()
    {
        mNodeType = "NodeHydraulic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


class NodeMechanic :public Node
{
    static Node* CreatorFunction() {return new NodeMechanic;}
    static string iDummyId;

public:
    enum {VELOCITY, FORCE, DATALENGTH};
    NodeMechanic() : Node()
    {
        mNodeType = "NodeMechanic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};

#endif // NODES_H_INCLUDED
