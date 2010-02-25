//!
//! @file   Nodes.h
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains all built in node types
//!
//$Id$

#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include "../Node.h"

DLLIMPORTEXPORT void register_nodes(NodeFactory* nfact_ptr);

//!
//! @class NodeSignal
//! @brief A signal node
//! @ingroup SignalNode
//!
class NodeSignal :public Node
{
public:
    enum {VALUE, DATALENGTH};
    static Node* CreatorFunction() {return new NodeSignal;}

private:
    NodeSignal() : Node()
    {
        mNodeType = "NodeSignal";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


//!
//! @class NodeHydraulic
//! @brief A hydraulic node
//! @ingroup HydraulicNode
//!
class NodeHydraulic :public Node
{
public:
    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    static Node* CreatorFunction() {return new NodeHydraulic;}

private:
    NodeHydraulic() : Node()
    {
        mNodeType = "NodeHydraulic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


//!
//! @class NodeMechanic
//! @brief A mechanic node
//! @ingroup MechanicalNode
//!
class NodeMechanic :public Node
{
public:
    enum {VELOCITY, FORCE, POSITION, WAVEVARIABLE, CHARIMP, DATALENGTH};
    static Node* CreatorFunction() {return new NodeMechanic;}

private:
    NodeMechanic() : Node()
    {
        mNodeType = "NodeMechanic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};

#endif // NODES_H_INCLUDED
