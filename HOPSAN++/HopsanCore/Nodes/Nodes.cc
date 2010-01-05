#include "Nodes.h"

NodeTypeT NodeSignal::iDummyId = NodeFactory::RegisterCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
NodeTypeT NodeHydraulic::iDummyId = NodeFactory::RegisterCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
NodeTypeT NodeMechanic::iDummyId = NodeFactory::RegisterCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);

