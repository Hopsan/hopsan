/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Bjï¿½rn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linkï¿½ping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   Nodes.cc
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_nodes function that registers all built in nodes
//!
//$Id$
#include "Nodes.h"
#include "Port.h"

//! @defgroup Nodes Nodes

//! @defgroup NodeHydraulic NodeHydraulic
//! @ingroup Nodes

//! @defgroup NodePneumatic NodePneumatic
//! @ingroup Nodes

//! @defgroup NodeMechanic NodeMechanic
//! @ingroup Nodes

//! @defgroup NodeMechanicRotational NodeMechanicRotational
//! @ingroup Nodes

//! @defgroup NodeSignal NodeSignal
//! @ingroup Nodes

//! @defgroup NodeElectric NodeElectric
//! @ingroup Nodes

//! @defgroup NodeEmpty NodeEmpty
//! @ingroup Nodes

using namespace hopsan;

//!
//! @brief Registers the creator function of all built in nodes
//! @param [in] pNodeFactory A pointer the the node factory in which to register the nodes
//!
void hopsan::register_default_nodes(NodeFactory* pNodeFactory)
{
    pNodeFactory->registerCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodePneumatic", NodePneumatic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanicRotational", NodeMechanicRotational::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeElectric", NodeElectric::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeEmpty", NodeEmpty::CreatorFunction);
}

HydraulicNodeDataPointerStructT hopsan::getHydraulicNodeDataPointerStruct(const Port *pPort)
{
    HydraulicNodeDataPointerStructT data;
    if (pPort->getNodeType() == "NodeHydraulic")
    {
        data.pQ = pPort->getNodeDataPtr(NodeHydraulic::Flow);
        data.pP = pPort->getNodeDataPtr(NodeHydraulic::Pressure);
        data.pT = pPort->getNodeDataPtr(NodeHydraulic::Temperature);
        data.pQdot = pPort->getNodeDataPtr(NodeHydraulic::HeatFlow);
        data.pZc = pPort->getNodeDataPtr(NodeHydraulic::CharImpedance);
        data.pC = pPort->getNodeDataPtr(NodeHydraulic::WaveVariable);
    }
    return data;
}

HydraulicNodeDataValueStructT hopsan::getHydraulicNodeDataValueStruct(const Port *pPort)
{
    HydraulicNodeDataValueStructT data;
    const Node *pNode = pPort->getNodePtr();
    data.q = pNode->getDataValue(NodeHydraulic::Flow);
    data.p = pNode->getDataValue(NodeHydraulic::Pressure);
    data.T = pNode->getDataValue(NodeHydraulic::Temperature);
    data.Qdot = pNode->getDataValue(NodeHydraulic::HeatFlow);
    data.Zc = pNode->getDataValue(NodeHydraulic::CharImpedance);
    data.c = pNode->getDataValue(NodeHydraulic::WaveVariable);
    return data;
}

HydraulicNodeDataValueStructT hopsan::getHydraulicNodeDataValueStruct(const HydraulicNodeDataPointerStructT &rPtrStruct)
{
    HydraulicNodeDataValueStructT data;
    data.q = *rPtrStruct.pQ;
    data.p = *rPtrStruct.pP;
    data.T = *rPtrStruct.pT;
    data.Qdot = *rPtrStruct.pQdot;
    data.Zc = *rPtrStruct.pZc;
    data.c = *rPtrStruct.pC;
    return data;
}
