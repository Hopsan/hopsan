#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include "../Node.h"

DLLIMPORTEXPORT void register_nodes(NodeFactory* nfact_ptr);

class NodeSignal :public Node
{
public:
    static Node* CreatorFunction() {return new NodeSignal;}
    //static NodeTypeT iDummyId;

    enum {VALUE, DATALENGTH};
    NodeSignal() : Node()
    {
        mNodeType = "NodeSignal";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


class NodeHydraulic :public Node
{
public:
    static Node* CreatorFunction() {return new NodeHydraulic;}
    //static NodeTypeT iDummyId;

    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    NodeHydraulic() : Node()
    {
        mNodeType = "NodeHydraulic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


class NodeMechanic :public Node
{
public:
    static Node* CreatorFunction() {return new NodeMechanic;}
    //static NodeTypeT iDummyId;

    enum {VELOCITY, FORCE, POSITION, WAVEVARIABLE, CHARIMP, DATALENGTH};
    NodeMechanic() : Node()
    {
        mNodeType = "NodeMechanic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};

#endif // NODES_H_INCLUDED
