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
//! @param [in,out] nfampND_ct A pointer the the node factory in wich to register the nodes
//!

using namespace hopsan;

DLLIMPORTEXPORT void hopsan::register_nodes(NodeFactory* nfampND_ct)
{
    nfampND_ct->registerCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    nfampND_ct->registerCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    nfampND_ct->registerCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
    nfampND_ct->registerCreatorFunction("NodeMechanicRotational", NodeMechanicRotational::CreatorFunction);
}

