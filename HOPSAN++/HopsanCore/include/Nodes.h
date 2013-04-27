/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   Nodes.h
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains all built in node types
//!
//$Id$

#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include "Node.h"

namespace hopsan {

DLLIMPORTEXPORT void register_default_nodes(NodeFactory* pNodeFactory);

//! @brief A signal node
//! @ingroup NodeSignal
class NodeSignal :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeSignal
    enum DataIndexEnumT {Value, DataLength};
    enum DataIndexEnumOldT {VALUE};
    static Node* CreatorFunction() {return new NodeSignal;}

    void setSignalDataUnitAndDescription(const std::string &rUnit, const std::string &rDescription)
    {
        mDataDescriptions[Value].unit = rUnit;
        mDataDescriptions[Value].description = rDescription;
    }

    void copySignalDataUnitAndDescriptionTo(Node *pOtherNode) const
    {
        // Copy variable valus from this to pNode
        if(pOtherNode->getNodeType()==this->getNodeType())
        {
            for(size_t i=0; i<mDataDescriptions.size(); ++i)
            {
                pOtherNode->setSignalDataUnitAndDescription(mDataDescriptions[i].unit, mDataDescriptions[i].description);
            }
        }
    }

    //! @brief For signals allways return Value slot even if name has been changed
    int getDataIdFromName(const std::string /*name*/) const
    {
        return Value;
    }

private:
    NodeSignal() : Node(DataLength)
    {
        setNiceName("signal");
        setDataCharacteristics(Value, "Value", "y", "-");
    }
};


//! @brief A hydraulic node
//! @ingroup NodeHydraulic
class NodeHydraulic :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeHydraulic
    enum DataIndexEnumT {Flow, Pressure, Temperature, WaveVariable, CharImpedance, HeatFlow, DataLength};
    enum DataIndexEnumOldT {FLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW};
    static Node* CreatorFunction() {return new NodeHydraulic;}

private:
    NodeHydraulic() : Node(DataLength)
    {
        setNiceName("hydraulic");
        setDataCharacteristics(Flow, "Flow", "q", "m^3/s", FlowType);
        setDataCharacteristics(Pressure, "Pressure", "p", "Pa", IntensityType);
        setDataCharacteristics(Temperature, "Temperature", "T", "K", HiddenType);
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Pa", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "?", TLMType);
        setDataCharacteristics(HeatFlow, "HeatFlow", "Qdot", "?", HiddenType);

        // Set default intial startvales to resonable (non-zero) values
        mDataValues[Pressure] = 100000;
        mDataValues[WaveVariable] = 100000;
        mDataValues[Temperature] = 293;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode)
    {
        pOtherNode->setDataValue(WaveVariable, mDataValues[Pressure]);
        //! todo Maybe also write CHARIMP?
    }
};


//! @brief A pneumatic node
//! @ingroup NodePneumatic
class NodePneumatic :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodePneumatic
    enum DataIndexEnumT {MassFlow, EnergyFlow, Pressure, Temperature, WaveVariable, CharImpedance, DataLength};
    enum DataIndexEnumOldT {MASSFLOW, ENERGYFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP};
    static Node* CreatorFunction() {return new NodePneumatic;}

private:
    NodePneumatic() : Node(DataLength)
    {
        setNiceName("pneumatic");
        setDataCharacteristics(MassFlow, "MassFlow", "mdot", "kg/s", FlowType);
        setDataCharacteristics(Pressure, "Pressure", "p", "Pa", IntensityType);
        setDataCharacteristics(Temperature, "Temperature", "T", "K", DefaultType);
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Pa", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "?", TLMType);
        setDataCharacteristics(EnergyFlow, "EnergyFlow", "Qdot", "J/s", DefaultType);

        // Set default intial startvales to resonable (non-zero) values
        mDataValues[Pressure] = 100000;
        mDataValues[WaveVariable] = 100000;
        mDataValues[Temperature] = 293;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode)
    {
        pOtherNode->setDataValue(WaveVariable, mDataValues[Pressure]);
        //! todo Maybe also write CharImpedance?
    }
};

//! @brief A mechanic node
//! @ingroup NodeMechanic
class NodeMechanic :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeMechanic
    enum DataIndexEnumT {Velocity, Force, Position, WaveVariable, CharImpedance, EquivalentMass, DataLength};
    enum DataIndexEnumOldT {VELOCITY, FORCE, POSITION, WAVEVARIABLE, CHARIMP, EQMASS};
    static Node* CreatorFunction() {return new NodeMechanic;}

private:
    NodeMechanic() : Node(DataLength)
    {
        setNiceName("mechanic");
        setDataCharacteristics(Velocity, "Velocity", "v", "m/s", FlowType);
        setDataCharacteristics(Force, "Force", "F", "N", IntensityType);
        setDataCharacteristics(Position, "Position", "x", "m");
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "N", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "N s/m", TLMType);
        setDataCharacteristics(EquivalentMass, "EquivalentMass", "me", "kg", DefaultType);

        mDataValues[EquivalentMass]=1;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode)
    {
        pOtherNode->setDataValue(WaveVariable, mDataValues[Force]);
        //! todo Maybe also write CharImpedance?
    }
};

//! @brief A rotational mechanic node
//! @ingroup NodeMechanicRotational
class NodeMechanicRotational :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeMechanicRotational
    enum DataIndexEnumT {AngularVelocity, Torque, Angle, WaveVariable, CharImpedance, EquivalentInertia, DataLength};
    enum DataIndexEnumOldT {ANGULARVELOCITY, TORQUE, ANGLE, WAVEVARIABLE, CHARIMP, EQINERTIA};
    static Node* CreatorFunction() {return new NodeMechanicRotational;}

private:
    NodeMechanicRotational() : Node(DataLength)
    {
        setNiceName("mechanicrotational");
        setDataCharacteristics(AngularVelocity, "AngularVelocity", "w", "rad/s", FlowType);
        setDataCharacteristics(Torque, "Torque", "T", "Nm", IntensityType);
        setDataCharacteristics(Angle, "Angle", "a", "rad");
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Nm", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "?", TLMType);
        setDataCharacteristics(EquivalentInertia, "EquivalentInertia", "Je", "kgm^2", HiddenType);
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode)
    {
        pOtherNode->setDataValue(WaveVariable, mDataValues[Torque]);
        //! todo Maybe also write CharImpedance?
    }
};



//! @brief An electric node
//! @ingroup NodeElectric
class NodeElectric :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeElectric
    enum DataIndexEnumT {Voltage, Current, WaveVariable, CharImpedance, DataLength};
    enum DataIndexEnumOldT {VOLTAGE, CURRENT, WAVEVARIABLE, CHARIMP};
    static Node* CreatorFunction() {return new NodeElectric;}

private:
    NodeElectric() : Node(DataLength)
    {
        setNiceName("electric");
        setDataCharacteristics(Voltage, "Voltage", "U", "V", IntensityType);
        setDataCharacteristics(Current, "Current", "I", "A", FlowType);
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "V", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "V/A", TLMType);
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode)
    {
        pOtherNode->setDataValue(WaveVariable, mDataValues[Voltage]);
        //! todo Maybe also write CharImpedance?
    }
};

class NodeEmpty : public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeEmpty
    enum DataIndexEnumT {DataLength};
    static Node* CreatorFunction() {return new NodeEmpty;}

private:
    NodeEmpty() : Node(DataLength)
    {
        setNiceName("empty");
    }
};

}

#endif // NODES_H_INCLUDED
