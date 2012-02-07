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

using namespace hopsan;

//!
//! @brief Registers the creator function of all built in nodes
//! @param [in] pNodeFactory A pointer the the node factory in which to register the nodes
//!
DLLIMPORTEXPORT void hopsan::register_nodes(NodeFactory* pNodeFactory)
{
    pNodeFactory->registerCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodePneumatic", NodePneumatic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanicRotational", NodeMechanicRotational::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeElectric", NodeElectric::CreatorFunction);
}

