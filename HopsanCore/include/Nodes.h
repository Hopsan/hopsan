/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
#include "HopsanCoreMacros.h"

namespace hopsan {

void register_default_nodes(NodeFactory* pNodeFactory);

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

    void setSignalQuantity(const HString &rQuantity, const HString &rUnit)
    {
        if (mDataDescriptions[Value].userModifiableQuantity)
        {
            mDataDescriptions[Value].quantity = rQuantity;
            mDataDescriptions[Value].unit = rUnit;
        }
    }

    void setSignalQuantityModifyable(bool tf)
    {
        mDataDescriptions[Value].userModifiableQuantity = tf;
    }

    HString getSignalQuantity() const
    {
        return mDataDescriptions[Value].quantity;
    }

    bool getSignalQuantityModifyable() const
    {
        return mDataDescriptions[Value].userModifiableQuantity;
    }

    void copySignalQuantityAndUnitTo(Node *pOtherNode) const
    {
        // Copy variable values from this to pNode
        if(pOtherNode->getNodeType()==this->getNodeType())
        {
            for(size_t i=0; i<mDataDescriptions.size(); ++i)
            {
                pOtherNode->setSignalQuantity(mDataDescriptions[i].quantity, mDataDescriptions[i].unit);
                pOtherNode->setSignalQuantityModifyable(mDataDescriptions[i].userModifiableQuantity);
            }
        }
    }

private:
    NodeSignal() : Node(DataLength)
    {
        setNiceName("signal");
        setDataCharacteristics(Value, "Value", "y", "");
    }
};

//! @brief A hydraulic node
//! @ingroup NodeHydraulic
class NodeHydraulic :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeHydraulicTemperature
    enum DataIndexEnumT {Flow, Pressure, Temperature, WaveVariable, CharImpedance, HeatFlow, DataLength};
    enum DataIndexEnumOldT {FLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW};
    static Node* CreatorFunction() {return new NodeHydraulic;}

private:
    NodeHydraulic() : Node(DataLength)
    {
        setNiceName("hydraulic");
        setDataCharacteristics(Flow, "Flow", "q", "Flow", FlowType);
        setDataCharacteristics(Pressure, "Pressure", "p", "Pressure", IntensityType);
        setDataCharacteristics(Temperature, "Temperature", "T", "K", HiddenType);
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Pressure", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "?", TLMType);
        setDataCharacteristics(HeatFlow, "HeatFlow", "Qdot", "?", HiddenType);

        // Set default initial startvales to reasonable (non-zero) values
        mDataValues[Pressure] = 100000;
        mDataValues[WaveVariable] = 100000;
        mDataValues[Temperature] = 293;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
    {
        pOtherNode->setDataValue(WaveVariable, mDataValues[Pressure]);
        //! todo Maybe also write CHARIMP?
    }
};

////! @brief A hydraulic node
////! @ingroup NodeHydraulic
//class NodeHydraulic :public Node
//{
//public:
//    //! @brief The data variable indexes, DataLength is used internally
//    //! @ingroup NodeHydraulic
//    enum DataIndexEnumT {Flow, Pressure, WaveVariable, CharImpedance, DataLength};
//    enum DataIndexEnumOldT {FLOW, PRESSURE, WAVEVARIABLE, CHARIMP};
//    static Node* CreatorFunction() {return new NodeHydraulic;}

//private:
//    NodeHydraulic() : Node(DataLength)
//    {
//        setNiceName("hydraulic");
//        setDataCharacteristics(Flow, "Flow", "q", "Flow", FlowType);
//        setDataCharacteristics(Pressure, "Pressure", "p", "Pressure", IntensityType);
//        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Pressure", TLMType);
//        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "?", TLMType);

//        // Set default initial startvales to reasonable (non-zero) values
//        mDataValues[Pressure] = 100000;
//        mDataValues[WaveVariable] = 100000;
//    }

//    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
//    {
//        pOtherNode->setDataValue(WaveVariable, mDataValues[Pressure]);
//        //! todo Maybe also write CHARIMP?
//    }
//};

//! @brief A hydraulic node
//! @ingroup NodeHydraulicTemperature
class NodeHydraulicTemperature :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeHydraulicTemperature
    enum DataIndexEnumT {Flow, Pressure, Temperature, WaveVariable, CharImpedance, HeatFlow, DataLength};
    static Node* CreatorFunction() {return new NodeHydraulicTemperature;}

private:
    NodeHydraulicTemperature() : Node(DataLength)
    {
        setNiceName("hydraulic");
        setDataCharacteristics(Flow, "Flow", "q", "Flow", FlowType);
        setDataCharacteristics(Pressure, "Pressure", "p", "Pressure", IntensityType);
        setDataCharacteristics(Temperature, "Temperature", "T", "K", HiddenType);
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Pressure", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "?", TLMType);
        setDataCharacteristics(HeatFlow, "HeatFlow", "Qdot", "?", HiddenType);

        // Set default initial startvales to reasonable (non-zero) values
        mDataValues[Pressure] = 100000;
        mDataValues[WaveVariable] = 100000;
        mDataValues[Temperature] = 293;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
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

        // Set default initial startvales to reasonable (non-zero) values
        mDataValues[Pressure] = 100000;
        mDataValues[WaveVariable] = 100000;
        mDataValues[Temperature] = 293;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
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
        setDataCharacteristics(Velocity, "Velocity", "v", "Velocity", FlowType);
        setDataCharacteristics(Force, "Force", "f", "Force", IntensityType);
        setDataCharacteristics(Position, "Position", "x", "Position");
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Force", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "N s/m", TLMType);
        setDataCharacteristics(EquivalentMass, "EquivalentMass", "me", "kg", DefaultType);

        mDataValues[EquivalentMass]=1;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
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
        setDataCharacteristics(AngularVelocity, "AngularVelocity", "w", "AngularVelocity", FlowType);
        setDataCharacteristics(Torque, "Torque", "T", "Torque", IntensityType);
        setDataCharacteristics(Angle, "Angle", "a", "Angle");
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Torque", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "?", TLMType);
        setDataCharacteristics(EquivalentInertia, "EquivalentInertia", "Je", "kgm^2", HiddenType);
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
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
        setDataCharacteristics(Voltage, "Voltage", "U", "Voltage", IntensityType);
        setDataCharacteristics(Current, "Current", "I", "Current", FlowType);
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Voltage", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "V/A", TLMType);
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
    {
        pOtherNode->setDataValue(WaveVariable, mDataValues[Voltage]);
        //! todo Maybe also write CharImpedance?
    }
};


//! @brief A 2D mechanic node
//! @ingroup NodeMechanic
class NodeMechanic2D : public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeMechanic
    enum DataIndexEnumT {AngleR, AngularVelocityR, TorqueR, WaveVariableR, CharImpedanceR, EquivalentInertiaR,
                         PositionX, VelocityX, ForceX,WaveVariableX, CharImpedanceX, EquivalentMassX,
                         PositionY, VelocityY, ForceY, WaveVariableY, CharImpedanceY, EquivalentMassY, DataLength};
    enum DataIndexEnumOldT {VELOCITY, FORCE, POSITION, WAVEVARIABLE, CHARIMP, EQMASS};
    static Node* CreatorFunction() {return new NodeMechanic2D;}

private:
    NodeMechanic2D() : Node(DataLength)
    {
        setNiceName("mechanic2d");
        setDataCharacteristics(AngularVelocityR, "AngularVelocity", "wr", "rad/s", FlowType);
        setDataCharacteristics(VelocityX, "VelocityX", "vx", "m/s", FlowType);
        setDataCharacteristics(VelocityY, "VelocityY", "vy", "m/s", FlowType);
        setDataCharacteristics(TorqueR, "TorqueR", "Tr", "Nm", IntensityType);
        setDataCharacteristics(ForceX, "ForceX", "fx", "N", IntensityType);
        setDataCharacteristics(ForceY, "ForceY", "fy", "N", IntensityType);
        setDataCharacteristics(AngleR, "AngleR", "a", "rad");
        setDataCharacteristics(PositionX, "PositionX", "x", "m");
        setDataCharacteristics(PositionY, "PositionY", "y", "m");
        setDataCharacteristics(WaveVariableR, "WaveVariableR", "cr", "Nm", TLMType);
        setDataCharacteristics(WaveVariableX, "WaveVariableX", "cx", "N", TLMType);
        setDataCharacteristics(WaveVariableY, "WaveVariableY", "cy", "N", TLMType);
        setDataCharacteristics(CharImpedanceR, "CharImpedanceR", "Zcr", "?", TLMType);
        setDataCharacteristics(CharImpedanceX, "CharImpedanceX", "Zcx", "Ns/m", TLMType);
        setDataCharacteristics(CharImpedanceY, "CharImpedanceY", "Zcy", "Ns/m", TLMType);
        setDataCharacteristics(EquivalentInertiaR, "EquivalentInertiaR", "mer", "kgm^2", DefaultType);
        setDataCharacteristics(EquivalentMassX, "EquivalentMassX", "mex", "kg", DefaultType);
        setDataCharacteristics(EquivalentMassY, "EquivalentMassY", "mey", "kg", DefaultType);

        mDataValues[EquivalentInertiaR]=1;
        mDataValues[EquivalentMassX]=1;
        mDataValues[EquivalentMassY]=1;
    }

    virtual void setTLMNodeDataValuesTo(Node *pOtherNode) const
    {
        pOtherNode->setDataValue(WaveVariableR, mDataValues[TorqueR]);
        pOtherNode->setDataValue(WaveVariableX, mDataValues[ForceX]);
        pOtherNode->setDataValue(WaveVariableY, mDataValues[ForceY]);
        //! todo Maybe also write CharImpedance?
    }
};


class NodeModelica : public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeEmpty
    enum DataIndexEnumT {DataLength};
    static Node* CreatorFunction() {return new NodeModelica;}

private:
    NodeModelica() : Node(DataLength)
    {
        setNiceName("modelica");
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
