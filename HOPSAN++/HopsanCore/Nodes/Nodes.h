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

#include <iostream>

namespace hopsan {

    DLLIMPORTEXPORT void register_nodes(NodeFactory* nfampND_ct);

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
            setDataCharacteristics(VALUE, "Value", "-", Node::FLOW);
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
        enum {FLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
        static Node* CreatorFunction() {return new NodeHydraulic;}

    private:
        NodeHydraulic() : Node(DATALENGTH)
        {
            setDataCharacteristics(FLOW, "Flow", "m^3/s", Node::FLOW);
            setDataCharacteristics(PRESSURE, "Pressure", "Pa", Node::INTENSITY);
            setDataCharacteristics(TEMPERATURE, "Temperature", "K", Node::INTENSITY, Node::NOPLOT);
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "?", Node::INTENSITY, Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "?", Node::FLOW, Node::NOPLOT);
            setDataCharacteristics(HEATFLOW, "HeatFlow", "?", Node::FLOW, Node::NOPLOT);
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(size_t i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[PRESSURE]);
                    std::cout << "SpecialStartValue: Name: " << mDataNames[i] << "  Value: " << mDataVector[i] << "  Unit: " << mDataUnits[i] << std::endl;
                }
                //! todo Maybe also write CHARIMP?
            }
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
            setDataCharacteristics(VELOCITY, "Velocity", "m/s", Node::INTENSITY);
            setDataCharacteristics(FORCE, "Force", "N", Node::FLOW);
            setDataCharacteristics(POSITION, "Position", "m", Node::INTENSITY);
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "?", Node::FLOW, Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "?", Node::INTENSITY, Node::NOPLOT);
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(size_t i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[FORCE]);
                }
                //! todo Maybe also write CHARIMP?
            }
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
            setDataCharacteristics(ANGULARVELOCITY, "Angular Velocity", "rad/s", Node::INTENSITY);
            setDataCharacteristics(TORQUE, "Torque", "Nm", Node::FLOW);
            setDataCharacteristics(ANGLE, "Angle", "rad", Node::INTENSITY);
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "?", Node::FLOW, Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "?", Node::INTENSITY, Node::NOPLOT);
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(size_t i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[TORQUE]);
                }
                //! todo Maybe also write CHARIMP?
            }
        }
    };
}

#endif // NODES_H_INCLUDED
