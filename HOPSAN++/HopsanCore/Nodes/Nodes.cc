//!
//! @file   Nodes.cc
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_nodes function that registers all built in nodes
//!
//$Id$
#include "Nodes.h"

//! @defgroup Nodes Nodes
//! @defgroup HydraulicNode HydraulicNode
//! @ingroup Nodes
//! @defgroup MechanicalNode MechanicalNode
//! @ingroup Nodes
//! @defgroup SignalNode SignalNode
//! @ingroup Nodes

using namespace hopsan;

//!
//! @brief Registers the creator function of all built in nodes
//! @param [in,out] pNodeFactory A pointer the the node factory in wich to register the nodes
//!
DLLIMPORTEXPORT void hopsan::register_nodes(NodeFactory* pNodeFactory)
{
    pNodeFactory->registerCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanicRotational", NodeMechanicRotational::CreatorFunction);
}

