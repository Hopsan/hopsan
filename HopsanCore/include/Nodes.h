/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

    virtual void setSignalQuantity(const HString &rQuantity, const HString &rUnit, const size_t dataId=Value)
    {
        HOPSAN_UNUSED(dataId)
        if (mDataDescriptions[Value].userModifiableQuantity)
        {
            mDataDescriptions[Value].quantity = rQuantity;
            mDataDescriptions[Value].unit = rUnit;
        }
    }

    virtual void setSignalQuantityModifyable(bool tf, const size_t dataId=Value)
    {
        HOPSAN_UNUSED(dataId)
        mDataDescriptions[Value].userModifiableQuantity = tf;
    }

    virtual HString getSignalQuantity(const size_t dataId=Value) const
    {
        HOPSAN_UNUSED(dataId)
        return mDataDescriptions[Value].quantity;
    }

    virtual bool getSignalQuantityModifyable(const size_t dataId=Value) const
    {
        HOPSAN_UNUSED(dataId)
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

//! @brief The base class for n-dimensional signal nodes, (that can be dynamically sized, but size should be kept constant during use)
//! @ingroup NodeSignal
class NodeSignalND :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeSignal
    enum DataIndexEnumT {Value};
    static Node* CreatorFunction() {return new NodeSignalND;}

    virtual void setSignalNumDimensions(size_t numDims);

    virtual void setSignalQuantity(const HString &rQuantity, const HString &rUnit, const size_t dataId=Value)
    {
        if (mDataDescriptions[dataId].userModifiableQuantity)
        {
            mDataDescriptions[dataId].quantity = rQuantity;
            mDataDescriptions[dataId].unit = rUnit;
        }
    }

    virtual void setSignalQuantityModifyable(bool tf, const size_t dataId=Value)
    {
        mDataDescriptions[dataId].userModifiableQuantity = tf;
    }

    virtual HString getSignalQuantity(const size_t dataId=Value) const
    {
        return mDataDescriptions[dataId].quantity;
    }

    virtual bool getSignalQuantityModifyable(const size_t dataId=Value) const
    {
        return mDataDescriptions[dataId].userModifiableQuantity;
    }

protected:
    NodeSignalND() : Node(1)
    {
        // This is the default setup, dynamic reconfiguration is possible
        setNiceName("signal1d");
        setDataCharacteristics(0, "v1", "y1", "");
    }
};

//! @brief The 2d signal node
//! @ingroup NodeSignal
class NodeSignal2D :public NodeSignalND
{
public:
    //! @ingroup NodeSignal
    static Node* CreatorFunction() {return new NodeSignal2D;}

private:
    NodeSignal2D() : NodeSignalND()
    {
        setSignalNumDimensions(2);
    }
};

//! @brief The 3d signal node
//! @ingroup NodeSignal
class NodeSignal3D :public NodeSignalND
{
public:
    //! @ingroup NodeSignal
    static Node* CreatorFunction() {return new NodeSignal3D;}

private:
    NodeSignal3D() : NodeSignalND()
    {
        setSignalNumDimensions(3);
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
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "Pa s/m^3", TLMType);
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
//        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "Pa s/m^3", TLMType);

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
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "Pa s/m^3", TLMType);
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
    enum DataIndexEnumT {EnergyFlow, Pressure, WaveVariable, CharImpedance, MassFlow, Density, DensityWaveVariable, DensityCharImpedance, Temperature, DataLength};
    enum DataIndexEnumOldT {ENERGYFLOW, PRESSURE, WAVEVARIABLE, CHARIMP, MASSFLOW, DENSITY, DENSITYWAVEVARIABLE, DENSITYCHARIMP, TEMPERATURE};
    static Node* CreatorFunction() {return new NodePneumatic;}

private:
    NodePneumatic() : Node(DataLength)
    {
        setNiceName("pneumatic");
        setDataCharacteristics(EnergyFlow, "EnergyFlow", "Qdot", "J/s", FlowType);
        setDataCharacteristics(Pressure, "Pressure", "p", "Pa", IntensityType);
        setDataCharacteristics(WaveVariable, "WaveVariable", "c", "Pa", TLMType);
        setDataCharacteristics(CharImpedance, "CharImpedance", "Zc", "s/m^3", TLMType);
        setDataCharacteristics(MassFlow, "MassFlow", "mdot", "kg/s", FlowType);
        setDataCharacteristics(Density, "Density", "rho", "kg/m^3", IntensityType);
        setDataCharacteristics(DensityWaveVariable, "DensityWaveVariable", "crho", "kg/m^3", TLMType);
        setDataCharacteristics(DensityCharImpedance, "DensityCharImpedance", "Zcrho", "s/m^3", TLMType);
        setDataCharacteristics(Temperature, "Temperature", "T", "K", DefaultType);

        // Set default initial startvales to reasonable (non-zero) values
        mDataValues[Pressure] = 100000;
        mDataValues[WaveVariable] = 100000;
        mDataValues[Density] = 1.225;
        mDataValues[DensityWaveVariable] = 1.225;
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
        setDataCharacteristics(EquivalentInertia, "EquivalentInertia", "Je", "MomentOfInertia", HiddenType);
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
        setDataCharacteristics(EquivalentInertiaR, "EquivalentInertiaR", "mer", "MomentOfInertia", DefaultType);
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


//! @brief A petri net node
//! @ingroup NodePetriNet
class NodePetriNet : public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodePetriNet
    enum DataIndexEnumT {State, Flow, DataLength};
    static Node* CreatorFunction() {return new NodePetriNet;}

private:
    NodePetriNet() : Node(DataLength)
    {
        setNiceName("petrinet");
        setDataCharacteristics(State, "State", "s", "-", IntensityType);
        setDataCharacteristics(Flow, "Flow", "q", "-", FlowType);
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
