#include "Nodes.h"

NodeTypeT NodeHydraulic::iDummyId = NodeFactory::RegisterCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
NodeTypeT NodeMechanic::iDummyId = NodeFactory::RegisterCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
