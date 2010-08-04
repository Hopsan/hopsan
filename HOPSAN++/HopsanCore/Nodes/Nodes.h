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
    NodeSignal() : Node(DATALENGTH)
    {
        mNodeType = "NodeSignal";
        setDataNameAndUnit(VALUE, "Value", "-");
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
    NodeHydraulic() : Node(DATALENGTH)
    {
        mNodeType = "NodeHydraulic";
        setDataNameAndUnit(MASSFLOW, "MassFlow", "Kg/m^3");
        setDataNameAndUnit(PRESSURE, "Pressure", "Pa");
        setDataNameAndUnit(TEMPERATURE, "Temperature", "?");
        setDataNameAndUnit(WAVEVARIABLE, "WaveVariable", "?");
        setDataNameAndUnit(CHARIMP, "CharImp", "?");
        setDataNameAndUnit(HEATFLOW, "HeatFlow", "?");
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
    NodeMechanic() : Node(DATALENGTH)
    {
        mNodeType = "NodeMechanic";
        setDataNameAndUnit(VELOCITY, "Velocity", "m/s");
        setDataNameAndUnit(FORCE, "Force", "N");
        setDataNameAndUnit(POSITION, "Position", "m");
        setDataNameAndUnit(WAVEVARIABLE, "WaveVariable", "?");
        setDataNameAndUnit(CHARIMP, "CharImp", "?");
    }
};


//!
//! @class NodeMechanicRotational
//! @brief A rotational mechanic node
//! @ingroup RotationalMechanicalNode
//!
class NodeMechanicRotational :public Node
{
public:
    enum {ANGULARVELOCITY, TORQUE, ANGLE, WAVEVARIABLE, CHARIMP, DATALENGTH};
    static Node* CreatorFunction() {return new NodeMechanicRotational;}

private:
    NodeMechanicRotational() : Node(DATALENGTH)
    {
        mNodeType = "NodeMechanicRotational";
        setDataNameAndUnit(ANGULARVELOCITY, "Angular Velocity", "rad/s");
        setDataNameAndUnit(TORQUE, "Torque", "Nm");
        setDataNameAndUnit(ANGLE, "Angle", "rad");
        setDataNameAndUnit(WAVEVARIABLE, "WaveVariable", "?");
        setDataNameAndUnit(CHARIMP, "CharImp", "?");
    }
};

#endif // NODES_H_INCLUDED
