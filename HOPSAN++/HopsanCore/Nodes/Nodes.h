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
//! @ingroup Nodes
//!
class NodeSignal :public Node
{
    //friend void register_nodes(NodeFactory* nfact_ptr);

public:
    enum {VALUE, DATALENGTH};
    static Node* CreatorFunction() {return new NodeSignal;}

private:
    //static NodeTypeT iDummyId;

    NodeSignal() : Node()
    {
        mNodeType = "NodeSignal";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


//!
//! @class NodeHydraulic
//! @brief A hydraulic node
//! @ingroup Nodes
//!
class NodeHydraulic :public Node
{
    //friend void register_nodes(NodeFactory* nfact_ptr);

public:
    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    static Node* CreatorFunction() {return new NodeHydraulic;}
private:
    //static NodeTypeT iDummyId;

    NodeHydraulic() : Node()
    {
        mNodeType = "NodeHydraulic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


//!
//! @class NodeMechanic
//! @brief A mechanic node
//! @ingroup Nodes
//!
class NodeMechanic :public Node
{
    //friend void register_nodes(NodeFactory* nfact_ptr);

public:
    enum {VELOCITY, FORCE, POSITION, WAVEVARIABLE, CHARIMP, DATALENGTH};
    static Node* CreatorFunction() {return new NodeMechanic;}
private:
    //static NodeTypeT iDummyId;

    NodeMechanic() : Node()
    {
        mNodeType = "NodeMechanic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};

#endif // NODES_H_INCLUDED
