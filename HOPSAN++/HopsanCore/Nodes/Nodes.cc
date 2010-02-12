//!
//! @file   Nodes.cc
//! @author <FluMeS>
//! @date   2010-01-08
//! @brief Contains the register_nodes function that registers all built in nodes
//!
//$Id$
#include "Nodes.h"

//NodeTypeT NodeSignal::iDummyId = NodeFactory::RegisterCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
//NodeTypeT NodeHydraulic::iDummyId = NodeFactory::RegisterCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
//NodeTypeT NodeMechanic::iDummyId = NodeFactory::RegisterCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);

//!
//! @brief Registers the creator function of all built in nodes
//! @param [in,out] nfact_ptr A pointer the the node factory in wich to register the nodes
//!
DLLIMPORTEXPORT void register_nodes(NodeFactory* nfact_ptr)
{
    nfact_ptr->RegisterCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    nfact_ptr->RegisterCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    nfact_ptr->RegisterCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
}

