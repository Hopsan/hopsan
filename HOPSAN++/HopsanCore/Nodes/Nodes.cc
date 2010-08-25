//!
//! @file   Nodes.cc
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_nodes function that registers all built in nodes
//!
//$Id$
#include "Nodes.h"

//! @defgroup HydraulicNode HydraulicNode
//! @ingroup Nodes
//! @defgroup MechanicalNode MechanicalNode
//! @ingroup Nodes
//! @defgroup SignalNode SignalNode
//! @ingroup Nodes

//!
//! @brief Registers the creator function of all built in nodes
//! @param [in,out] nfact_ptr A pointer the the node factory in wich to register the nodes
//!

using namespace hopsan;

DLLIMPORTEXPORT void hopsan::register_nodes(NodeFactory* nfact_ptr)
{
    nfact_ptr->registerCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    nfact_ptr->registerCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    nfact_ptr->registerCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
    nfact_ptr->registerCreatorFunction("NodeMechanicRotational", NodeMechanicRotational::CreatorFunction);
}

