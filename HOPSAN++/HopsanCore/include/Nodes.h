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
//#include <iostream>

namespace hopsan {

DLLIMPORTEXPORT void register_nodes(NodeFactory* pNodeFactory);

//! @brief A signal node
//! @ingroup NodeSignal
class NodeSignal :public Node
{
public:
    //! @brief The data variable indexes, DATALENGTH is used internally
    //! @ingroup NodeSignal
    enum DataIndexEnumT {VALUE, DATALENGTH};
    static Node* CreatorFunction() {return new NodeSignal;}

    void setSignalDataUnit(const std::string unit)
    {
        mDataDescriptions[VALUE].unit = unit;
    }

    void setSignalDataName(const std::string name)
    {
        mDataDescriptions[VALUE].name = name;
    }

    //! @brief For signals allways return VALUE slot even if name has been changed
    int getDataIdFromName(const std::string /*name*/)
    {
        return VALUE;
    }

private:
    NodeSignal() : Node(DATALENGTH)
    {
        setNiceName("signal");
        setDataCharacteristics(VALUE, "Value", "y", "-");
    }
};


//! @brief A hydraulic node
//! @ingroup NodeHydraulic
class NodeHydraulic :public Node
{
public:
    //! @brief The data variable indexes, DATALENGTH is used internally
    //! @ingroup NodeHydraulic
    enum DataIndexEnumT {FLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    static Node* CreatorFunction() {return new NodeHydraulic;}

private:
    NodeHydraulic() : Node(DATALENGTH)
    {
        setNiceName("hydraulic");
        setDataCharacteristics(FLOW, "Flow", "q", "m^3/s", Flow);
        setDataCharacteristics(PRESSURE, "Pressure", "p", "Pa", Intensity);
        setDataCharacteristics(TEMPERATURE, "Temperature", "T", "K", Hidden);
        setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "c", "Pa", TLM);
        setDataCharacteristics(CHARIMP, "CharImp", "Zc", "?", TLM);
        setDataCharacteristics(HEATFLOW, "HeatFlow", "Qdot", "?", Hidden);
    }

    virtual void setSpecialStartValues(Node *pNode)
    {
        pNode->setDataValue(WAVEVARIABLE, mDataValues[PRESSURE]);
        //std::cout << "SpecialStartValue: Name: " << mDataNames[WAVEVARIABLE] << "  Value: " << mDataVector[WAVEVARIABLE] << "  Unit: " << mDataUnits[WAVEVARIABLE] << std::endl;
        //! todo Maybe also write CHARIMP?
    }
};


//! @brief A pneumatic node
//! @ingroup NodePneumatic
class NodePneumatic :public Node
{
public:
    //! @brief The data variable indexes, DATALENGTH is used internally
    //! @ingroup NodePneumatic
    enum DataIndexEnumT {MASSFLOW, ENERGYFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    static Node* CreatorFunction() {return new NodePneumatic;}

private:
    NodePneumatic() : Node(DATALENGTH)
    {
        setNiceName("pneumatic");
        setDataCharacteristics(MASSFLOW, "MassFlow", "mdot", "kg/s", Flow);
        setDataCharacteristics(PRESSURE, "Pressure", "p", "Pa", Intensity);
        setDataCharacteristics(TEMPERATURE, "Temperature", "T", "K", Hidden);
        setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "c", "Pa", TLM);
        setDataCharacteristics(CHARIMP, "CharImp", "Zc", "?", TLM);
        setDataCharacteristics(ENERGYFLOW, "EnergyFlow", "Qdot", "J/s", Hidden);
    }

    virtual void setSpecialStartValues(Node *pNode)
    {
        pNode->setDataValue(WAVEVARIABLE, mDataValues[PRESSURE]);
        //std::cout << "SpecialStartValue: Name: " << mDataNames[i] << "  Value: " << mDataVector[i] << "  Unit: " << mDataUnits[i] << std::endl;
        //! todo Maybe also write CHARIMP?
    }
};

//! @brief A mechanic node
//! @ingroup NodeMechanic
class NodeMechanic :public Node
{
public:
    //! @brief The data variable indexes, DATALENGTH is used internally
    //! @ingroup NodeMechanic
    enum DataIndexEnumT {VELOCITY, FORCE, POSITION, WAVEVARIABLE, CHARIMP, EQMASS, DATALENGTH};
    static Node* CreatorFunction() {return new NodeMechanic;}

private:
    NodeMechanic() : Node(DATALENGTH)
    {
        setNiceName("mechanic");
        setDataCharacteristics(VELOCITY, "Velocity", "v", "m/s", Flow);
        setDataCharacteristics(FORCE, "Force", "F", "N", Intensity);
        setDataCharacteristics(POSITION, "Position", "x", "m");
        setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "c", "N", TLM);
        setDataCharacteristics(CHARIMP, "CharImp", "Zc", "N s/m", TLM);
        setDataCharacteristics(EQMASS, "EquivalentMass", "me", "kg", Hidden);
    }

    virtual void setSpecialStartValues(Node *pNode)
    {
        pNode->setDataValue(WAVEVARIABLE, mDataValues[FORCE]);
        //! todo Maybe also write CHARIMP?
    }
};

//! @brief A rotational mechanic node
//! @ingroup NodeMechanicRotational
class NodeMechanicRotational :public Node
{
public:
    //! @brief The data variable indexes, DATALENGTH is used internally
    //! @ingroup NodeMechanicRotational
    enum DataIndexEnumT {ANGULARVELOCITY, TORQUE, ANGLE, WAVEVARIABLE, CHARIMP, EQINERTIA, DATALENGTH};
    static Node* CreatorFunction() {return new NodeMechanicRotational;}

private:
    NodeMechanicRotational() : Node(DATALENGTH)
    {
        setNiceName("mechanicrotational");
        setDataCharacteristics(ANGULARVELOCITY, "Angular Velocity", "w", "rad/s", Flow);
        setDataCharacteristics(TORQUE, "Torque", "T", "Nm", Intensity);
        setDataCharacteristics(ANGLE, "Angle", "a", "rad");
        setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "c", "Nm", TLM);
        setDataCharacteristics(CHARIMP, "CharImp", "Zc", "?", TLM);
        setDataCharacteristics(EQINERTIA, "Equivalent Inertia", "Je", "kgm^2", Hidden);
    }

    virtual void setSpecialStartValues(Node *pNode)
    {
        pNode->setDataValue(WAVEVARIABLE, mDataValues[TORQUE]);
        //! todo Maybe also write CHARIMP?
    }
};



//! @brief An electric node
//! @ingroup NodeElectric
class NodeElectric :public Node
{
public:
    //! @brief The data variable indexes, DATALENGTH is used internally
    //! @ingroup NodeElectric
    enum DataIndexEnumT {VOLTAGE, CURRENT, WAVEVARIABLE, CHARIMP, DATALENGTH};
    static Node* CreatorFunction() {return new NodeElectric;}

private:
    NodeElectric() : Node(DATALENGTH)
    {
        setNiceName("electric");
        setDataCharacteristics(VOLTAGE, "Voltage", "U", "V", Intensity);
        setDataCharacteristics(CURRENT, "Current", "I", "A", Flow);
        setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "c", "V", TLM);
        setDataCharacteristics(CHARIMP, "CharImp", "Zc", "V/A", TLM);
    }

    virtual void setSpecialStartValues(Node *pNode)
    {
        pNode->setDataValue(WAVEVARIABLE, mDataValues[VOLTAGE]);
        //! todo Maybe also write CHARIMP?
    }
};

}

#endif // NODES_H_INCLUDED
